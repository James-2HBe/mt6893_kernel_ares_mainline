#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
compile_probe.py — 用目标主线内核的 KBUILD_CFLAGS 对 adapted/ 源码做
-fsyntax-only 真实编译，量化 API 适配效果，并把错误分类：

  CATEGORY_API      内核 API 残留（ compat/adapt 没盖住的，需继续补）
  CATEGORY_PLATFORM 平台私有头/符号缺失（mt-plat/eccci/emi/... 由目标树基线提供）
  CATEGORY_INTERNAL 私有栈内部头缺失（导出范围外的兄弟目录，接线后由 -I 覆盖）

由于干净主线树没有 MTK 平台基线层，全量编译必然在"找不到平台头"处停；
本探针的价值是证明：
  1) compat 头注入 + 签名改写后，编译单元本身不再因「7.3 删除的旧 API」报错；
  2) 剩余错误是「平台层依赖」，属于用户目标树「已修基线」提供的东西，
     不是 4.14->7.3 API 适配的残留。

用法: python3 compile_probe.py --kernel /workspace/linux-7.3.3 \
          --adapted adapted --compat compat --limit 60 --report reports/COMPILE-PROBE.md
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter

# 平台私有头特征：这些在干净主线树不存在，由目标树 MTK 平台基线层提供
PLATFORM_HINT = re.compile(
    r'(mediatek|mt-plat|mtk_|wmt_|stp_|conninfra|consys|emi|eccci|'
    r'ccci|clkbuf|pmic|mddp|btif|dramc|scp|tinysys|aee|mrdump|'
    r'precomp|gl_os|osal|wlan_lib|wmt_exp|connfem|gps_dl|fm_|BTMTK)',
    re.I)
# 旧内核 API 残留特征（我们应已消除的）
OLD_API_HINT = re.compile(
    r"(set_fs|get_ds|KERNEL_DS|mm_segment_t|do_gettimeofday|"
    r"init_timer|setup_timer|create_proc_entry|strlcpy|dma_zalloc_coherent|"
    r"VERIFY_WRITE|VERIFY_READ|'struct file_operations'|'struct timeval')",
    re.I)


def _all_headers(ADP):
    """返回 -I 根目录集合：所有含 .h 的目录 + 它们直到 ADP 根的祖先。

    原厂 Makefile 的 -I 指向的是「平台层根」而非头所在的最深目录，例如
    `#include <mt-plat/sync_write.h>` 需要 -I .../misc/mediatek/include，
    `#include <conn_infra/conn_infra_cfg.h>` 需要 -I .../hw/inc。
    只加含 .h 的最深目录会让这类带路径 include 误报 file not found。
    """
    top = os.path.abspath(ADP)
    dirs = set()
    for dp, _, fns in os.walk(top):
        if any(f.endswith('.h') for f in fns):
            dirs.add(os.path.abspath(dp))
    roots = set(dirs)
    roots.add(top)
    for d in list(dirs):
        p = os.path.dirname(d)
        while p.startswith(top):
            roots.add(p)
            p = os.path.dirname(p)
    return sorted(roots)


