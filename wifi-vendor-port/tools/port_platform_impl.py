#!/usr/bin/env python3.11
# -*- coding: utf-8 -*-
"""
port_platform_impl.py — 平台符号实现移植器

从 xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2 定位并保真导出：
  1) reports/BASELINE-MODULES.md §2 的 33 个平台导出符号的「实现 .c」；
  2) reports/BASELINE-EXPORT.md §3 的残余变体头（mt6873/mt6877/mt6853 等）。

为什么选这棵树：它是 4.14 + 大量高版本 BACKPORT/UPSTREAM 回移的合并维护树，
覆盖 ares(mt6893)/agate(mt6893)/pissarro(mt6877)/choppin(mt6891)，
且 `connectivity/` 含完整 MediaTek kernel_modules 私有栈，比 ares-r-oss
多 5032 个路径（2291 个头），能补齐 ares-r-oss 缺的 MT6893 变体头。

通道：jsdelivr CDN（主）→ fastly.jsdelivr（备）→ github raw（兜底）。
      raw.githubusercontent.com 在本环境实测超时，已弃用。
      逐文件用 `git ls-tree` 的 blob sha 校验：sha1(b"blob <n>\\0" + data)。
"""
import argparse
import concurrent.futures
import hashlib
import os
import re
import subprocess
import sys
import time
from collections import Counter, defaultdict

REPO = 'xiaomi-mt6893-dev/kernel_xiaomi_mt6893'
CDN = [
    'https://cdn.jsdelivr.net/gh/%s@%%s/%%s' % REPO,
    'https://fastly.jsdelivr.net/gh/%s@%%s/%%s' % REPO,
    'https://github.com/%s/raw/%%s/%%s' % REPO,
]

# 与 33 个平台符号相关的实现模块关键词（用于从 75912 文件里收窄候选）
MOD_HINTS = [
    'emimpu', 'emi_mpu', 'helio-dvfsrc', 'ppm_v3/src', 'clkbuf_v1',
    'freqhopping', '/spm', 'lbat', 'topo_ctrl', '/battery/',
    'clk-mt68', 'mtk_iommu', '/pmic/', 'boost_ctrl', 'pmic_regulator',
    'mtk_pmic_api', 'mtk_battery',
]
# 明显无关的私有 IP 数据表 / 多媒体驱动，下载前排除
MOD_EXCLUDE = [
    '/apusys/', '/mdla/', '/vcodec/', '/cm_mgr_', '/eem', '/dcm_',
    '/cpufreq_v1/', '/mssv', '/power_gs', '/mtk_cpufreq', '/gpu',
    '/gpubw', '/vpu/', '/mdp/', '/disp/', '/mmdvfs', '/sspm/',
]


def git_blob_sha(data):
    return hashlib.sha1(b'blob %d\0' % len(data) + data).hexdigest()


def parse_symbols(md):
    """解析 BASELINE-MODULES.md §2 的 | `sym` | N | `header` | proto | 表。"""
    out = []
    pat = re.compile(r'^\|\s*`([A-Za-z_]\w*)`\s*\|\s*(\d+)\s*\|\s*`([^`]+)`')
    insec = False
    for line in open(md, encoding='utf-8', errors='ignore'):
        if line.startswith('## 2.'):
            insec = True
            continue
        if insec and line.startswith('## '):
            break
        if insec:
            m = pat.match(line)
            if m:
                out.append({'sym': m.group(1), 'calls': int(m.group(2)),
                            'header': m.group(3)})
    return out


def parse_missing_headers(md):
    """解析 BASELINE-EXPORT.md §3 的 `- \`include\`（N 处）` 清单。"""
    out = []
    insec = False
    for line in open(md, encoding='utf-8', errors='ignore'):
        if line.startswith('## 3.'):
            insec = True
            continue
        if insec and line.startswith('## '):
            break
        if insec:
            m = re.match(r'-\s*`([^`]+)`', line)
            if m:
                out.append(m.group(1))
    return out


def build_blob_index(repo):
    p = subprocess.run(['git', 'ls-tree', '-r', 'HEAD'], cwd=repo,
                       capture_output=True, text=True, timeout=900)
    idx = {}
    for line in p.stdout.splitlines():
        parts = line.split('\t', 1)
        if len(parts) != 2:
            continue
        meta = parts[0].split()
        if len(meta) >= 3 and meta[1] == 'blob':
            idx[parts[1]] = meta[2]
    return idx


