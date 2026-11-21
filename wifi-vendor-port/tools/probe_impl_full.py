#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
probe_impl_full.py — 全量编译 baseline/impl-adapted/ 的 21 个 4.14 私有驱动，
对每文件 dump 全部 error/fatal error，并按类别统计，作为「路径 B 整体 API 适配」
的起点证据。

分类：
  MISSING_HEADER   fatal: 'xxx.h' file not found（私有栈/平台头缺）
  PLATFORM_TYPE    MTK 私有类型/宏/字段/符号未声明（头已找到但缺定义）
  API             7.3 已删除的旧 API 残留
  OTHER           其它（语法/签名/未定义外部符号）

用法：python3 tools/probe_impl_full.py --kernel /workspace/linux-7.3.3
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict

OLD_API_HINT = re.compile(
    r"(set_fs|get_ds|KERNEL_DS|mm_segment_t|do_gettimeofday|"
    r"init_timer|setup_timer|create_proc_entry|strlcpy|dma_zalloc_coherent|"
    r"VERIFY_WRITE|VERIFY_READ|'struct file_operations'|'struct timeval')",
    re.I)


def _all_headers(top):
    roots = set([os.path.abspath(top)])
    for dp, _, fns in os.walk(top):
        if any(f.endswith('.h') for f in fns):
            d = os.path.abspath(dp)
            roots.add(d)
            p = os.path.dirname(d)
            while p.startswith(os.path.abspath(top)):
                roots.add(p)
                p = os.path.dirname(p)
    return sorted(roots)


