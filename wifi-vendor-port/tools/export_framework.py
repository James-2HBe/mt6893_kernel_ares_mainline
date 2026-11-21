#!/usr/bin/env python3.11
# -*- coding: utf-8 -*-
"""
B 类（clk/iommu/devfreq）框架符号移植 —— 保真导出器（stage-5）

从两个 GitHub 开源仓库按 blob sha 逐文件校验导出：

  1) torvalds/linux @ master            —— 主线 Linux 的 MT6893 框架等价实现（锚点）
  2) xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2
                                        —— 4.14 私有 B 类符号实现 + MT6893 variant 链

通道（本环境实测）：
  - 主线：api.github.com contents 取 blob sha -> raw.githubusercontent.com 下载
          -> sha1("blob <len>\\0" + data) 与 git blob sha 比对
          （cdn.jsdelivr.net 对 torvalds/linux 返回 403 "package > 50 MB"，弃用）
  - vendor：本地稀疏克隆 git ls-tree 取 blob sha -> cdn.jsdelivr.net/gh/<repo>@<sha>/<path>
          -> 同样的 git object hash 比对

产出：framework/upstream/、framework/vendor/、tools/framework-manifest.tsv
包内保真副本自包含，目标树接线无需联网；本脚本仅用于审计复现。
"""
from __future__ import annotations

import argparse
import base64
import hashlib
import json
import os
import subprocess
import sys
import time
import urllib.request

UPSTREAM_REPO = "torvalds/linux"
UPSTREAM_REF = "master"
VENDOR_REPO = "xiaomi-mt6893-dev/kernel_xiaomi_mt6893"
VENDOR_COMMIT = "aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f"

# 主线 Linux：B 类三个子系统里，与私有符号语义对应的框架实现（10 个）
UPSTREAM_FILES = [
    "drivers/clk/mediatek/clk-mtk.c",                    # mtk_clk_simple_probe / notifier 框架
    "drivers/clk/mediatek/clk-mtk.h",                    # mtk_clk_desc.clk_notifier_func
    "drivers/clk/mediatek/clk-mux.c",                    # devm_mtk_clk_mux_notifier_register
    "drivers/memory/mtk-smi.c",                          # mt6893-smi-larb + larb suspend/resume
    "drivers/iommu/mtk_iommu.c",                         # mt6893_data / mt6893-iommu-mm
    "drivers/soc/mediatek/mtk-dvfsrc.c",                 # dvfsrc_get_current_opp()->vcore_opp
    "drivers/regulator/mtk-dvfsrc-regulator.c",          # DVFSRC regulator 对外接口
    "drivers/pmdomain/mediatek/mtk-pm-domains.c",        # generic_pm_domain power_on/off
    "drivers/pmdomain/mediatek/mt6893-pm-domains.h",     # MT6893 power domain 描述
    "include/soc/mediatek/smi.h",                        # mtk_smi_larb_iommu / MTK_SMI_MMU_EN
]

# 4.14 合并树：B 类符号的私有实现与 MT6893 variant 依赖链
VENDOR_FILES = [
    "drivers/clk/mediatek/clk-mt6873-pg.c",              # register_pg_callback 真实实现
    "drivers/clk/mediatek/clk-mt6873-pg.h",              # struct pg_callbacks / enum subsys_id
    "drivers/clk/mediatek/Makefile",
    "drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c",     # smi_reg_{backup,restore}_sec
    "drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.h",
    "drivers/misc/mediatek/pseudo_m4u/mt_smi.h",
    "drivers/misc/mediatek/pseudo_m4u/Makefile",
    "drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/pseudo_m4u.h",
    "drivers/devfreq/helio-dvfsrc-opp.c",                # get_cur_vcore_opp 真实实现
    "drivers/devfreq/helio-dvfsrc-opp.h",
    "drivers/devfreq/helio-dvfsrc-opp-mt6893.h",         # MT6893 OPP 表变体
    "drivers/devfreq/helio-dvfsrc-v3/helio-dvfsrc-opp-mt6893.c",
    "drivers/devfreq/helio-dvfsrc-v3/helio-dvfsrc-mt6893.c",
    "drivers/devfreq/Makefile",
    "drivers/iommu/mtk_iommu_ext.c",
    "drivers/iommu/mtk_iommu_ext.h",                     # smi_reg_*_sec 声明
]