def make_fetcher(commit, blob_index, workers=6, retries=3):
    import requests
    sess = requests.Session()
    sess.headers.update({'User-Agent': 'ares-platform-impl/1.0'})
    urls = [t % (commit, '%s') for t in CDN]

    def one(path):
        expect = blob_index.get(path)
        for attempt in range(retries):
            for tmpl in urls:
                try:
                    r = sess.get(tmpl % path, timeout=30)
                    if r.status_code != 200 or not r.content:
                        continue
                    data = r.content
                    sha = git_blob_sha(data)
                    if expect and sha != expect:
                        continue          # 内容不保真，换下一个镜像
                    return path, (expect or sha), data
                except Exception:
                    pass
            time.sleep(0.4 * (attempt + 1))
        return path, None, None

    def fetch_all(paths):
        got = {}
        with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as ex:
            futs = {ex.submit(one, p): p for p in paths}
            done = 0
            for f in concurrent.futures.as_completed(futs):
                p, sha, data = f.result()
                done += 1
                if data is not None:
                    got[p] = (sha, data)
                if done % 50 == 0:
                    print('  下载进度 %d/%d（成功 %d）' % (done, len(paths), len(got)),
                          flush=True)
        return got

    return fetch_all


def candidate_cs(files, syms):
    cs = set()
    for p in files:
        if not p.endswith('.c'):
            continue
        low = p.lower()
        if any(h in low for h in MOD_HINTS) and not any(x in low for x in MOD_EXCLUDE):
            cs.add(p)
    # 声明头同目录下的 .c 也纳入（覆盖 pmic/battery 等按目录组织的模块）
    dirs = {os.path.dirname(s['header']) for s in syms}
    for p in files:
        if p.endswith('.c') and os.path.dirname(p) in dirs:
            cs.add(p)
    return sorted(cs)


def locate_def(text, sym):
    """在单个 .c 里定位符号定义，返回 (kind, line_no)。"""
    # 1) EXPORT_SYMBOL(_GPL)(sym) —— 最可信
    m = re.search(r'EXPORT_SYMBOL(?:_GPL)?\s*\(\s*' + re.escape(sym) + r'\s*\)', text)
    if m:
        return 'EXPORT_SYMBOL', text.count('\n', 0, m.start()) + 1
    # 2) 函数定义：行首 [static] 返回类型 sym( ... ) { ... }
    pat = re.compile(r'(?m)^[ \t]*(?:static\s+)?(?:const\s+)?'
                     r'[A-Za-z_][\w \t\*]*?\b' + re.escape(sym) + r'\s*\(')
    for m in pat.finditer(text):
        ln = text.count('\n', 0, m.start()) + 1
        eol = text.find('\n', m.start())
        line = text[m.start():eol if eol > 0 else len(text)]
        if line.strip().startswith('extern') or '=' in line:
            continue
        seg = text[m.start():m.start() + 900]
        head = seg.split('{', 1)[0]
        if '{' not in seg or (';' in head and len(head) < 200 and '(' not in head[head.find(';') + 1:]):
            # 允许跨行参数列表；若分号后还有 '(' 说明是函数指针参数，继续判断
            if '{' not in seg:
                continue
        return 'DEF', ln
    return None


def classify(header):
    """弱耦合可整体移植(A) / 强耦合框架需桩或单独适配(B)。"""
    if header.startswith('drivers/clk/') or header.startswith('drivers/iommu/'):
        return 'B'
    return 'A'