def kernel_include_flags(K):
    try:
        res = subprocess.check_output(['clang', '-print-resource-dir'],
                                      text=True, timeout=15).strip()
        builtin_inc = ['-I' + os.path.join(res, 'include')]
    except Exception:
        builtin_inc = []
    return [
        '-DKBUILD_MODNAME="mtk_impl_probe"',
        '-DKBUILD_MODFILE="mtk_impl_probe.c"',
        '-nostdinc',
    ] + builtin_inc + [
        '-I./arch/arm64/include', '-I./arch/arm64/include/generated',
        '-I./include', '-I./net',
        '-I./arch/arm64/include/uapi',
        '-I./arch/arm64/include/generated/uapi',
        '-I./include/uapi', '-I./include/generated/uapi',
        '-include', './include/linux/compiler-version.h',
        '-include', './include/linux/kconfig.h',
        '-include', './include/linux/compiler_types.h',
        '-D__KERNEL__', '--target=aarch64-linux-gnu', '-mlittle-endian',
        '-fshort-wchar', '-funsigned-char', '-fno-common', '-fno-PIE',
        '-fno-strict-aliasing', '-std=gnu11', '-fms-extensions',
        '-Wno-gnu', '-Wno-microsoft-anon-tag',
        '-Wno-unused-command-line-argument', '-mgeneral-regs-only', '-O2',
        '-fsyntax-only',
    ]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--kernel', default='/workspace/linux-7.3.3')
    ap.add_argument('--impl', default='baseline/impl-adapted')
    ap.add_argument('--extra', action='append', default=[],
                    help='额外 -I 根（可重复）')
    ap.add_argument('--compat', default='compat')
    ap.add_argument('--report', default='reports/IMPL-FULL-PROBE.md')
    ap.add_argument('--dump', default='reports/IMPL-FULL-ERRORS.txt')
    args = ap.parse_args()

    K = os.path.abspath(args.kernel)
    IMP = os.path.abspath(args.impl)
    CMP = os.path.abspath(args.compat)
    if not os.path.isfile(os.path.join(K, 'include/generated/autoconf.h')):
        print('SKIP: 内核未配置', file=sys.stderr); sys.exit(0)

    cfiles = []
    for dp, _, fns in os.walk(IMP):
        for fn in sorted(fns):
            if fn.endswith('.c'):
                cfiles.append(os.path.join(dp, fn))
    cfiles.sort()

    inc_dirs = _all_headers(IMP)
    extra_dirs = []
    for e in args.extra:
        extra_dirs += _all_headers(e)

    flags = (kernel_include_flags(K) + ['-I' + CMP]
             + ['-I' + d for d in inc_dirs]
             + ['-I' + d for d in extra_dirs])

    cat = Counter()
    miss_hdr = Counter()
    api_clean = 0
    per_file = {}
    all_err_lines = []
    for c in cfiles:
        rel = os.path.relpath(c, IMP)
        p = subprocess.run(['clang'] + flags + [c], cwd=K,
                           capture_output=True, text=True, timeout=180)
        errs = [l for l in (p.stdout + p.stderr).splitlines()
                if re.search(r':\d+:\d+: (error|fatal error):', l)]
        hdrs = [m.group(1) for e in errs
                for m in [re.search(r"fatal error: '([^']+)' file not found", e)] if m]
        has_oldapi = any(OLD_API_HINT.search(e) for e in errs)
        for h in hdrs:
            miss_hdr[h] += 1
        if p.returncode == 0 and not errs:
            cat['CLEAN'] += 1
            per_file[rel] = ('CLEAN', 0, errs)
        elif hdrs:
            cat['MISSING_HEADER'] += 1
            per_file[rel] = ('MISSING_HEADER', len(errs), errs)
        elif has_oldapi:
            cat['API'] += 1
            per_file[rel] = ('API', len(errs), errs)
        else:
            cat['PLATFORM_OR_OTHER'] += 1
            # 进一步区分：是否有 MTK 平台符号未声明
            plat = any(re.search(r"undeclared|has no member|expected|conflicting", e) for e in errs)
            per_file[rel] = (('PLATFORM' if plat else 'OTHER'), len(errs), errs)
        if not has_oldapi:
            api_clean += 1
        for e in errs:
            all_err_lines.append(f'{rel}: {e.strip()}')

    print(f'编译单元: {len(cfiles)}')
    print(f'无旧 API 残留: {api_clean}/{len(cfiles)}')
    for k in ('CLEAN', 'MISSING_HEADER', 'API', 'PLATFORM_OR_OTHER'):
        print(f'  {k:18s}: {cat[k]}')
    print(f'  累计 error/fatal: {sum(len(v[2]) for v in per_file.values())}')
    if miss_hdr:
        print('  缺失头 Top15: ' + ', '.join('%s(%d)' % (h, n)
                                            for h, n in miss_hdr.most_common(15)))

    # 写详细错误文本
    os.makedirs(os.path.dirname(os.path.abspath(args.dump)), exist_ok=True)
    with open(args.dump, 'w') as f:
        f.write('# 21 个 4.14 私有驱动 全量编译错误\n\n')
        f.write(f'kernel={K}\nimpl={IMP}\n\n')
        for rel in sorted(per_file):
            tag, n, errs = per_file[rel]
            f.write(f'\n## {rel}  [{tag}]  errors={n}\n')
            for e in errs:
                f.write(f'  {e.strip()}\n')

    # 写 Markdown 摘要
    os.makedirs(os.path.dirname(os.path.abspath(args.report)), exist_ok=True)
    L = ['# 路径 B 21 个 4.14 私有驱动 — 全量编译探针\n\n']
    L.append('由 `tools/probe_impl_full.py` 生成。对 `baseline/impl-adapted/` '
             '全部 21 个 `.c` 用 7.3.3 真实 KBUILD_CFLAGS 做 `-fsyntax-only`。\n\n')
    L.append('| 指标 | 结果 |\n|---|---|\n')
    L.append(f'| 编译单元 | {len(cfiles)} |\n')
    L.append(f'| 无「7.3 已删除旧 API」残留 | **{api_clean}/{len(cfiles)}** |\n')
    L.append(f'| 完全干净（0 error） | {cat["CLEAN"]} |\n')
    L.append(f'| 仅缺头（fatal file not found） | {cat["MISSING_HEADER"]} |\n')
    L.append(f'| 旧 API 残留 | {cat["API"]} |\n')
    L.append(f'| 平台符号/类型/其它 | {cat["PLATFORM_OR_OTHER"]} |\n')
    L.append(f'| 累计 error/fatal 条 | {sum(len(v[2]) for v in per_file.values())} |\n\n')
    L.append('## 每文件状态\n\n')
    L.append('| 文件 | 类别 | errors |\n|---|---|---|\n')
    for rel in sorted(per_file):
        tag, n, _ = per_file[rel]
        L.append(f'| `{rel}` | {tag} | {n} |\n')
    if miss_hdr:
        L.append('\n## 缺失头 Top 20\n\n')
        L.append('| 头 | 命中数 |\n|---|---|\n')
        for h, n in miss_hdr.most_common(20):
            L.append(f'| `{h}` | {n} |\n')
    L.append('\n详细错误见 `reports/IMPL-FULL-ERRORS.txt`。\n')
    open(args.report, 'w').write(''.join(L))
    print('报告:', args.report)
    print('错误明细:', args.dump)


if __name__ == '__main__':
    main()
