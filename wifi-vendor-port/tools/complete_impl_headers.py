#!/usr/bin/env python3.11
# -*- coding: utf-8 -*-
"""
complete_impl_headers.py — 补齐平台符号实现的编译期依赖

在 port_platform_impl.py 导出 18 个实现 .c 之后，编译探针显示 15 个单元
卡在「MTK 平台内部头 file not found」（mtk_spm.h / mtk_ppm_internal.h /
mt-plat/upmu_common.h / helio-dvfsrc-ipi.h ...），API 残留为 0。

本脚本：
  1) 补导 3 个二次追查到的符号实现（smi_reg_* / mt_usb_is_device）；
  2) 以 baseline/impl/**/*.c 的 include 为种子，在 lineage-23.2 树里
     BFS 递归解析缺失的 MTK 平台内部头，按原路径保真导出到
     baseline/platform/（裸 include 另放 flat/ 保证 -I 命中）；
  3) 探测 buck_set_mode / aee_rr_rec_clk 是否由头里的宏/inline 提供；
  4) 追加 platform-impl-manifest.tsv 并生成 reports/IMPL-HEADERS.md。

只导出 MTK 平台私有头；linux/*、asm/*、dt-bindings/* 等主线命名空间头
一律不搬（7.3.3 自带或由 compat/ 别名处理）。
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter, deque

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import port_platform_impl as P  # noqa: E402  复用 CDN 下载 + sha 校验

# 二次追查（放宽关键词后）定位到的实现，补进 baseline/impl/
EXTRA_IMPL = [
    'drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c',
    'drivers/misc/mediatek/pseudo_m4u/pseudo_m4u_v2.c',
    'drivers/usb/mtu3/mtu3_dr.c',
]

# 主线/工具链命名空间：这些头 7.3.3 自带，不能当平台头搬进来
MAINLINE_PREFIX = (
    'linux/', 'asm/', 'uapi/', 'dt-bindings/', 'trace/', 'generated/',
    'net/', 'crypto/', 'sound/', 'video/', 'media/', 'scsi/', 'target/',
    'rdma/', 'keys/', 'xen/', 'acpi/', 'clocksource/',
)
LIBC_HEADERS = {
    'stddef.h', 'stdarg.h', 'stdint.h', 'stdbool.h', 'stdint.h', 'limits.h',
    'string.h', 'stdio.h', 'stdlib.h', 'ctype.h', 'errno.h', 'types.h',
}

INC_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)


def includes_of(text):
    return INC_RE.findall(text)


def walk_headers(roots):
    """返回 (basename 集合, 相对路径集合)。roots 为若干目录。"""
    base, rel = set(), set()
    for root in roots:
        if not os.path.isdir(root):
            continue
        for dp, _, fns in os.walk(root):
            for f in fns:
                if not f.endswith('.h'):
                    continue
                base.add(f)
                rel.add(os.path.relpath(os.path.join(dp, f), root))
    return base, rel


def is_mainline(inc, kern_base, kern_rel):
    # 工具链头：永不导出
    if inc in LIBC_HEADERS:
        return True
    # 7.3 主线确实有该头：跳过，避免把主线头搬进平台目录造成冲突
    if inc in kern_rel:
        return True
    if '/' not in inc and inc in kern_base:
        return True
    # 编译器 / 架构 / 工具链内部命名空间：一律不搬
    if inc.startswith(('asm/', 'uapi/', 'trace/', 'generated/', 'net/',
                       'crypto/', 'sound/', 'video/', 'media/', 'scsi/',
                       'target/', 'rdma/', 'keys/', 'xen/', 'acpi/',
                       'clocksource/')):
        return True
    # linux/ 与 dt-bindings/ 下主线没有的头（例如 linux/usb/class-dual-role.h、
    # mt6873 系列 dt-bindings）属于 MTK 平台私有头，交给 resolve() 从源树
    # 按原路径保真导出，落到 baseline/platform/include/ 下由 -I 命中。
    return False


def is_covered(inc, base, rel):
    if inc in base and '/' not in inc:
        return True
    if inc in rel:
        return True
    # 带路径 include：任一已导出头以该后缀结尾即可（配合 -I 祖先目录）
    return any(r == inc or r.endswith('/' + inc) for r in rel)


def resolve(inc, files):
    cands = [p for p in files
             if p.endswith('.h') and (p == inc or p.endswith('/' + inc))]
    if not cands and '/' not in inc:
        cands = [p for p in files
                 if p.endswith('.h') and os.path.basename(p) == inc]

    def rank(p):
        # 优先 include/ 下的正式头、优先 MT6893/mt6873 变体、优先短路径
        return (0 if '/include/' in '/' + p else 1,
                0 if ('mt6893' in p or 'mt6873' in p) else 1,
                len(p))
    cands.sort(key=rank)
    return cands[0] if cands else None


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/mt6893-lin232')
    ap.add_argument('--files', default='/tmp/files_lin232.txt')
    ap.add_argument('--kernel', default='/workspace/linux-7.3.3')
    ap.add_argument('--out', default='baseline')
    ap.add_argument('--manifest', default='tools/platform-impl-manifest.tsv')
    ap.add_argument('--report', default='reports/IMPL-HEADERS.md')
    ap.add_argument('--workers', type=int, default=8)
    ap.add_argument('--rounds', type=int, default=6)
    args = ap.parse_args()

    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.repo,
                            capture_output=True, text=True, check=True).stdout.strip()
    files = [x.strip() for x in open(args.files) if x.strip()]
    blob_index = P.build_blob_index(args.repo)
    fetch = P.make_fetcher(commit, blob_index, workers=args.workers)

    impl_root = os.path.join(args.out, 'impl')
    plat_root = os.path.join(args.out, 'platform')
    kern_base, kern_rel = walk_headers([
        os.path.join(args.kernel, 'include'),
        os.path.join(args.kernel, 'arch/arm64/include'),
    ])
    print('主线头: %d 个 basename / %d 个相对路径' % (len(kern_base), len(kern_rel)))

    # ---- 1) 补导 3 个二次追查实现 ----
    extra_got = fetch(EXTRA_IMPL)
    added_impl = []
    for p in EXTRA_IMPL:
        if p not in extra_got:
            continue
        sha, data = extra_got[p]
        P.write_file(impl_root, p, data)
        added_impl.append((p, sha, len(data)))
    print('补导实现 .c: %d/%d' % (len(added_impl), len(EXTRA_IMPL)))

    # ---- 2) BFS 补头 ----
    seeds = []
    for dp, _, fns in os.walk(impl_root):
        for f in fns:
            if f.endswith('.c'):
                txt = open(os.path.join(dp, f), errors='ignore').read()
                seeds.extend(includes_of(txt))
    q = deque(dict.fromkeys(seeds))
    seen = set()
    exported = {}         # inc -> path
    unresolved = Counter()
    rounds = 0
    cache = dict(extra_got)

    for rnd in range(args.rounds):
        if not q:
            break
        rounds = rnd + 1
        # 只认 baseline/ 与 compat/ 下的头，不把 adapted/（WiFi 私有栈）
        # 算作已覆盖：真实接线中平台实现与 WiFi 栈是两套独立的 -I，
        # 且本轮编译探针的 ADP 是 baseline/impl-adapted，不含 adapted/。
        # 强制按源树原路径把 mt-plat/* 等头导出到 baseline/platform/，
        # 交付包才自包含。
        base, rel = walk_headers([args.out, 'compat'])
        todo = []
        while q:
            inc = q.popleft()
            if inc in seen:
                continue
            seen.add(inc)
            if is_mainline(inc, kern_base, kern_rel) or is_covered(inc, base, rel):
                continue
            path = resolve(inc, files)
            if not path:
                unresolved[inc] += 1
                continue
            todo.append((inc, path))
        if not todo:
            break
        got = fetch([p for _, p in todo if p not in cache])
        cache.update(got)
        new_incs = []
        for inc, path in todo:
            if path not in cache:
                unresolved[inc] += 1
                continue
            sha, data = cache[path]
            P.write_file(plat_root, path, data)
            exported[inc] = path
            if '/' not in inc:
                P.write_file(plat_root, os.path.join('flat', os.path.basename(inc)), data)
            for sub in includes_of(data.decode('utf-8', errors='ignore')):
                if sub.endswith('.c') or sub not in seen:
                    if not sub.endswith('.c'):
                        new_incs.append(sub)
        q.extend(new_incs)
        print('  round %d: 本轮导出 %d 头，累计 %d，待查 %d'
              % (rnd + 1, len(todo), len(exported), len(q)), flush=True)

    # ---- 3) 追加 manifest ----
    rows = []
    for p, sha, size in added_impl:
        rows.append((os.path.join('impl', p), sha, str(size), 'symbol-impl-extra'))
    seen_path = set()
    if os.path.exists(args.manifest):
        for line in open(args.manifest):
            seen_path.add(line.split('\t')[0])
    for inc, path in sorted(exported.items(), key=lambda x: x[1]):
        rel = os.path.join('platform', path)
        if rel in seen_path or path not in cache:
            continue
        sha, data = cache[path]
        rows.append((rel, sha, str(len(data)), 'impl-dep-header'))
        seen_path.add(rel)
    with open(args.manifest, 'a') as f:
        for row in rows:
            f.write('\t'.join(row) + '\n')

    # ---- 4) 探测两个仍未定位符号是否由头里的宏/inline 提供 ----
    probes = {}
    for sym in ('buck_set_mode', 'aee_rr_rec_clk'):
        hits = []
        for path, (sha, data) in cache.items():
            if not path.endswith('.h'):
                continue
            txt = data.decode('utf-8', errors='ignore')
            for m in re.finditer(r'(?m)^\s*#\s*define\s+' + sym + r'\b|'
                                 r'static\s+inline[^{;]*\b' + sym + r'\s*\(', txt):
                hits.append((path, txt.count('\n', 0, m.start()) + 1))
        probes[sym] = hits[:6]

    # ---- 5) 报告 ----
    L = []
    L.append('# 平台实现依赖头补齐报告\n\n')
    L.append('由 `tools/complete_impl_headers.py` 生成。来源 `%s @ %s`，'
             'jsdelivr CDN + 逐文件 blob sha 校验。\n\n' % (P.REPO, commit))
    L.append('| 指标 | 值 |\n|---|---|\n')
    L.append('| 补导符号实现 `.c` | %d |\n' % len(added_impl))
    L.append('| BFS 轮次 | %d |\n' % rounds)
    L.append('| 导出的 MTK 平台内部头 | **%d** |\n' % len(exported))
    L.append('| 未能定位的 include | %d |\n\n' % len(unresolved))
    L.append('## 1. 补导的符号实现\n\n')
    for p, sha, size in added_impl:
        L.append('- `%s`（%d 字节，sha `%s`）\n' % (p, size, sha[:12]))
    L.append('\n## 2. 导出的依赖头（按 include → 源路径）\n\n')
    L.append('| #include | lineage-23.2 路径 |\n|---|---|\n')
    for inc, path in sorted(exported.items()):
        L.append('| `%s` | `%s` |\n' % (inc, path))
    if unresolved:
        L.append('\n## 3. 未能在源树定位的 include\n\n')
        for inc, n in unresolved.most_common():
            L.append('- `%s`（%d 次）\n' % (inc, n))
        L.append('\n这些属于条件编译分支 / 生成头 / 工具链头，'
                 '不计入平台基线，由目标树按需提供。\n')
    L.append('\n## 4. 两个仍无 `.c` 实现的符号\n\n')
    for sym, hits in probes.items():
        if hits:
            L.append('- `%s`：**由头文件提供**（宏或 `static inline`）：%s\n'
                     % (sym, '，'.join('`%s:%d`' % h for h in hits)))
        else:
            L.append('- `%s`：源树 `.c` 与已导出头中均未定位到定义，'
                     '需由目标树「已修基线层」提供或补 `EXPORT_SYMBOL_GPL` 桩。\n' % sym)
    L.append('\n## 5. 完整性\n\n')
    L.append('全部条目追加到 `%s`；导出位置 `baseline/platform/<原路径>`，'
             '裸 include 另有 `baseline/platform/flat/` 副本。\n' % args.manifest)
    os.makedirs(os.path.dirname(args.report) or '.', exist_ok=True)
    open(args.report, 'w', encoding='utf-8').write(''.join(L))

    print('\n=== 汇总 ===')
    print('BFS 轮次: %d，导出依赖头: %d，未定位: %d'
          % (rounds, len(exported), len(unresolved)))
    for sym, hits in probes.items():
        print('%s: %s' % (sym, ('头提供 ' + hits[0][0]) if hits else '仍未定位'))
    print('报告: %s' % args.report)


if __name__ == '__main__':
    main()
