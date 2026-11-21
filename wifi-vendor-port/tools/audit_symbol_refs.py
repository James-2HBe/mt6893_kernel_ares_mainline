#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""复核「链接期 33 平台符号」的调用计数，修正注释误统计。

背景（stage-4）：
  stage-2 的 scan_baseline_modules.py::count_refs 直接对源码文本跑
  `\\b(name)\\s*\\(`，**没有剥离 C 注释**，于是 msdc_cust.c 里 7 行
  `/* buck_set_mode(VCORE, enable); */` 被当成 7 次真实调用，
  把 buck_set_mode 误列入「链接期必须提供的 33 符号」。

本工具：
  1) 从 reports/BASELINE-MODULES.md §2 读出旧表（符号 + 旧计数 + 声明头）；
  2) 用「剥离块/行注释 + 剔除 extern 声明行」的口径重新统计 adapted/；
  3) 对 buck_set_mode / aee_rr_rec_clk 单独列出全部出现位置并标注是否注释，
     以及 baseline/impl/ 里的真实调用点；
  4) 产出 reports/SYMBOL-REFS-CORRECTED.md，保留旧报告作审计对照。

只读，不改动任何源码/原报告。
"""
import argparse
import os
import re
from collections import Counter, defaultdict

OLD_TABLE = re.compile(
    r'^\|\s+`(\w+)`\s+\|\s+(\d+)\s+\|\s+`([^`]*)`\s+\|\s+`(.*?)`\s+\|', re.M)
USE = re.compile(r'\b([A-Za-z_]\w{3,})\s*\(')
DECL_LINE = re.compile(
    r'(?m)^[ \t]*(?:extern[ \t]+)?(?:static[ \t]+)?(?:inline[ \t]+)?'
    r'[A-Za-z_][\w \t\*]*?\b\w+\s*\([^;{}]*\)\s*;[ \t]*$')
SKIP = ('uboot_driver', '/uboot/', 'virtual_os', 'wpa_supp/src/ut')


def strip_c_comments(txt):
    txt = re.sub(r'/\*.*?\*/', ' ', txt, flags=re.S)
    txt = re.sub(r'(?m)^[ \t]*//.*$', ' ', txt)
    return txt


def iter_sources(root):
    for dp, _, fns in os.walk(root):
        for fn in fns:
            if not fn.endswith(('.c', '.h')):
                continue
            rel = os.path.relpath(os.path.join(dp, fn), root)
            if any(x in '/' + rel for x in SKIP):
                continue
            p = os.path.join(dp, fn)
            try:
                yield rel, open(p, errors='ignore').read()
            except OSError:
                continue


def count_real_calls(roots, names):
    name_set = set(names)
    refs = Counter()
    where = defaultdict(set)
    for root in roots:
        if not os.path.isdir(root):
            continue
        for rel, txt in iter_sources(root):
            txt = DECL_LINE.sub(' ', strip_c_comments(txt))
            for n in USE.findall(txt):
                if n in name_set:
                    refs[n] += 1
                    where[n].add(root.rstrip('/') + '/' + rel)
    return refs, where


def occurrences(roots, sym):
    """列出 sym 在源码里的每一行，标注 行内注释 / 块注释 / 声明 / 真实代码。"""
    out = []
    for root in roots:
        if not os.path.isdir(root):
            continue
        for rel, txt in iter_sources(root):
            lines = txt.splitlines()
            in_block = False
            for i, raw in enumerate(lines, 1):
                s = raw.strip()
                if sym not in raw:
                    # 仍需维护块注释状态
                    if '/*' in raw and '*/' not in raw:
                        in_block = True
                    if '*/' in raw:
                        in_block = False
                    continue
                kind = 'CODE'
                if in_block or s.startswith('/*') or s.startswith('*') or '/*' in raw:
                    kind = 'COMMENT'
                elif s.startswith('//'):
                    kind = 'COMMENT'
                elif re.search(r'\bextern\b', raw):
                    kind = 'DECL'
                elif re.search(r'\bstatic\s+inline\b', raw):
                    # 头里的空实现（CONFIG_MTK_RAM_CONSOLE=n 分支），不是调用
                    kind = 'INLINE-IMPL'
                # 其余（含以 ; 结尾的调用语句）一律按真实代码计
                out.append((root.rstrip('/') + '/' + rel, i, kind, s[:150]))
                if '/*' in raw and '*/' not in raw:
                    in_block = True
                if '*/' in raw:
                    in_block = False
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--modules', default='reports/BASELINE-MODULES.md')
    ap.add_argument('--adapted', default='adapted')
    ap.add_argument('--impl', default='baseline/impl')
    ap.add_argument('--report', default='reports/SYMBOL-REFS-CORRECTED.md')
    args = ap.parse_args()

    md = open(args.modules, errors='ignore').read()
    old = OLD_TABLE.findall(md)
    names = [r[0] for r in old]
    old_cnt = {r[0]: int(r[1]) for r in old}
    print('旧表符号数: %d' % len(old))

    roots = [args.adapted, args.impl]
    refs, where = count_real_calls(roots, names)

    changed = [(n, old_cnt[n], refs[n]) for n in names if refs[n] != old_cnt[n]]
    changed.sort(key=lambda x: x[0])

    focus = ['buck_set_mode', 'aee_rr_rec_clk']

    L = []
    L.append('# 链接期平台符号调用计数复核（注释误统计修正）\n\n')
    L.append('由 `tools/audit_symbol_refs.py` 生成，只读复核，'
             '**不改动** `reports/BASELINE-MODULES.md`，旧表保留作审计对照。\n\n')
    L.append('## 1. 问题\n\n')
    L.append('`tools/scan_baseline_modules.py::count_refs`（stage-2）直接对源码文本\n'
             '匹配 `\\b(name)\\s*\\(`，**未剥离 C 注释，也未剔除函数声明行**。'
             '因此：\n\n'
             '- `adapted/mmc/host/mediatek/ComboA/*/msdc_cust.c` 中 7 行\n'
             '  `/* buck_set_mode(VCORE, enable); */` 被计成 7 次真实调用；\n'
             '- 头文件中 `extern`/`static inline` 声明也被计入。\n\n'
             '本工具口径：**剥离块注释与行注释 → 剔除以 `;` 结尾的声明行 → 再统计**，\n'
             '并把扫描范围从 `adapted/` 扩展到 `adapted/ + baseline/impl/`\n'
             '（stage-3 导出的平台实现 .c 也在其中）。\n\n')
    L.append('| 指标 | 值 |\n|---|---|\n')
    L.append('| 旧表符号数 | %d |\n' % len(old))
    L.append('| 计数发生变化的符号 | %d |\n' % len(changed))
    L.append('| 修正后真实调用数为 0（可从链接期清单移除） | %d |\n\n'
             % sum(1 for n, o, r in changed if r == 0))
    L.append('## 2. 计数变化明细（旧 → 新）\n\n')
    L.append('| 符号 | 旧计数 | 修正后 | 说明 |\n|---|---|---|---|\n')
    for n, o, r in changed:
        note = ('全部为注释/声明，无真实调用' if r == 0
                else '含声明或注释，真实调用 %d 处' % r)
        L.append('| `%s` | %d | **%d** | %s |\n' % (n, o, r, note))
    L.append('\n## 3. 两个「未开源符号」专项结论\n\n')
    for sym in focus:
        occ = occurrences(roots, sym)
        n_comment = sum(1 for x in occ if x[2] == 'COMMENT')
        n_decl = sum(1 for x in occ if x[2] in ('DECL', 'INLINE-IMPL'))
        n_code = sum(1 for x in occ if x[2] == 'CODE')
        L.append('### `%s`\n\n' % sym)
        L.append('出现 %d 行：注释 %d / 声明 %d / **真实调用 %d**。'
                 '修正后计数 **%d**（旧表 %d）。\n\n'
                 % (len(occ), n_comment, n_decl, n_code, refs[sym], old_cnt.get(sym, 0)))
        L.append('| 位置 | 类型 | 内容 |\n|---|---|---|\n')
        for path, ln, kind, txt in occ[:30]:
            L.append('| `%s:%d` | %s | `%s` |\n'
                     % (path, ln, kind, txt.replace('|', '\\|')))
        if len(occ) > 30:
            L.append('| … | | 另有 %d 行省略 |\n' % (len(occ) - 30))
        L.append('\n')
    L.append('## 4. 复现\n\n```bash\n'
             'python3 tools/audit_symbol_refs.py \\\n'
             '    --modules reports/BASELINE-MODULES.md \\\n'
             '    --adapted adapted --impl baseline/impl \\\n'
             '    --report reports/SYMBOL-REFS-CORRECTED.md\n```\n')

    open(args.report, 'w').writelines(L)
    print('计数变化符号: %d' % len(changed))
    for n, o, r in changed:
        print('  %-36s %d -> %d' % (n, o, r))
    for sym in focus:
        occ = occurrences(roots, sym)
        print('%s: 出现 %d 行, 注释 %d, 声明 %d, 真实调用 %d, 新计数 %d' % (
            sym, len(occ),
            sum(1 for x in occ if x[2] == 'COMMENT'),
            sum(1 for x in occ if x[2] in ('DECL', 'INLINE-IMPL')),
            sum(1 for x in occ if x[2] == 'CODE'), refs[sym]))
    print('报告 ->', args.report)


if __name__ == '__main__':
    main()