UA = {"User-Agent": "ares-stage5-exporter"}


def git_blob_sha(data: bytes) -> str:
    return hashlib.sha1(b"blob %d\0" % len(data) + data).hexdigest()


def http_get(url: str, timeout: int = 40, retries: int = 3) -> bytes:
    last = None
    for i in range(retries):
        try:
            req = urllib.request.Request(url, headers=UA)
            with urllib.request.urlopen(req, timeout=timeout) as r:
                return r.read()
        except Exception as e:  # noqa: BLE001
            last = e
            time.sleep(1.2 * (i + 1))
    raise RuntimeError("GET failed %s: %s" % (url, last))


def upstream_blob_sha(path: str, ref: str) -> str:
    """GitHub contents API: 单文件 blob sha（无需克隆整棵树）。"""
    url = ("https://api.github.com/repos/%s/contents/%s?ref=%s"
           % (UPSTREAM_REPO, path, ref))
    req = urllib.request.Request(url, headers=UA)
    for i in range(3):
        try:
            with urllib.request.urlopen(req, timeout=30) as r:
                d = json.load(r)
            if not isinstance(d, dict) or "sha" not in d:
                raise RuntimeError("unexpected API reply for %s" % path)
            return d["sha"]
        except Exception as e:  # noqa: BLE001
            if i == 2:
                raise RuntimeError("contents API %s: %s" % (path, e))
            time.sleep(1.5 * (i + 1))
    raise AssertionError("unreachable")


def upstream_download(path: str, ref: str, expect_sha: str) -> bytes:
    """raw 主通道，GitHub API blob(base64) 备通道；两者都做 object-hash 校验。"""
    urls = [
        "https://raw.githubusercontent.com/%s/%s/%s" % (UPSTREAM_REPO, ref, path),
    ]
    for url in urls:
        try:
            data = http_get(url, timeout=40, retries=2)
            if git_blob_sha(data) == expect_sha:
                return data
        except Exception:  # noqa: BLE001
            pass
    # 备通道：git blobs API（base64）
    api = "https://api.github.com/repos/%s/git/blobs/%s" % (UPSTREAM_REPO, expect_sha)
    try:
        raw = http_get(api, timeout=40, retries=2)
        d = json.loads(raw)
        data = base64.b64decode(d["content"])
        if git_blob_sha(data) == expect_sha:
            return data
    except Exception as e:  # noqa: BLE001
        raise RuntimeError("upstream fallback failed %s: %s" % (path, e))
    raise RuntimeError("upstream blob sha mismatch: %s" % path)


def vendor_blob_index(repo: str, files: list[str]) -> dict[str, str]:
    """本地稀疏克隆的 git ls-tree：path -> blob sha（不取 blob 内容）。"""
    out = subprocess.run(
        ["git", "-C", repo, "ls-tree", "-r", "HEAD"] + files,
        capture_output=True, text=True, check=True).stdout
    idx = {}
    for line in out.splitlines():
        # "<mode> blob <sha>\t<path>"
        if "\t" not in line:
            continue
        meta, path = line.split("\t", 1)
        parts = meta.split()
        if len(parts) >= 3 and parts[1] == "blob":
            idx[path] = parts[2]
    return idx


def vendor_download(path: str, expect_sha: str, commit: str) -> bytes:
    urls = [
        "https://cdn.jsdelivr.net/gh/%s@%s/%s" % (VENDOR_REPO, commit, path),
        "https://fastly.jsdelivr.net/gh/%s@%s/%s" % (VENDOR_REPO, commit, path),
        "https://raw.githubusercontent.com/%s/%s/%s" % (VENDOR_REPO, commit, path),
    ]
    last = "no channel"
    for url in urls:
        try:
            data = http_get(url, timeout=35, retries=2)
            if git_blob_sha(data) == expect_sha:
                return data
            last = "sha mismatch (%s)" % git_blob_sha(data)[:12]
        except Exception as e:  # noqa: BLE001
            last = str(e)
    raise RuntimeError("vendor download failed %s: %s" % (path, last))


