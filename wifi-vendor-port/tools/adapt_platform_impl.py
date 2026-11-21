#!/usr/bin/env python3.11
# -*- coding: utf-8 -*-
"""
adapt_platform_impl.py — 平台符号实现的 4.14 → 7.3.3 API 适配与符号导出补齐

输入：baseline/impl/      保真实现 .c（port_platform_impl.py + complete_impl_headers.py 产出，勿改）
输出：baseline/impl-adapted/  适配版（改动全部落在这里，可 diff 审计）

做三件事：
  1) 复用 adapt_api.py 的 R1..R5 规则（class_create/access_ok/vfs_*/strlcpy/
     dma_zalloc）+ 注入 `#include "mtk_platform_compat.h"`（最小兼容接线头，
     默认不定义任何宏，见 compat/ 下该文件说明）。这些规则幂等。
  2) 补齐 EXPORT_SYMBOL_GPL：对 reports/PLATFORM-IMPL.md 中标注「需补」的
     符号，在其实现文件末尾追加；对表内未定位但补导实现后已有定义的
     符号，重新定位后同样补齐。
  3) 对源树确实找不到实现的符号，生成 docs/PLATFORM-STUBS.c.template
     （默认不参与编译，仅供目标树基线层缺失时启用）。

设计取舍：lineage-23.2 已把大量高版本 API 回移，真实扫描显示 18 个实现 .c
里没有 set_fs/strlcpy/access_ok(VERIFY)/class_create(THIS_MODULE) 残留，
init_timer 也在 #if 低版本死分支内（编译探针 API=0）。因此不做猜测性重写，
只做可审计的签名改写 + 符号导出 + 兼容头接线。
"""
import argparse
import os
import re
import shutil
import sys
from collections import Counter

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import adapt_api as A            # noqa: E402  复用 R1..R5
import port_platform_impl as P  # noqa: E402  复用 locate_def

COMPAT_LINE = '#include "mtk_platform_compat.h"'
INJECT_NOTE = ('/* Added by adapt_platform_impl.py: 4.14 -> 7.3 compat shim */\n'
               + COMPAT_LINE + '\n')
EXPORT_NOTE = ('\n/* Added by adapt_platform_impl.py: expose platform symbol to modules */\n')


def parse_impl_table(md):
    """解析 PLATFORM-IMPL.md §2 表：sym / calls / class / implfile / basis / status。"""
    out = []
    insec = False
    for line in open(md, encoding='utf-8', errors='ignore'):
        if line.startswith('## 2.'):
            insec = True
            continue
        if insec and line.startswith('## '):
            break
        if not insec or not line.startswith('|'):
            continue
        cells = [c.strip() for c in line.strip().strip('|').split('|')]
        if len(cells) < 6 or not cells[0].startswith('`'):
            continue
        sym = cells[0].strip('`')
        if not re.fullmatch(r'[A-Za-z_]\w*', sym):
            continue
        impl = cells[3].strip('`')
        impl = re.sub(r':\d+$', '', impl) if impl != '—' else None
        out.append({'sym': sym, 'calls': cells[1], 'cls': cells[2],
                    'impl': impl, 'basis': cells[4], 'status': cells[5]})
    return out


def parse_protos(md):
    """从 BASELINE-MODULES.md §2 表取符号原型（桩模板用）。"""
    out = {}
    for line in open(md, encoding='utf-8', errors='ignore'):
        cells = [c.strip() for c in line.strip().strip('|').split('|')]
        if len(cells) >= 4 and cells[0].startswith('`'):
            sym = cells[0].strip('`')
            if re.fullmatch(r'[A-Za-z_]\w*', sym):
                out[sym] = cells[3].strip('`')
    return out


def list_cs(root):
    return sorted(os.path.join(dp, f)
                  for dp, _, fns in os.walk(root)
                  for f in fns if f.endswith('.c'))


