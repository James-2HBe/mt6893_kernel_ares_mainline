#!/usr/bin/env python3
"""校验保真导出的完整性（自包含，无需网络 / 无需原厂 git 仓库）。

用法:
  python3 verify_export.py [导出根目录] [可选: 原厂 git 仓库目录]

模式:
  1) 默认：读 tools/MANIFEST.txt（本包自带，记录了每个文件的 git blob sha），
     逐文件重算 blob sha 比对，检测导出后被改动/缺失/多余的文件。
     只需本包自身即可运行，适合交付后用户自检。
  2) 传入含目标 commit 的 git 仓库目录时：额外用 `git ls-tree -r HEAD`
     交叉验证 MANIFEST 里记录的 sha 与原厂 tree 一致（真正的"保真"校验）。
     仓库可为 --filter=blob:none 浅克隆，因为只读 tree 里的 sha，不读 blob 内容。

blob sha = sha1(b"blob <len>\\0" + content)，与 git object id 定义一致。
退出码: 0 = 全部通过；1 = 存在 MISMATCH / MISSING。
"""
import os, sys, subprocess, hashlib

def blob_sha(content):
    h = hashlib.sha1()
    h.update(b"blob " + str(len(content)).encode() + b"\0")
    h.update(content)
    return h.hexdigest()

def load_manifest(root):
    mf = os.path.join(root, "tools", "MANIFEST.txt")
    result = {}
    for line in open(mf, encoding="utf-8"):
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        parts = [x.strip() for x in line.split("|")]
        if len(parts) == 3:
            result[parts[0]] = (int(parts[1]), parts[2])
    return result

def main():
    root = sys.argv[1] if len(sys.argv) > 1 else os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    repo = sys.argv[2] if len(sys.argv) > 2 else None
    manifest = load_manifest(root)
    print(f"MANIFEST 记录 {len(manifest)} 文件")

    ok = mismatch = missing = 0
    problems = []
    for rel, (size, sha) in manifest.items():
        full = os.path.join(root, rel)
        if not os.path.isfile(full):
            missing += 1; problems.append(("MISSING", rel)); continue
        c = open(full, "rb").read()
        if len(c) == size and blob_sha(c) == sha:
            ok += 1
        else:
            mismatch += 1; problems.append(("MISMATCH", rel))
    print(f"完整性(vs MANIFEST): OK={ok} MISMATCH={mismatch} MISSING={missing}")

    # 额外：检测 drivers/ 下是否有 MANIFEST 未记录的文件（多余/注入）
    extra = 0
    for base in ("drivers", os.path.join("dts", "original")):
        for dp, _, fns in os.walk(os.path.join(root, base)):
            for fn in fns:
                rel = os.path.relpath(os.path.join(dp, fn), root)
                if rel not in manifest:
                    extra += 1
    print(f"未在清单内的源码/DT 文件: {extra}")

    rc = 0
    for t, p in problems[:30]:
        print(f"  {t} {p}")
    if problems:
        rc = 1

    # 可选：交叉验证 MANIFEST 权威性与原厂 tree 一致
    if repo and os.path.isdir(os.path.join(repo, ".git")):
        listing = subprocess.check_output(["git", "-C", repo, "ls-tree", "-r", "HEAD"], text=True)
        expect = {}
        for line in listing.splitlines():
            meta, path = line.split("\t", 1)
            pp = meta.split()
            if len(pp) >= 3 and pp[1] == "blob":
                expect[path] = pp[2]
        # 只核对 MANIFEST 里属于原厂仓库路径的条目
        checked = agree = disagree = 0
        for rel, (_, sha) in manifest.items():
            repo_path = rel
            if repo_path in expect:
                checked += 1
                if expect[repo_path] == sha: agree += 1
                else: disagree += 1
        print(f"权威性(vs 原厂 tree): 核对 {checked} 条, 一致 {agree}, 不一致 {disagree}")
        if disagree:
            rc = 1
    else:
        print("权威性校验: 跳过（未提供原厂 git 仓库；交付后自检用 MANIFEST 模式即可）")

    return rc

if __name__ == "__main__":
    sys.exit(main())