def write_file(root: str, rel: str, data: bytes):
    dst = os.path.join(root, rel)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, "wb") as f:
        f.write(data)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--root", default="framework")
    ap.add_argument("--manifest", default="tools/framework-manifest.tsv")
    ap.add_argument("--vendor-repo", default="/tmp/mt6893-lin232")
    ap.add_argument("--upstream-ref", default=UPSTREAM_REF)
    ap.add_argument("--skip-upstream", action="store_true")
    ap.add_argument("--skip-vendor", action="store_true")
    args = ap.parse_args()

    rows = []  # (side, repo, ref, path, blob_sha, size, ok)

    if not args.skip_upstream:
        print("[upstream] %s@%s, %d files" % (UPSTREAM_REPO, args.upstream_ref,
                                              len(UPSTREAM_FILES)))
        for i, p in enumerate(UPSTREAM_FILES, 1):
            try:
                sha = upstream_blob_sha(p, args.upstream_ref)
                data = upstream_download(p, args.upstream_ref, sha)
                assert git_blob_sha(data) == sha
                write_file(os.path.join(args.root, "upstream"), p, data)
                rows.append(("upstream", UPSTREAM_REPO, args.upstream_ref, p,
                             sha, len(data), "OK"))
                print("  [%2d/%d] OK %-58s %8d" % (i, len(UPSTREAM_FILES), p, len(data)))
            except Exception as e:  # noqa: BLE001
                rows.append(("upstream", UPSTREAM_REPO, args.upstream_ref, p,
                             "-", 0, "FAIL:%s" % e))
                print("  [%2d/%d] FAIL %s: %s" % (i, len(UPSTREAM_FILES), p, e),
                      file=sys.stderr)

    if not args.skip_vendor:
        commit = VENDOR_COMMIT
        if os.path.isdir(args.vendor_repo):
            try:
                commit = subprocess.run(["git", "-C", args.vendor_repo, "rev-parse", "HEAD"],
                                        capture_output=True, text=True,
                                        check=True).stdout.strip()
            except Exception:  # noqa: BLE001
                pass
        print("[vendor] %s@%s, %d files" % (VENDOR_REPO, commit[:12], len(VENDOR_FILES)))
        idx = vendor_blob_index(args.vendor_repo, VENDOR_FILES)
        for i, p in enumerate(VENDOR_FILES, 1):
            sha = idx.get(p)
            if not sha:
                rows.append(("vendor", VENDOR_REPO, commit, p, "-", 0,
                             "FAIL:not in git ls-tree"))
                print("  [%2d/%d] FAIL %s (not in tree)" % (i, len(VENDOR_FILES), p),
                      file=sys.stderr)
                continue
            try:
                data = vendor_download(p, sha, commit)
                assert git_blob_sha(data) == sha
                write_file(os.path.join(args.root, "vendor"), p, data)
                rows.append(("vendor", VENDOR_REPO, commit, p, sha, len(data), "OK"))
                print("  [%2d/%d] OK %-58s %8d" % (i, len(VENDOR_FILES), p, len(data)))
            except Exception as e:  # noqa: BLE001
                rows.append(("vendor", VENDOR_REPO, commit, p, sha, 0, "FAIL:%s" % e))
                print("  [%2d/%d] FAIL %s: %s" % (i, len(VENDOR_FILES), p, e),
                      file=sys.stderr)

    os.makedirs(os.path.dirname(args.manifest) or ".", exist_ok=True)
    with open(args.manifest, "w", encoding="utf-8") as f:
        f.write("side\trepo\tref\tpath\tblob_sha\tsize\tstatus\n")
        for r in rows:
            f.write("%s\t%s\t%s\t%s\t%s\t%d\t%s\n" % r)

    ok = sum(1 for r in rows if r[6] == "OK")
    print("\n汇总: %d/%d 保真校验通过 -> %s" % (ok, len(rows), args.manifest))
    if ok != len(rows):
        sys.exit(2)


if __name__ == "__main__":
    main()