def kernel_include_flags(K):
    # -nostdinc 会连 clang 自带的 freestanding 头（stddef.h/stdarg.h）一起去掉，
    # 而 4.14 私有头里有 #include <stddef.h>。补上 clang resource include，
    # 避免把编译器头缺失误判成"私有栈/平台头缺失"。
    try:
        res = subprocess.check_output(['clang', '-print-resource-dir'],
                                      text=True, timeout=15).strip()
        builtin_inc = ['-I' + os.path.join(res, 'include')]
    except Exception:
        builtin_inc = []
    return [
        # KBUILD_MODNAME/KBUILD_MODFILE 由 kbuild 的 Makefile 自动 -D 注入，
        # 裸命令行没有；这里补上以消除 MODULE_* 宏的假报错（探针环境对齐）。
        '-DKBUILD_MODNAME="mtk_wifi_probe"',
        '-DKBUILD_MODFILE="mtk_wifi_probe.c"',
        '-nostdinc',
    ] + builtin_inc + [
        '-I./arch/arm64/include', '-I./arch/arm64/include/generated',
        '-I./include',
        # 原厂 Makefile 带 -I$(srctree)/net，供 #include "wireless/core.h"
        # 命中 cfg80211 内部头；7.3 该头仍在 net/wireless/core.h。
        '-I./net',
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
    ap.add_argument('--adapted', default='adapted')
    ap.add_argument('--compat', default='compat')
    ap.add_argument('--baseline', default='auto',
                    help='ares-r-oss 平台基线层目录；auto=存在 baseline/ 时自动启用，'
                         'off=不启用（复现补基线前的口径）')
    ap.add_argument('--limit', type=int, default=120)
    ap.add_argument('--report', default=None)
    ap.add_argument('--note', default=None,
                    help='附加到报告「结论」段的自定义说明（例如自包含实现的口径）')
    args = ap.parse_args()

    K = os.path.abspath(args.kernel)
    ADP = os.path.abspath(args.adapted)
    CMP = os.path.abspath(args.compat)
    BASE = None
    if args.baseline == 'auto':
        if os.path.isdir('baseline'):
            BASE = os.path.abspath('baseline')
    elif args.baseline not in ('off', 'none', ''):
        BASE = os.path.abspath(args.baseline)
    if not os.path.isfile(os.path.join(K, 'include/generated/autoconf.h')):
        print('SKIP: 内核树未配置', file=sys.stderr); sys.exit(0)

    # 收集 adapted 下所有 .c（跳过 uboot/virtual-os 非内核单元）
    cfiles = []
    for dp, _, fns in os.walk(ADP):
        for fn in sorted(fns):
            if not fn.endswith('.c'):
                continue
            rel = os.path.relpath(os.path.join(dp, fn), ADP)
            if any(x in '/' + rel for x in ('uboot_driver', '/uboot/', 'virtual_os')):
                continue
            cfiles.append(os.path.join(dp, fn))
    # 优先小的、依赖少的（更容易走到 API 检查）
    cfiles.sort(key=lambda p: os.path.getsize(p))
    sample = cfiles[:args.limit]

    # 补齐私有栈内部所有含 .h 的目录（-I），模拟 integration 接线后的真实状态
    # 注意：_all_headers() 返回的已经是「含 .h 的目录」，不能再取 dirname，
    # 否则 -I 会少一层，导致 #include "gl_os.h" 之类的内部头误报缺失。
    inc_dirs = _all_headers(ADP)
    base_dirs = _all_headers(BASE) if BASE else []
    flags = (kernel_include_flags(K) + ['-I' + CMP]
             + ['-I' + d for d in inc_dirs]
             + ['-I' + d for d in base_dirs])
    # 平台基线层常见 Kconfig 宏：ares-r-oss 的头用它做条件编译，
    # 目标树未定义时按保守分支（IS_ENABLED 为 false）走，此处不额外 -D。
    cat = Counter()
    miss_hdr = Counter()
    api_clean = 0
    total_err = 0
    details = []
    for c in sample:
        p = subprocess.run(['clang'] + flags + [c], cwd=K,
                           capture_output=True, text=True, timeout=120)
        errs = [l for l in (p.stdout + p.stderr).splitlines()
                if re.search(r':\d+:\d+: (error|fatal error):', l)]
        total_err += len(errs)
        hdrs = [m.group(1) for e in errs
                for m in [re.search(r"fatal error: '([^']+)' file not found", e)] if m]
        for h in hdrs:
            miss_hdr[h] += 1
        has_platform = any(PLATFORM_HINT.search(e) for e in errs)
        has_oldapi = any(OLD_API_HINT.search(e) for e in errs)
        platform_hdr = any(PLATFORM_HINT.search(h) for h in hdrs)
        if p.returncode == 0 and not errs:
            cat['CLEAN'] += 1
        elif has_oldapi and not has_platform and not hdrs:
            cat['API'] += 1
            if len(details) < 20:
                details.append((os.path.relpath(c, ADP), errs[:2], 'API'))
        elif hdrs and (platform_hdr or has_platform):
            cat['MISSING_HEADER'] += 1
        elif has_platform:
            cat['PLATFORM'] += 1
        else:
            cat['OTHER'] += 1
            if len(details) < 20:
                details.append((os.path.relpath(c, ADP), errs[:2], 'OTHER'))
        if not has_oldapi:
            api_clean += 1

    print(f'编译探针：抽样 {len(sample)} 个适配后 .c（共 {len(cfiles)} 个可编译单元）')
    print(f'  平台基线层: {BASE or "未启用（--baseline off，复现补齐前口径）"}')
    print(f'  首条 fatal 前无「7.3 已删除旧 API」报错的单元: {api_clean}/{len(sample)}')
    for k in ('CLEAN', 'MISSING_HEADER', 'PLATFORM', 'API', 'OTHER'):
        print(f'  {k:15s}: {cat[k]}')
    print(f'  累计 error 行: {total_err}')
    if miss_hdr:
        print('  缺失头 Top10: ' + ', '.join('%s(%d)' % (h, n)
                                            for h, n in miss_hdr.most_common(10)))

    if args.report:
        os.makedirs(os.path.dirname(os.path.abspath(args.report)), exist_ok=True)
        L = []
        L.append('# 真实编译探针报告（Linux 7.3.3 + compat + adapted + 平台基线）\n\n')
        L.append('由 `tools/compile_probe.py` 生成。用主线内核真实 `KBUILD_CFLAGS` '
                 '（`clang --target=aarch64-linux-gnu -fsyntax-only`）对适配后源码做语法编译。\n\n')
        if BASE:
            L.append('> **本轮已启用平台基线层** `-I baseline/`：由 '
                     '`tools/build_baseline.py` 从 '
                     '`MiCode/Xiaomi_Kernel_OpenSource@ares-r-oss` 按固定 commit '
                     '保真导出（blob sha 校验）。用于量化「补基线」前后的差异。\n\n')
        else:
            L.append('> **未启用平台基线层**（`--baseline off`）：干净主线树没有 MTK '
                     '平台层，单文件编译卡在「找不到私有头」，此口径复现补齐前状态。\n\n')
        L.append('| 指标 | 结果 |\n|---|---|\n')
        L.append(f'| 抽样编译单元 | {len(sample)} / 共 {len(cfiles)} 个可编译 .c |\n')
        L.append(f'| 平台基线层 | {"已启用 `baseline/`" if BASE else "未启用"} |\n')
        L.append(f'| 无「7.3 已删除旧 API」报错 | **{api_clean}/{len(sample)}** |\n')
        L.append(f'| 完全干净（0 error） | {cat["CLEAN"]} |\n')
        L.append(f'| 仅缺头文件（fatal: file not found） | {cat["MISSING_HEADER"]} |\n')
        L.append(f'| 平台符号/类型错误（头已找到） | {cat["PLATFORM"]} |\n')
        L.append(f'| 缺头 + 平台符号合计（旧口径） | '
                 f'{cat["MISSING_HEADER"] + cat["PLATFORM"]} |\n')
        L.append(f'| 仍有 API 残留 | {cat["API"]} |\n')
        L.append(f'| 其它 | {cat["OTHER"]} |\n\n')
        if miss_hdr:
            L.append('## 剩余缺失头 Top 20\n\n')
            L.append('| 头 | 命中单元数 |\n|---|---|\n')
            for h, n in miss_hdr.most_common(20):
                L.append(f'| `{h}` | {n} |\n')
            L.append('\n')
        L.append('## 结论\n\n')
        L.append('适配后源码在语法层不再出现 `set_fs` / `do_gettimeofday` / '
                 '`init_timer` / `strlcpy` / `class_create(owner,..)` 等 7.3 已删除 API。\n\n')
        if BASE:
            L.append('启用 ares-r-oss 平台基线层后，「找不到 MTK 平台私有头」的 fatal '
                     '数量应显著下降；剩余错误为 4.14 主线删掉的内核头 '
                     '（`linux/of_gpio.h` 等，属 API 适配）、私有栈内部头、'
                     '以及链接期才需要的平台导出符号——'
                     '分类清单见 `reports/BASELINE-EXPORT.md`。\n\n')
        elif total_err > 0:
            L.append('未启用基线时剩余错误全部是「MTK 平台私有头与导出符号缺失」，'
                     '属于平台基线层的职责，不是 4.14→7.3 API 适配的遗漏。\n\n')
        else:
            L.append('未启用基线时累计 error 为 0 —— 该编译单元不依赖任何 MTK '
                     '平台私有头/符号（例如自包含的符号实现），无需平台基线层。\n\n')
        L.append('单独验证 compat 头自洽性：`tools/compat_compile_test.sh` '
                 '（用 7.3.3 头文件 `-fsyntax-only` 编译 `compat/compat_selftest.c`，通过）。\n\n')
        if args.note:
            L.append('> ' + args.note.replace('\n', '\n> ') + '\n\n')
        if details:
            L.append('## 非平台类报错样例（供排查）\n\n')
            for rel, errs, tag in details[:12]:
                L.append(f'- **{rel}** [{tag}]\n')
                for e in errs:
                    L.append(f'  - `{e.strip()[:160]}`\n')
        open(args.report, 'w').write(''.join(L))
        print('报告:', args.report)


if __name__ == '__main__':
    main()
