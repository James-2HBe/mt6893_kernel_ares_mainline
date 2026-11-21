#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
scan_baseline_modules.py — 补齐「链接期平台基线」的落点清单。

build_baseline.py 解决的是**头文件**（编译期 -fsyntax-only）；
本脚本解决的是**实现**：从 ares-r-oss 导出 WiFi 栈所依赖的 MTK 平台层
Makefile / Kconfig（保真），解析出「Kconfig 开关 -> 目标模块 -> .c 实现」，
再从 baseline/ 头的函数声明与 adapted/ 的调用点求交集，列出链接期真正
需要目标树提供的平台导出符号。

产出：
  baseline/build-files/<原路径>       保真 Makefile/Kconfig
  reports/BASELINE-MODULES.md         模块表 + 平台符号表 + 接线建议
  tools/baseline-modules.tsv          机器可读模块表
"""
import argparse
import os
import re
import sys
from collections import Counter, defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from build_baseline import git_read_blobs, load_blob_index  # noqa: E402

PLATFORM_DIRS = [
    'drivers/misc/mediatek/base/power',
    'drivers/misc/mediatek/pmic',
    'drivers/misc/mediatek/emi',
    'drivers/misc/mediatek/emi_mpu',
    'drivers/misc/mediatek/m4u',
    'drivers/misc/mediatek/eccci',
    'drivers/misc/mediatek/mddp',
    'drivers/misc/mediatek/performance',
    'drivers/misc/mediatek/freqhopping',
    'drivers/misc/mediatek/debug_latch',
    'drivers/misc/mediatek/devmpu',
    'drivers/misc/mediatek/connectivity',
    'drivers/devfreq',
    'drivers/clk/mediatek',
    'drivers/power/supply/mediatek/battery',
    'drivers/iommu',
]

OBJ_CFG = re.compile(r'^\s*obj-\$\((CONFIG_\w+)\)\s*\+=\s*(.+?)\s*$', re.M)
OBJ_Y = re.compile(r'^\s*obj-y\s*\+=\s*(.+?)\s*$', re.M)
CFG_DEF = re.compile(r'^\s*config\s+(\w+)\s*$', re.M)
FUNC_DECL = re.compile(
    r'^\s*(?:extern\s+)?'
    r'(?:const\s+)?'
    r'(?:struct\s+\w+|union\s+\w+|enum\s+\w+|'
    r'void|int|unsigned(?:\s+\w+)?|u8|u16|u32|u64|s8|s16|s32|s64|'
    r'bool|char|long|short|size_t|ssize_t|phys_addr_t|dma_addr_t|'
    r'uint\d+_t|int\d+_t|__\w+|ktime_t|int\s+\w+)\s*\*?\s*'
    r'(\w+)\s*\([^;{}]*\)\s*;', re.M)


def kconfig_descriptions(blobs):
    """{CONFIG_FOO: (prompt, type)}"""
    out = {}
    for path, (_, data) in blobs.items():
        if not path.endswith('Kconfig'):
            continue
        txt = data.decode(errors='ignore')
        lines = txt.splitlines()
        for i, line in enumerate(lines):
            m = CFG_DEF.match(line)
            if not m:
                continue
            name = 'CONFIG_' + m.group(1)
            prompt, typ = '', ''
            for j in range(i + 1, min(i + 8, len(lines))):
                s = lines[j].strip()
                if re.match(r'^(config|endmenu|menuconfig|source|if|endif)\b', s):
                    break
                pm = re.match(r'^(tristate|bool|int|string)\s+"?(.+?)"?\s*$', s)
                if pm and not typ:
                    typ, prompt = pm.group(1), pm.group(2).strip('"')
                elif s.startswith('prompt ') and not prompt:
                    prompt = s[7:].strip().strip('"')
                elif s.startswith('help') or s.startswith('---help---'):
                    break
            out.setdefault(name, (prompt, typ))
    return out


def parse_makefiles(blobs):
    rows = []
    for path, (_, data) in sorted(blobs.items()):
        if not path.endswith('Makefile'):
            continue
        txt = data.decode(errors='ignore')
        for m in OBJ_CFG.finditer(txt):
            cfg, objs = m.group(1), m.group(2)
            for o in objs.split():
                if o.endswith('.o'):
                    rows.append((path, cfg, o[:-2]))
        for m in OBJ_Y.finditer(txt):
            for o in m.group(1).split():
                if o.endswith('.o') and not o.startswith('/'):
                    rows.append((path, 'obj-y', o[:-2]))
    return rows


def platform_decls(baseline):
    """baseline/ 头文件里声明的函数名 -> (头文件, 原型行)"""
    decls = {}
    for dp, _, fns in os.walk(baseline):
        for fn in fns:
            if not fn.endswith('.h'):
                continue
            p = os.path.join(dp, fn)
            try:
                txt = open(p, errors='ignore').read()
            except OSError:
                continue
            for m in FUNC_DECL.finditer(txt):
                name = m.group(1)
                if name.startswith('_') or len(name) < 4:
                    continue
                decls.setdefault(name, (os.path.relpath(p, baseline),
                                        ' '.join(m.group(0).split())[:150]))
    return decls


def strip_c_comments(txt):
    """剥离 C 块注释与行注释，避免把注释掉的代码计入「真实调用」。

    stage-3 复核发现：msdc_cust.c 里 7 行 `/* buck_set_mode(VCORE, enable); */`
    曾被当成 7 次真实引用，导致该符号被误列入「链接期必须提供的 33 符号」。
    块注释必须先去掉；行注释只处理「行首/缩进后的 //」，不误伤字符串中的
    `http://` 之类内容。字符串字面量里的 `sym(` 极少，不影响统计口径。
    """
    txt = re.sub(r'/\*.*?\*/', ' ', txt, flags=re.S)
    txt = re.sub(r'(?m)^[ \t]*//.*$', ' ', txt)
    return txt


def count_refs(adapted, names):
    refs = Counter()
    name_set = set(names)
    use = re.compile(r'\b([A-Za-z_]\w{3,})\s*\(')
    for dp, _, fns in os.walk(adapted):
        for fn in fns:
            if not fn.endswith(('.c', '.h')):
                continue
            rel = os.path.relpath(os.path.join(dp, fn), adapted)
            if any(x in '/' + rel for x in ('uboot_driver', '/uboot/',
                                            'virtual_os', 'wpa_supp/src/ut')):
                continue
            try:
                txt = open(os.path.join(dp, fn), errors='ignore').read()
            except OSError:
                continue
            txt = strip_c_comments(txt)      # ← 先剥离注释再统计
            for n in use.findall(txt):
                if n in name_set:
                    refs[n] += 1
    return refs


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/ares-r-oss')
    ap.add_argument('--files', default='/tmp/ares_r_oss_files.txt')
    ap.add_argument('--baseline', default='baseline')
    ap.add_argument('--adapted', default='adapted')
    ap.add_argument('--out', default='baseline/build-files')
    ap.add_argument('--report', default='reports/BASELINE-MODULES.md')
    ap.add_argument('--tsv', default='tools/baseline-modules.tsv')
    args = ap.parse_args()

    allfiles = [x.strip() for x in open(args.files) if x.strip()]
    want = [p for p in allfiles
            if p.endswith(('Makefile', 'Kconfig'))
            and any(p.startswith(d + '/') for d in PLATFORM_DIRS)]
    print('平台层 Makefile/Kconfig: %d 个' % len(want))
    idx = load_blob_index(args.repo)
    blobs = git_read_blobs(args.repo, want, idx)
    print('保真下载: %d / %d' % (len(blobs), len(want)))

    for p, (_, data) in blobs.items():
        dst = os.path.join(args.out, p)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, 'wb').write(data)

    descs = kconfig_descriptions(blobs)
    rows = parse_makefiles(blobs)
    with open(args.tsv, 'w') as f:
        f.write('# Makefile\tKconfig开关\t目标.o\t源文件\t开关说明\n')
        for mk, cfg, mod in rows:
            src = os.path.join(os.path.dirname(mk), mod + '.c')
            desc = descs.get(cfg, ('', ''))[0] if cfg.startswith('CONFIG_') else ''
            f.write('%s\t%s\t%s.o\t%s\t%s\n' % (mk, cfg, mod, src, desc))

    decls = platform_decls(args.baseline)
    refs = count_refs(args.adapted, decls.keys())
    used = [(n, refs[n]) for n in refs if refs[n] > 0]
    used.sort(key=lambda x: -x[1])

    L = []
    L.append('# ares-r-oss 平台基线层：实现模块与链接期符号清单\n\n')
    L.append('由 `tools/scan_baseline_modules.py` 生成。'
             '`build_baseline.py` 补的是**头文件**（编译期）；'
             '本文件补的是**实现落点**（链接期）。\n\n')
    L.append('| 指标 | 值 |\n|---|---|\n')
    L.append('| 保真导出的平台层 Makefile/Kconfig | %d |\n' % len(blobs))
    L.append('| 解析出的 kbuild 目标（obj-y / obj-$(CONFIG_X)） | %d |\n' % len(rows))
    L.append('| 涉及 Kconfig 开关 | %d |\n' % len({r[1] for r in rows}))
    L.append('| baseline/ 头中声明的函数 | %d |\n' % len(decls))
    L.append('| 其中在 adapted/ 被调用（链接期需提供） | %d |\n\n' % len(used))
    L.append('## 1. 平台驱动模块（按目录分组）\n\n')
    bydir = defaultdict(list)
    for mk, cfg, mod in rows:
        bydir[os.path.dirname(mk)].append((cfg, mod))
    for d in sorted(bydir):
        L.append('### `%s`\n\n' % d)
        L.append('| Kconfig 开关 | 目标模块 | 说明 |\n|---|---|---|\n')
        for cfg, mod in sorted(bydir[d])[:60]:
            desc = descs.get(cfg, ('', ''))[0] if cfg.startswith('CONFIG_') else ''
            L.append('| `%s` | `%s.o` | %s |\n' % (cfg, mod, desc[:60]))
        L.append('\n')
    L.append('## 2. 链接期必须提供的平台导出符号（头声明 ∩ 驱动调用点）\n\n')
    L.append('下表是 `adapted/` 里真实调用、且声明来自 ares-r-oss 平台头的函数。'
             '它们是全量链接时 `undefined symbol` 的主要来源，'
             '必须由目标树「已修好的平台基线层」实现并 `EXPORT_SYMBOL_GPL`。\n\n')
    L.append('| 符号 | 调用次数 | 声明所在头 | 原型 |\n|---|---|---|---|\n')
    for n, c in used[:120]:
        h, proto = decls[n]
        L.append('| `%s` | %d | `%s` | `%s` |\n' % (n, c, h, proto.replace('|', '\\|')))
    L.append('\n## 3. 接线建议\n\n')
    L.append('1. 把 `baseline/build-files/` 里的 Makefile/Kconfig 作为**路径与开关的'
             '参考**；不要直接覆盖目标树的同名文件（4.14 kbuild 语法与 7.3 有别）。\n')
    L.append('2. defconfig 中启用上表对应 `CONFIG_*`（原厂 `ares_defconfig` 已有，'
             '见 `config/wifi-vendor-defconfig.fragment`）。\n')
    L.append('3. 这些平台驱动同样是 4.14 源码，**本身也需要 4.14→7.3 的 API 适配**；'
             '本包只导出其头文件与构建清单，不代做平台实现的适配。\n')
    L.append('4. §2 的符号若目标树已有等价实现，用 `EXPORT_SYMBOL_GPL` 暴露即可；'
             '若没有，需按 `baseline/build-files/` 的定位把对应 `.c` 一并移植。\n')
    L.append('5. 编译期接线统一用 `integration/ccflags.mk`（含 `$(src)` 与 '
             '`$(BASELINE)` 两套 -I，已补祖先目录）。\n\n')
    L.append('> 机器可读模块表：`tools/baseline-modules.tsv`；'
             '逐文件 blob sha：`tools/baseline-manifest.tsv`。\n')

    os.makedirs(os.path.dirname(args.report), exist_ok=True)
    open(args.report, 'w').writelines(L)
    print('模块 %d 行，平台符号 %d 个 -> %s' % (len(rows), len(used), args.report))


if __name__ == '__main__':
    main()