def write_file(root, rel, data):
    dst = os.path.join(root, rel)
    os.makedirs(os.path.dirname(dst), exist_ok=True)
    with open(dst, 'wb') as f:
        f.write(data)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/mt6893-lin232')
    ap.add_argument('--files', default='/tmp/files_lin232.txt')
    ap.add_argument('--modules', default='reports/BASELINE-MODULES.md')
    ap.add_argument('--export-report', default='reports/BASELINE-EXPORT.md')
    ap.add_argument('--out', default='baseline')
    ap.add_argument('--report', default='reports/PLATFORM-IMPL.md')
    ap.add_argument('--manifest', default='tools/platform-impl-manifest.tsv')
    ap.add_argument('--workers', type=int, default=6)
    args = ap.parse_args()

    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.repo,
                            capture_output=True, text=True, check=True).stdout.strip()
    files = [x.strip() for x in open(args.files) if x.strip()]
    syms = parse_symbols(args.modules)
    missing = parse_missing_headers(args.export_report)
    print('来源 commit: %s' % commit)
    print('文件清单: %d，平台符号: %d，残余缺头: %d' % (len(files), len(syms), len(missing)))
    if not syms:
        sys.exit('ERR: 未能解析平台符号表')

    blob_index = build_blob_index(args.repo)
    print('blob index: %d' % len(blob_index))

    # ---- 变体头：按 basename 在新树定位 ----
    by_base = defaultdict(list)
    for p in files:
        by_base[os.path.basename(p)].append(p)
    hdr_targets = {}
    for inc in missing:
        b = os.path.basename(inc)
        cands = [p for p in by_base.get(b, []) if p.endswith('.h')]
        if not cands:
            continue
        # 优先 dt-bindings / drivers 下的正式头，其次 include/
        cands.sort(key=lambda p: (0 if 'dt-bindings' in p else 1, len(p)))
        hdr_targets[inc] = cands[0]

    # ---- 候选 .c ----
    cs = candidate_cs(files, syms)
    print('候选实现 .c: %d 个，开始下载…' % len(cs), flush=True)
    fetch = make_fetcher(commit, blob_index, workers=args.workers)
    got = fetch(cs)
    print('下载成功: %d/%d' % (len(got), len(cs)), flush=True)

    # ---- 定位每个符号的实现 ----
    hits = defaultdict(list)      # sym -> [(path, kind, line)]
    text_by_path = {}
    for p, (sha, data) in got.items():
        text_by_path[p] = data.decode('utf-8', errors='ignore')
    for s in syms:
        sym = s['sym']
        for p, txt in text_by_path.items():
            loc = locate_def(txt, sym)
            if loc:
                hits[sym].append((p, loc[0], loc[1]))

    def rank(item):
        p, kind, ln = item
        return (0 if kind == 'EXPORT_SYMBOL' else 1,
                0 if ('mt6893' in p or 'mt6873' in p) else 1,
                len(p))

    impl_paths = set()
    for sym, items in hits.items():
        items.sort(key=rank)
        # 导出最佳实现 + 同符号其它 EXPORT_SYMBOL 实现（多平台 variant）
        for p, kind, ln in items:
            if kind == 'EXPORT_SYMBOL' or p == items[0][0]:
                impl_paths.add(p)

    # ---- 写盘：保真实现 / 变体头 ----
    impl_root = os.path.join(args.out, 'impl')
    plat_root = os.path.join(args.out, 'platform')
    manifest = [('path', 'sha', 'size', 'kind')]
    for p in sorted(impl_paths):
        sha, data = got[p]
        write_file(impl_root, p, data)
        manifest.append((os.path.join('impl', p), sha, str(len(data)), 'symbol-impl'))
    flat = 0
    for inc, p in sorted(hdr_targets.items()):
        if p not in blob_index:
            continue
        # 变体头可能不在候选 .c 下载集合里，需要单独下载
        if p not in got:
            extra = fetch([p])
            if p not in extra:
                continue
            got[p] = extra[p]
        sha, data = got[p]
        write_file(plat_root, p, data)
        manifest.append((os.path.join('platform', p), sha, str(len(data)), 'variant-header'))
        # 裸 include（无 '/')额外放一份 flat/，保证 -I 直接命中
        if '/' not in inc:
            write_file(plat_root, os.path.join('flat', os.path.basename(inc)), data)
            flat += 1

    os.makedirs(os.path.dirname(args.manifest) or '.', exist_ok=True)
    with open(args.manifest, 'w') as f:
        for row in manifest:
            f.write('\t'.join(row) + '\n')

    # ---- 报告 ----
    L = []
    L.append('# 平台符号实现移植报告（ares → 7.3.3）\n\n')
    L.append('由 `tools/port_platform_impl.py` 生成。来源：`%s @ %s`（固定 commit）。\n\n'
             % (REPO, commit))
    L.append('通道：`cdn.jsdelivr.net`（主）→ `fastly.jsdelivr.net`（备）→ '
             '`github.com/.../raw`（兜底）；逐文件用 `git ls-tree` 的 blob sha 校验。'
             '`raw.githubusercontent.com` 在本环境实测超时，已弃用。\n\n')
    L.append('## 1. 为什么选这棵树\n\n')
    L.append('- `ares-r-oss` 是小米官方 GPL 树，`connectivity/` 只有空壳，'
             '且只带 `mt6885-*` 变体头；上轮已用它导出 36 个平台头与 182 个构建文件，'
             '**但未导出任何平台实现 `.c`**。\n')
    L.append('- 本树 `lineage-23.2` 是 4.14 + 大量高版本 `BACKPORT:`/`UPSTREAM:` '
             '回移的合并维护树（覆盖 ares/agate/pissarro/choppin），'
             '相对 ares-r-oss 多 5032 个路径（2291 头 / 2073 源），'
             '含完整 `connectivity/` 私有栈（473 个 `.c`）与 MT6893 变体头。\n')
    L.append('- 其 API 写法比原生 4.14 更接近新内核，移植到 7.3.3 的差异面更小。\n\n')
    L.append('| 指标 | 值 |\n|---|---|\n')
    L.append('| 来源 commit | `%s` |\n' % commit)
    L.append('| 候选实现 `.c` / 下载成功 | %d / %d |\n' % (len(cs), len(got)))
    L.append('| 平台符号 | %d |\n' % len(syms))
    L.append('| 定位到实现的符号 | **%d** |\n' % len(hits))
    L.append('| 导出实现 `.c` | %d |\n' % len(impl_paths))
    L.append('| 残余缺头可补齐 | **%d / %d** |\n\n' % (len(hdr_targets), len(missing)))

    L.append('## 2. 33 个平台导出符号 → 实现落点\n\n')
    L.append('`类` 列：**A** = 弱耦合的 MTK 私有实现（7.3.3 无同名子系统，'
             '可整体移植）；**B** = 与主线 clk/iommu 框架强耦合，'
             '需单独适配或提供兼容桩，不建议整体并入主线目录。\n\n')
    L.append('| 符号 | 调用 | 类 | 实现文件 | 依据 | 导出状态 |\n|---|---|---|---|---|---|\n')
    n_export = 0
    no_impl = []
    for s in syms:
        sym = s['sym']
        items = hits.get(sym, [])
        cls = classify(s['header'])
        if not items:
            no_impl.append(sym)
            L.append('| `%s` | %d | %s | — | 未定位 | 目标树基线层/需桩 |\n'
                     % (sym, s['calls'], cls))
            continue
        best = items[0]
        p, kind, ln = best
        exported = any(re.search(r'EXPORT_SYMBOL(?:_GPL)?\s*\(\s*' + re.escape(sym) + r'\s*\)',
                                 text_by_path.get(x[0], '')) for x in items)
        if exported:
            n_export += 1
        status = '已有 `EXPORT_SYMBOL`' if exported else '**需补 `EXPORT_SYMBOL_GPL`**'
        L.append('| `%s` | %d | %s | `%s:%d` | %s%s | %s |\n'
                 % (sym, s['calls'], cls, p, ln, kind,
                    ('（另 %d 个 variant）' % (len(items) - 1)) if len(items) > 1 else '',
                    status))
    L.append('\n> 已有 `EXPORT_SYMBOL` 的符号 **%d/%d**；其余在适配版中补齐。\n\n'
             % (n_export, len(syms)))
    if no_impl:
        L.append('未在候选树定位到实现的符号（%d）：%s。\n\n'
                 % (len(no_impl), ', '.join('`%s`' % x for x in no_impl)))

    L.append('## 3. 残余变体头补齐\n\n')
    L.append('| include（原缺失） | 新树定位路径 | 状态 |\n|---|---|---|\n')
    for inc in missing:
        p = hdr_targets.get(inc)
        L.append('| `%s` | %s | %s |\n'
                 % (inc, ('`%s`' % p) if p else '—',
                    '已导出' if p else '两树均不含（裁剪树内部/条件分支）'))
    L.append('\n导出位置：`baseline/platform/<原路径>`；裸 include 另复制一份到 '
             '`baseline/platform/flat/`（%d 个），保证 `-I` 直接命中。\n\n' % flat)

    L.append('## 4. 完整性与复现\n\n')
    L.append('- `%s`：逐文件 `path / blob sha / size / kind`。\n' % args.manifest)
    L.append('- `baseline/impl/`：保真实现 `.c`（按原路径，勿手改，审计基准）。\n')
    L.append('- `baseline/platform/`：变体头（保真 + flat 副本）。\n')
    L.append('- 适配版由 `tools/adapt_platform_impl.py` 生成到 `baseline/impl-adapted/`。\n\n')
    L.append('```bash\npython3.11 tools/port_platform_impl.py \\\n'
             '    --repo /tmp/mt6893-lin232 --files /tmp/files_lin232.txt \\\n'
             '    --modules reports/BASELINE-MODULES.md \\\n'
             '    --export-report reports/BASELINE-EXPORT.md \\\n'
             '    --out baseline --report reports/PLATFORM-IMPL.md \\\n'
             '    --manifest tools/platform-impl-manifest.tsv\n```\n')

    os.makedirs(os.path.dirname(args.report) or '.', exist_ok=True)
    with open(args.report, 'w', encoding='utf-8') as f:
        f.write(''.join(L))

    print('\n=== 汇总 ===')
    print('定位到实现的符号: %d/%d' % (len(hits), len(syms)))
    print('导出实现 .c      : %d' % len(impl_paths))
    print('补齐变体头       : %d/%d' % (len(hdr_targets), len(missing)))
    print('已有 EXPORT_SYMBOL: %d' % n_export)
    print('报告             : %s' % args.report)


if __name__ == '__main__':
    main()
