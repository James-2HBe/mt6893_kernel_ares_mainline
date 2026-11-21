#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
vendor_missing_headers.py — 迭代把 probe_impl_full.py 报出的「缺头」从
baseline/platform + baseline/drivers 的真实源 vendoring 进
baseline/impl-adapted/include/<X>，直到没有新增可搬缺头。

目的：让路径 B 的 21 个 4.14 私有驱动达到「编译期自包含」——依赖的平台头
随驱动一起搬运，目标树不需要再补任何私有头。

策略：
  - 缺头 X 原样复制到 impl-adapted/include/<X>（该目录是探针 -I 根之一）。
    例：X="mach/upmu_hw.h"  -> include/mach/upmu_hw.h
        X="include/pmic.h"  -> include/include/pmic.h
        X="mtk_spm.h"       -> include/mtk_spm.h
  - 源候选：find baseline/platform baseline/drivers -path "*<X>"。
    优先结构化源（非 flat/），flat/ 作为兜底；排除 impl-adapted 自身。
  - 无法在平台树定位的（如 7.3 已删除的 linux/*, asm/* 内核头）记为
    UNRESOLVED，交给 compat 桩人工处理，不猜测复制。

用法：python3 tools/vendor_missing_headers.py [--max-rounds 20]
"""
import argparse
import os
import re
import shutil
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
PROBE = os.path.join(ROOT, 'tools', 'probe_impl_full.py')
DUMP = os.path.join(ROOT, 'reports', 'IMPL-FULL-ERRORS.txt')
DST_BASE = os.path.join(ROOT, 'baseline', 'impl-adapted', 'include')
SRC_ROOTS = [os.path.join(ROOT, 'baseline', 'platform'),
             os.path.join(ROOT, 'baseline', 'drivers')]

FATAL = re.compile(r"fatal error: '([^']+)' file not found")


def run_probe():
    subprocess.run([sys.executable, PROBE], cwd=ROOT,
                   capture_output=True, text=True, timeout=1200)
    txt = open(DUMP, encoding='utf-8', errors='replace').read()
    return sorted(set(FATAL.findall(txt)))


def find_src(X):
    """返回 X 的候选源路径列表（结构化优先，flat 兜底）。"""
    cands = []
    for root in SRC_ROOTS:
        if not os.path.isdir(root):
            continue
        r = subprocess.run(['find', root, '-type', 'f', '-path', '*' + X],
                           capture_output=True, text=True, timeout=120)
        for line in r.stdout.splitlines():
            p = line.strip()
            if 'impl-adapted' in p:
                continue
            cands.append(p)
    # 去重；结构化源（不含 /flat/）排在前面
    seen, ordered = set(), []
    for p in sorted(cands, key=lambda q: ('/flat/' in q, len(q), q)):
        if p not in seen:
            seen.add(p)
            ordered.append(p)
    return ordered


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--max-rounds', type=int, default=20)
    args = ap.parse_args()

    total = 0
    for rnd in range(1, args.max_rounds + 1):
        missing = run_probe()
        if not missing:
            print(f'[round {rnd}] 缺头 = 0，收敛完成')
            break
        moved, unresolved = 0, []
        for X in missing:
            dst = os.path.join(DST_BASE, X)
            if os.path.exists(dst):
                continue  # 已 vendored（若仍报缺说明路径解析歧义，留待人工）
            srcs = find_src(X)
            if not srcs:
                unresolved.append(X)
                continue
            src = srcs[0]
            os.makedirs(os.path.dirname(dst), exist_ok=True)
            shutil.copy2(src, dst)
            moved += 1
            total += 1
            print(f'  + {X:42s} <- {os.path.relpath(src, ROOT)}')
        print(f'[round {rnd}] 报缺={len(missing)}  本轮新增={moved}  '
              f'无法定位={len(unresolved)}')
        if unresolved:
            for X in unresolved:
                print(f'      UNRESOLVED(需 compat 桩/人工): {X}')
        if moved == 0:
            print('  无新增可搬，停止迭代（剩余缺头需 compat 桩或路径修正）')
            break
    print(f'\n累计 vendoring 新头: {total}')
    print(f'当前 include/ 头文件总数: '
          f'{sum(1 for _ in subprocess.run(["find", DST_BASE, "-name", "*.h"], capture_output=True, text=True).stdout.splitlines())}')


if __name__ == '__main__':
    main()
