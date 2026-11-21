#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""保真导出「aee_rr_rec_clk 真实实现」的原厂文件，作为审计基准。

背景（stage-4）：
  stage-3 把 aee_rr_rec_clk / buck_set_mode 判为「全树只有 extern 声明、
  无 .c 实现，需桩」。本轮复核发现该结论不准确：
    - aee_rr_rec_clk 的真实实现在
      drivers/misc/mediatek/ram_console/mtk_ram_console.c:1129，
      属 aee_rr_rec_* 族（3598 行）；stage-3 的关键词搜索只覆盖
      aee/、pmic/、clk-mt68 等，没覆盖 ram_console/ 路径，故漏搜。
    - buck_set_mode 的 7 次「调用」全是注释掉的历史代码，且声明仅在
      CONFIG_MTK_PMIC_CHIP_MT6335 条件块内（MT6893 为 MT6359P）。

本脚本把真实实现文件按原路径保真导出到 baseline/reference/，
逐文件用 git blob sha 校验（复用 port_platform_impl.py 的通道），
生成 manifest；不改动任何已有实现/报告。

通道：cdn.jsdelivr.net（主）→ fastly.jsdelivr.net（备）→ github raw（兜底），
固定 commit，内容与 git ls-tree 的 blob sha 一致才算成功。
"""
import argparse
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import port_platform_impl as P

# 需要保真导出的原厂文件（relative to repo root）
REF_FILES = [
    'drivers/misc/mediatek/ram_console/mtk_ram_console.c',
    'drivers/misc/mediatek/ram_console/ram_console.h',
    'drivers/misc/mediatek/ram_console/desc/desc_def.h',
    'drivers/misc/mediatek/ram_console/desc/desc_def_per_cpu.h',
    'drivers/misc/mediatek/ram_console/desc/desc_s.h',
    'drivers/misc/mediatek/ram_console/Kconfig',
    'drivers/misc/mediatek/ram_console/Makefile',
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/mt6893-lin232')
    ap.add_argument('--out', default='baseline/reference')
    ap.add_argument('--manifest', default='tools/reference-manifest.tsv')
    ap.add_argument('--workers', type=int, default=6)
    args = ap.parse_args()

    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.repo,
                            capture_output=True, text=True).stdout.strip()
    print('源 commit: %s' % commit, flush=True)
    idx = P.build_blob_index(args.repo)
    fetch = P.make_fetcher(commit, idx, workers=args.workers)
    got = fetch(REF_FILES)
    print('下载成功: %d / %d' % (len(got), len(REF_FILES)), flush=True)

    rows = []
    for p in REF_FILES:
        if p not in got:
            print('  MISSING %s' % p)
            rows.append((p, 'MISSING', 0, 'download-failed'))
            continue
        sha, data = got[p]
        expect = idx.get(p)
        ok = (expect is None or sha == expect)
        dst = os.path.join(args.out, p)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, 'wb').write(data)
        rows.append((p, sha, len(data), 'OK' if ok else 'SHA-MISMATCH'))
        print('  %-6s %8d  %s' % ('OK' if ok else 'BAD', len(data), p))

    with open(args.manifest, 'w') as f:
        f.write('# path\tgit_blob_sha\tbytes\tstatus\n')
        for p, sha, n, st in rows:
            f.write('%s\t%s\t%d\t%s\n' % (p, sha, n, st))

    nok = sum(1 for r in rows if r[3] == 'OK')
    print('保真校验: OK=%d / %d' % (nok, len(REF_FILES)))
    print('manifest -> %s' % args.manifest)
    return 0 if nok == len(REF_FILES) else 1


if __name__ == '__main__':
    sys.exit(main())