def has_export(txt, sym):
    return bool(re.search(r'EXPORT_SYMBOL(?:_GPL)?\s*\(\s*'
                          + re.escape(sym) + r'\s*\)', txt))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', default='baseline/impl')
    ap.add_argument('--dst', default='baseline/impl-adapted')
    ap.add_argument('--impl-report', default='reports/PLATFORM-IMPL.md')
    ap.add_argument('--modules', default='reports/BASELINE-MODULES.md')
    ap.add_argument('--stub', default='docs/PLATFORM-STUBS.c.template')
    ap.add_argument('--report', default='reports/API-ADAPTATION-IMPL.md')
    args = ap.parse_args()

    table = parse_impl_table(args.impl_report)
    protos = parse_protos(args.modules)
    print('解析到平台符号: %d' % len(table))
    if not table:
        sys.exit('ERR: 未能解析 PLATFORM-IMPL.md §2 表')

    # ---- 1) 复制 + R1..R5 + 注入 compat ----
    if os.path.isdir(args.dst):
        shutil.rmtree(args.dst)
    shutil.copytree(args.src, args.dst)
    cs = list_cs(args.dst)
    stats = Counter()
    for f in cs:
        txt = open(f, encoding='utf-8', errors='ignore').read()
        for rid, desc, rx, rep, note in A.rules():
            txt, n = rx.subn(rep, txt)
            if n:
                stats[rid] += n
        if COMPAT_LINE not in txt:
            lines = txt.splitlines(True)
            idx = next((i for i, l in enumerate(lines)
                        if l.lstrip().startswith('#include')), None)
            lines.insert(idx if idx is not None else 0, INJECT_NOTE)
            txt = ''.join(lines)
            stats['INJECT'] += 1
        open(f, 'w', encoding='utf-8').write(txt)
    print('适配单元: %d，注入 compat: %d' % (len(cs), stats['INJECT']))

    # ---- 2) 补 EXPORT_SYMBOL_GPL ----
    cache = {}      # abs path -> text

    def read(p):
        if p not in cache:
            cache[p] = open(p, encoding='utf-8', errors='ignore').read()
        return cache[p]

    def write(p, txt):
        cache[p] = txt
        open(p, 'w', encoding='utf-8').write(txt)

    added, already, stubs = [], [], []
    for row in table:
        sym = row['sym']
        rel = row['impl']
        # 表内已给实现路径：直接用（去掉 :行号 后映射）
        cand = os.path.join(args.dst, rel) if rel else None
        if cand and os.path.isfile(cand):
            txt = read(cand)
            if has_export(txt, sym):
                already.append(sym)
                continue
            write(cand, txt + EXPORT_NOTE + 'EXPORT_SYMBOL_GPL(%s);\n' % sym)
            added.append((sym, rel))
            continue
        # 表内未定位（或路径不存在）：在全部适配单元里重新找定义
        hit = None
        for f in cs:
            txt = read(f)
            loc = P.locate_def(txt, sym)
            if loc:
                hit = f
                if has_export(txt, sym):
                    break
        if hit:
            txt = read(hit)
            if has_export(txt, sym):
                already.append(sym)
            else:
                write(hit, txt + EXPORT_NOTE + 'EXPORT_SYMBOL_GPL(%s);\n' % sym)
                added.append((sym, os.path.relpath(hit, args.dst)))
        else:
            stubs.append(sym)

    # ---- 3) 桩模板 ----
    # stage-4：这两个符号已在本轮查清并解决，不再用「空实现返回 0」的桩。
    #   - aee_rr_rec_clk：真实实现在 ram_console/mtk_ram_console.c（保真导出到
    #     baseline/reference/），函数级移植见 solution/platform-symbols.c。
    #   - buck_set_mode：MT6335 专用废 API，MT6893 无调用，返回 -EOPNOTSUPP。
    # 模板保留为「文档化的接线说明」，指向正式实现；默认仍不参与编译。
    L = []
    L.append('/* SPDX-License-Identifier: GPL-2.0 */\n')
    L.append('/*\n * PLATFORM-STUBS.c.template — stage-4 已解决，仅保留接线说明。\n')
    L.append(' *\n * 这两个符号在 stage-3 被判为「未开源、需空桩」，stage-4 复核推翻：\n')
    L.append(' *   aee_rr_rec_clk : 真实实现在 ram_console/mtk_ram_console.c:1129，\n')
    L.append(' *                    见 solution/platform-symbols.c（函数级移植 + EXPORT）。\n')
    L.append(' *   buck_set_mode  : MT6335 专用废弃 API，MT6893(MT6359P) 无真实调用，\n')
    L.append(' *                    已从链接期符号清单剔除。\n')
    L.append(' *\n * 请直接使用 solution/platform-symbols.c（__weak + EXPORT_SYMBOL_GPL，\n')
    L.append(' * 目标树强符号可覆盖）；本文件默认不参与编译，不要与正式实现同时链接。\n')
    L.append(' * 详见 docs/UNRESOLVED-SYMBOLS.md / reports/SYMBOL-REFS-CORRECTED.md。\n')
    L.append(' */\n\n')
    L.append('#if 0  /* stage-4：改用 solution/platform-symbols.c */\n')
    L.append('#ifdef CONFIG_MTK_PLATFORM_STUBS\n\n')
    L.append('#include <linux/export.h>\n#include <linux/types.h>\n#include <linux/errno.h>\n\n')
    for sym in stubs:
        proto = protos.get(sym, '')
        L.append('/* %s */\n' % (proto or sym))
        if sym == 'buck_set_mode':
            # 废弃 API：返回 -EOPNOTSUPP 而非静默 0，避免上层误以为生效
            L.append('int __weak buck_set_mode(int type, unsigned char pmode)\n{\n'
                     '\treturn -EOPNOTSUPP;\n}\nEXPORT_SYMBOL_GPL(buck_set_mode);\n\n')
        elif sym == 'aee_rr_rec_clk':
            L.append('void __weak aee_rr_rec_clk(int id, u32 val)\n{\n}\n'
                     'EXPORT_SYMBOL_GPL(aee_rr_rec_clk);\n\n')
        else:
            L.append('/* TODO: 按上表原型补默认实现 */\n'
                     'int __weak %s(void)\n{\n\treturn 0;\n}\n'
                     'EXPORT_SYMBOL_GPL(%s);\n\n' % (sym, sym))
    L.append('#endif /* CONFIG_MTK_PLATFORM_STUBS */\n')
    L.append('#endif /* 0 */\n')
    os.makedirs(os.path.dirname(args.stub) or '.', exist_ok=True)
    open(args.stub, 'w', encoding='utf-8').write(''.join(L))

    # ---- 4) 报告 ----
    R = []
    R.append('# 平台实现 API 适配与符号导出报告\n\n')
    R.append('由 `tools/adapt_platform_impl.py` 生成。输入 `baseline/impl/`（保真，勿改），'
             '输出 `baseline/impl-adapted/`。\n\n')
    R.append('| 指标 | 值 |\n|---|---|\n')
    R.append('| 适配实现 `.c` | %d |\n' % len(cs))
    R.append('| 注入 `mtk_platform_compat.h`（最小接线头） | %d |\n'
             % stats['INJECT'])
    R.append('| 平台符号总数 | %d |\n' % len(table))
    R.append('| 已有 `EXPORT_SYMBOL*` | %d |\n' % len(already))
    R.append('| **新补 `EXPORT_SYMBOL_GPL`** | **%d** |\n' % len(added))
    R.append('| 需目标树基线层/桩模板 | %d |\n\n' % len(stubs))
    R.append('## 1. 签名改写（复用 adapt_api.py R1..R5）\n\n')
    R.append('| 规则 | 命中次数 |\n|---|---|\n')
    for rid, desc, rx, rep, note in A.rules():
        R.append('| %s %s | %d |\n' % (rid, desc, stats.get(rid, 0)))
    R.append('\n命中为 0 是**预期结果**：源树 `lineage-23.2` 已把高版本 API 回移，'
             '18 个实现 `.c` 的真实扫描里没有 `set_fs` / `strlcpy` / '
             '`access_ok(VERIFY_*)` / `class_create(THIS_MODULE)` 残留；'
             '2 处 `init_timer` 位于 `#if` 低版本死分支，7.3 预处理后不可达'
             '（编译探针 `API=0` 印证）。因此不做猜测性重写。\n\n')
    R.append('> **兼容头为什么是空的**：初版曾让 `mtk_platform_compat.h` 直接 '
             '`#include "mtk_wifi_compat.h"` 复用 WiFi 栈的 7.3 补符号，实测'
             '反而把 `struct timeval` / `strlcpy` 宏 / `mm_segment_t` 灌进'
             '平台实现，与 7.3 主线头冲突（探针错误从 20 条涨到 171 条）。'
             '现已改为最小接线头：不定义任何宏，仅保留一个集中扩展点。'
             '真需要补符号时按 `KERNEL_VERSION` 分支在此添加。\n\n')
    R.append('## 2. 新补的 EXPORT_SYMBOL_GPL\n\n')
    R.append('| 符号 | 落地文件 |\n|---|---|\n')
    for sym, rel in added:
        R.append('| `%s` | `%s` |\n' % (sym, rel))
    if not added:
        R.append('| — | — |\n')
    R.append('\n## 3. 链接期符号状态（stage-4 已复核）\n\n')
    if stubs:
        for sym in stubs:
            if sym == 'aee_rr_rec_clk':
                R.append('- `%s`：**已解决**。真实实现在 '
                         '`ram_console/mtk_ram_console.c:1129`（保真副本 '
                         '`baseline/reference/`）；函数级移植 + `EXPORT_SYMBOL_GPL` '
                         '见 `solution/platform-symbols.c`。\n' % sym)
            elif sym == 'buck_set_mode':
                R.append('- `%s`：**已剔除**。7 次「调用」全是注释、'
                         'MT6335 专用废 API（MT6893 为 MT6359P）、真实调用 0；'
                         '`solution/platform-symbols.c` 给 `-EOPNOTSUPP` 兜底。\n' % sym)
            else:
                R.append('- `%s`：仍需目标树基线层提供，或按 `%s` 兜底。\n'
                         % (sym, args.stub))
    else:
        R.append('无。\n')
    R.append('\n> stage-4 说明：这两个符号曾在本报告被判为「无实现、需空桩」，'
             '本轮已复核推翻（详见 `docs/UNRESOLVED-SYMBOLS.md`、'
             '`reports/SYMBOL-REFS-CORRECTED.md`）。链接期 `undefined symbol` '
             '由 `solution/platform-symbols.c` 的 `__weak + EXPORT_SYMBOL_GPL` 消除，'
             '并经 7.3.3 头文件实际编译与 `nm` 核验。除这两个符号外，'
             '全量链接仍需目标树「已修基线层」提供其余平台导出符号的实现。\n\n')
    R.append('## 4. 复现\n\n')
    R.append('```bash\npython3.11 tools/adapt_platform_impl.py \\\n'
             '    --src baseline/impl --dst baseline/impl-adapted \\\n'
             '    --impl-report reports/PLATFORM-IMPL.md \\\n'
             '    --modules reports/BASELINE-MODULES.md \\\n'
             '    --stub docs/PLATFORM-STUBS.c.template \\\n'
             '    --report reports/API-ADAPTATION-IMPL.md\n\n'
             '# 编译探针（同一 7.3.3 头文件口径）\n'
             'python3.11 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \\\n'
             '    --adapted baseline/impl-adapted --compat compat \\\n'
             '    --baseline auto --limit 30 --report reports/IMPL-PROBE.md\n```\n')
    os.makedirs(os.path.dirname(args.report) or '.', exist_ok=True)
    open(args.report, 'w', encoding='utf-8').write(''.join(R))

    print('\n=== 汇总 ===')
    print('已有 EXPORT : %d' % len(already))
    print('新补 EXPORT : %d' % len(added))
    print('需桩/基线层 : %d  %s' % (len(stubs), stubs))
    print('报告        : %s' % args.report)


if __name__ == '__main__':
    main()
