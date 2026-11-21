#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
build_baseline.py — 用小米官方开源内核 ares-r-oss 补齐 WiFi 私有栈的平台基线层。

背景：seriaTvT/kernel_mt6893 是裁剪树，只导出了 connectivity 相关源码，缺
mt-plat / base/power / pmic / emi / m4u / devfreq / eccci 等平台层头文件，
导致 adapted/ 单文件编译普遍卡在 "xxx.h file not found"。
MiCode/Xiaomi_Kernel_OpenSource@ares-r-oss 是完整 MTK 平台树（connectivity
只有 6 个空壳 adapter），其平台层正好可作基线来源。

算法（BFS，幂等）：
 1) 扫描 adapted/ 的 #include，取「不在 adapted/、也不在目标主线树」的私有头作种子；
 2) 在 ares-r-oss 全文件清单里按 路径后缀 -> basename 回退 解析候选，打分选最优；
 3) 用 `git cat-file --batch` 按固定 commit 读取 blob（保真），按原路径写入 baseline/；
 4) 解析每个新头的 #include，把新的私有头继续入队，直到收敛；
 5) 产出 manifest（blob sha）、映射/歧义/未解析报告、以及 -I 接线清单。

用法：
  python3 tools/build_baseline.py \
      --repo /tmp/ares-r-oss --files /tmp/ares_r_oss_files.txt \
      --kernel /workspace/linux-7.3.3 --adapted adapted \
      --out baseline --report reports/BASELINE-EXPORT.md \
      --ccflags integration/baseline-ccflags.mk
"""
import argparse
import os
import re
import subprocess
import sys
from collections import Counter, defaultdict, deque

INC_RE = re.compile(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', re.M)

# 非内核编译单元：这些目录里的 include 不参与基线依赖（libc / bootloader / 模拟器）
SKIP_UNIT = ('uboot_driver', '/uboot/', 'virtual_os', 'uart_daemon',
             'wpa_supp/src/ut', '/none/include/', '/os/none/')
# 明确的 libc / 工具链 / 测试桩头，不是 MTK 平台基线
LIBC_HINT = re.compile(
    r'^(stdio|stdlib|stdint|stdbool|string|malloc|inttypes|errno|assert|time|'
    r'limits|stdarg|stddef|ctype|math|unistd|fcntl|sys/|machine/|winsock|'
    r'valgrind/|MsTypes|ShareType|CusConfig|MsVfs|MsDebug|MsSystem|drvPM|'
    r'loader_if|usb_type|simulator_kernel|ut_lib|fde_aes)', re.I)
# ares-r-oss 中与 WiFi 平台基线无关的大块，防 BFS 跑偏
PATH_BLACKLIST = re.compile(
    r'(drivers/misc/mediatek/(gpu|imgsensor|lens|lcm|video|apusys|vpu|mdp|'
    r'ccu|cameraisp|sensors|teei|gud|ext_disp|nand|flashlight|audio|'
    r'typec|mu3d|masp|hdmi|irrx|trusted_mem|geniezone|tkcore)/|'
    r'uboot|bootloader|/test/|/tests/|/tool/|/tools/|/sample)', re.I)
# 打分加权：ares = MT6893，优先 mt6893/mt6873 与 include/ 目录
GOOD_PATH = re.compile(
    r'(mt6893|mt6873|mt-plat|base/power|/pmic/|/emi|/m4u/|devfreq|'
    r'eccci|mddp|/include/|/clk/|/soc/mediatek|/iommu/|dvfsrc|clkbuf|'
    r'spm|ppm|battery|debug_latch)', re.I)

# include 分类（决定"补基线"的边界）
#   PLATFORM : MTK 平台私有头 -> 从 ares-r-oss 保真导出（本工具职责）
#   LEGACY   : 4.14 主线删掉的头 -> 不能把原厂文件搬进 7.3 的 linux/ 命名空间，
#              属内核 API 适配（另见 compat/ 与报告 §4）
#   INTERNAL : 私有栈内部头（空壳仓库与裁剪树都不含，多在条件编译分支）
PLATFORM_ROOT = re.compile(
    r'^(memory/|soc/mediatek/|mt-plat/|mach/|dt-bindings/|m4u/|'
    r'include/|clk-mt|pmic_|mtk_|mt_|helio-|topo_ctrl|plat_dbg_info|'
    r'conninfra|emi|dramc|tc1_|mddp|spm|ppm|dvfsrc|clkbuf|bwl|'
    r'devmpu|emi_mpu|port_ipc|ccci_|platform_mt|reset_fsm)', re.I)
LEGACY_LINUX = re.compile(r'^(linux/|asm/|uapi/|trace/|net/|crypto/|sound/)', re.I)


def classify(inc):
    if LEGACY_LINUX.match(inc):
        return 'LEGACY'
    if PLATFORM_ROOT.match(inc):
        return 'PLATFORM'
    return 'INTERNAL'


def build_index(paths):
    suffix = defaultdict(list)
    bases = defaultdict(list)
    for p in paths:
        parts = p.split('/')
        for i in range(len(parts)):
            suffix['/'.join(parts[i:])].append(p)
        bases[parts[-1]].append(p)
    return suffix, bases


def score(path, inc):
    s = 0
    if 'mt6893' in path:
        s += 12
    elif 'mt6873' in path or 'mt6885' in path:
        s += 5
    if '/include/' in path or path.startswith('include/'):
        s += 6
    if GOOD_PATH.search(path):
        s += 4
    if PATH_BLACKLIST.search(path):
        s -= 40
    if '/mt6893/' in path or '/mt6893' in path:
        s += 2
    # 同后缀时，路径越浅越可能是通用头（而非某个平台副本）
    s -= min(len(path.split('/')), 12) * 0.2
    return s


def resolve(inc, suffix, bases, allow_basename=True):
    cands = list(suffix.get(inc, []))
    used = 'suffix'
    if not cands and allow_basename:
        cands = bases.get(os.path.basename(inc), [])
        used = 'basename'
    # 带目录的 include 走 basename 回退风险高（如 soc/mediatek/emi.h
    # 会误配到 include/memory/mediatek/emi.h），默认禁用
    elif not cands and '/' in inc:
        return None, [], 'unresolved-path'
    cands = [c for c in cands if c.endswith('.h') or c.endswith('.c')]
    if not cands:
        return None, [], used
    ranked = sorted(cands, key=lambda p: -score(p, inc))
    return ranked[0], ranked, used


def load_blob_index(repo):
    """git ls-tree -r HEAD -> {path: blob_sha}（tree 已在本地，无需网络）。"""
    p = subprocess.run(['git', 'ls-tree', '-r', 'HEAD'], cwd=repo,
                       capture_output=True, text=True, check=True, timeout=300)
    idx = {}
    for line in p.stdout.splitlines():
        meta, _, path = line.partition('\t')
        parts = meta.split()
        if len(parts) >= 3 and parts[1] == 'blob':
            idx[path] = parts[2]
    return idx


def git_blob_sha(data):
    import hashlib
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def git_read_blobs(repo, paths, blob_index=None, workers=4, retries=3):
    """按固定 commit 从 raw.githubusercontent.com 保真下载 blob。

    不用 promisor lazy fetch：批量 `git cat-file --batch` 会触发
    GnuTLS "TLS connection was non-properly terminated"，实测不稳定。
    下载后按 git object hash 与 ls-tree 的 blob sha 逐文件校验。
    """
    import concurrent.futures
    import time
    import requests

    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=repo,
                            capture_output=True, text=True,
                            check=True).stdout.strip()
    base = ('https://raw.githubusercontent.com/MiCode/'
            'Xiaomi_Kernel_OpenSource/%s/' % commit)
    sess = requests.Session()
    sess.headers.update({'User-Agent': 'ares-baseline-exporter/1.0'})

    def one(path):
        for attempt in range(retries):
            try:
                r = sess.get(base + path, timeout=60)
                if r.status_code == 200 and r.content:
                    return path, r.content
            except Exception:
                pass
            time.sleep(0.6 * (attempt + 1))
        # 兜底：本地已缓存的 blob
        try:
            q = subprocess.run(['git', 'cat-file', 'blob', 'HEAD:' + path],
                               cwd=repo, capture_output=True, timeout=30)
            if q.returncode == 0 and q.stdout:
                return path, q.stdout
        except Exception:
            pass
        return path, None

    out = {}
    with concurrent.futures.ThreadPoolExecutor(max_workers=workers) as ex:
        for path, data in ex.map(one, paths):
            if data is None:
                continue
            expect = (blob_index or {}).get(path)
            sha = git_blob_sha(data)
            if expect and sha != expect:
                sys.stderr.write('WARN sha mismatch: %s\n' % path)
                continue
            out[path] = (expect or sha, data)
    return out


def kernel_headers(K):
    rel, base = set(), set()
    for root in ('include', 'arch/arm64/include', 'drivers', 'lib', 'kernel'):
        top = os.path.join(K, root)
        if not os.path.isdir(top):
            continue
        for dp, _, fns in os.walk(top):
            for f in fns:
                if not f.endswith('.h'):
                    continue
                base.add(f)
                rel.add(os.path.relpath(os.path.join(dp, f), K))
    return rel, base


def adapted_headers(ADP):
    rel, base = set(), set()
    for dp, _, fns in os.walk(ADP):
        for f in fns:
            if f.endswith('.h'):
                rel.add(os.path.relpath(os.path.join(dp, f), ADP))
                base.add(f)
    return rel, base


def includes_of(text):
    return INC_RE.findall(text)


def is_covered(inc, exported, adp_rel, adp_base, kern_rel, kern_base):
    if inc in adp_rel or os.path.basename(inc) in adp_base:
        return True
    if inc in kern_rel or os.path.basename(inc) in kern_base:
        return True
    # 已导出文件里存在以该 include 结尾的路径
    for p in exported:
        if p == inc or p.endswith('/' + inc) or p.endswith(inc):
            return True
    return False


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/ares-r-oss')
    ap.add_argument('--files', default='/tmp/ares_r_oss_files.txt')
    ap.add_argument('--kernel', default='/workspace/linux-7.3.3')
    ap.add_argument('--adapted', default='adapted')
    ap.add_argument('--out', default='baseline')
    ap.add_argument('--report', default='reports/BASELINE-EXPORT.md')
    ap.add_argument('--ccflags', default='integration/baseline-ccflags.mk')
    ap.add_argument('--max-files', type=int, default=1800)
    args = ap.parse_args()

    paths = [x.strip() for x in open(args.files) if x.strip()]
    suffix, bases = build_index(paths)
    blob_index = load_blob_index(args.repo)
    print('ares-r-oss blob 索引: %d 个文件' % len(blob_index))
    kern_rel, kern_base = kernel_headers(args.kernel)
    adp_rel, adp_base = adapted_headers(args.adapted)

    # --- 种子：adapted/ 里缺失的私有 include ---
    seeds = Counter()
    seed_from = defaultdict(set)
    for dp, _, fns in os.walk(args.adapted):
        for fn in fns:
            if not fn.endswith(('.c', '.h')):
                continue
            rel = os.path.relpath(os.path.join(dp, fn), args.adapted)
            if any(x in '/' + rel for x in SKIP_UNIT):
                continue
            try:
                txt = open(os.path.join(dp, fn), errors='ignore').read()
            except OSError:
                continue
            for inc in includes_of(txt):
                if LIBC_HINT.search(inc):
                    continue
                if inc.endswith('.c'):
                    continue
                if is_covered(inc, [], adp_rel, adp_base, kern_rel, kern_base):
                    continue
                seeds[inc] += 1
                seed_from[inc].add(rel)

    # --- 分类：只有 PLATFORM 类进入基线导出 BFS ---
    legacy = Counter()
    internal = Counter()
    for inc, n in seeds.items():
        k = classify(inc)
        if k == 'LEGACY':
            legacy[inc] += n
        elif k == 'INTERNAL':
            internal[inc] += n
    platform_seeds = [i for i in seeds if classify(i) == 'PLATFORM']
    print('种子缺失 include: %d 个（去重），累计引用 %d 次' %
          (len(seeds), sum(seeds.values())))
    print('  分类: 平台基线 %d / 4.14 主线头(API 适配) %d / 私有栈内部 %d' %
          (len(platform_seeds), len(legacy), len(internal)))

    # --- BFS ---
    q = deque(sorted(platform_seeds, key=lambda x: -seeds[x]))
    seen = set()
    exported = {}       # path -> sha
    contents = {}       # path -> bytes
    mapping = {}        # inc -> (path, mode, candidates)
    ambig = {}
    unresolved = Counter()
    rounds = 0
    while q and len(exported) < args.max_files:
        rounds += 1
        level = list(q)
        q.clear()
        for inc in level:
            if inc in seen:
                continue
            seen.add(inc)
            path, cands, mode = resolve(inc, suffix, bases,
                                       allow_basename=('/' not in inc))
            if not path:
                unresolved[inc] += seeds.get(inc, 1)
                continue
            mapping[inc] = (path, mode, len(cands))
            if len(cands) > 1 and score(cands[0], inc) - score(cands[1], inc) < 3:
                ambig[inc] = cands[:5]
            if path in exported:
                continue
            exported[path] = None
        todo = [p for p, s in exported.items() if s is None]
        if not todo:
            break
        got = git_read_blobs(args.repo, todo, blob_index)
        for p in todo:
            if p not in got:
                continue
            sha, data = got[p]
            exported[p] = sha
            contents[p] = data
            for sub in includes_of(data.decode(errors='ignore')):
                if LIBC_HINT.search(sub) or sub.endswith('.c'):
                    continue
                if is_covered(sub, list(exported.keys()), adp_rel, adp_base,
                              kern_rel, kern_base):
                    continue
                k = classify(sub)
                if k == 'LEGACY':
                    legacy[sub] += 1
                    continue
                if k == 'INTERNAL':
                    internal[sub] += 1
                    continue
                if sub not in seen:
                    q.append(sub)
        print('  round %d: 导出 %d 文件，待查 %d' % (rounds, len(exported), len(q)))

    # --- 落盘（按 ares-r-oss 原路径） ---
    written = 0
    for p, data in contents.items():
        dst = os.path.join(args.out, p)
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        with open(dst, 'wb') as f:
            f.write(data)
        written += 1

    # --- manifest ---
    man_path = os.path.join('tools', 'baseline-manifest.tsv')
    with open(man_path, 'w') as f:
        f.write('# ares-r-oss blob sha\tpath\n')
        for p in sorted(exported):
            f.write('%s\t%s\n' % (exported[p] or 'MISSING', p))

    # --- -I 接线 ---
    dirs = sorted({os.path.dirname(p) for p in exported})
    with open(args.ccflags, 'w') as f:
        f.write('# Auto-generated by build_baseline.py — ares-r-oss 平台基线层 -I 清单\n')
        f.write('# 追加到 connectivity 顶层 Makefile 的 subdir-ccflags-y；\n')
        f.write('# $(BASELINE) 指向本包 baseline/ 在目标树中的部署位置。\n')
        f.write('# 说明：路径按 ares-r-oss 原目录层级保存，故 -I 到具体目录即可命中\n')
        f.write('#       #include <memory/mediatek/emi.h> / <mach/xxx.h> 等写法。\n\n')
        for d in dirs:
            f.write('subdir-ccflags-y += -I$(BASELINE)/%s\n' % d)
    print('写出 %d 个文件，%d 个 -I 目录 -> %s' % (written, len(dirs), args.ccflags))

    # --- 报告 ---
    L = []
    L.append('# ares-r-oss 平台基线层导出报告\n\n')
    L.append('由 `tools/build_baseline.py` 生成。来源：'
             '`MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`（固定 commit），'
             '按 `raw.githubusercontent.com` 逐 blob 下载，并用 `git ls-tree` 的 '
             'blob sha 做 git object hash 校验，保证字节保真。\n\n')
    L.append('> **分类原则**：只把 **MTK 平台私有头** 当基线导出。'
             '`linux/*` 等 4.14 主线头在 7.3 已删除，**不能**把原厂文件搬进 '
             '主线的 `linux/` 命名空间覆盖，那是内核 API 适配（见 §4）；'
             '私有栈内部头不在这棵空壳仓库里（见 §5）。\n\n')
    L.append('| 指标 | 值 |\n|---|---|\n')
    L.append('| 种子缺失 include（去重） | %d |\n' % len(seeds))
    L.append('| ├ 平台基线类（导出对象） | %d |\n' % len(platform_seeds))
    L.append('| ├ 4.14 主线头（API 适配） | %d |\n' % len(legacy))
    L.append('| └ 私有栈内部头 | %d |\n' % len(internal))
    L.append('| BFS 轮次 | %d |\n' % rounds)
    L.append('| 导出平台层文件 | %d |\n' % written)
    L.append('| 生成的 -I 目录 | %d |\n' % len(dirs))
    L.append('| 已解析 include 映射 | %d |\n' % len(mapping))
    L.append('| 候选歧义（需人工确认） | %d |\n' % len(ambig))
    L.append('| 平台头未能在 ares-r-oss 定位 | %d |\n\n' % len(unresolved))
    L.append('## 1. 平台基线头 → ares-r-oss 路径映射\n\n')
    L.append('| include | 引用次数 | ares-r-oss 路径 | 匹配方式 | 候选数 |\n|---|---|---|---|---|\n')
    plat_sorted = sorted(platform_seeds, key=lambda x: -seeds[x])
    for inc, n in [(i, seeds[i]) for i in plat_sorted][:70]:
        m = mapping.get(inc)
        if m:
            L.append('| `%s` | %d | `%s` | %s | %d |\n' % (inc, n, m[0], m[1], m[2]))
        else:
            L.append('| `%s` | %d | **未定位** | - | - |\n' % (inc, n))
    L.append('\n## 2. 候选歧义（打分接近，建议人工确认平台 variant）\n\n')
    if ambig:
        for inc, c in sorted(ambig.items()):
            L.append('- `%s`: %s\n' % (inc, ', '.join('`%s`' % x for x in c)))
    else:
        L.append('无。\n')
    L.append('\n## 3. 平台类中未能在 ares-r-oss 定位的 include\n\n')
    L.append('均为 PLATFORM 类但 ares-r-oss 无对应路径，原因有三：'
             '**非 MT6893 平台 variant 头**（该仓库只带 `mt6885-*` 系列）、'
             '**4.14 老 mt-plat 路径**（`mach/*` 等，平台层已演进）、'
             '**connectivity 私有栈内部头**（空壳仓库不含真驱动）。'
             '都不计入平台基线导出；逐条归因与落点见 `compat/PATH-ALIASES.md` §3。\n\n')
    for inc, n in unresolved.most_common(80):
        L.append('- `%s`（%d 处）\n' % (inc, n))
    L.append('\n## 4. 4.14 主线头（7.3 已删除）—— 属 API 适配，不当基线搬\n\n')
    L.append('这些 `#include <linux/...>` 在 7.3.3 的 `include/` 下不存在。'
             '把 ares-r-oss 的 4.14 原文件放进 `linux/` 会与主线类型定义冲突，'
             '因此**本工具不导出**。落地方式：在 `compat/legacy-headers/` 提供'
             '最小声明桩（仅编译期），或把 `adapted/` 的调用点改写到 7.3 等价 API。\n\n')
    for inc, n in legacy.most_common(60):
        L.append('- `%s`（%d 处）\n' % (inc, n))
    L.append('\n## 5. 私有栈内部头（非平台基线）\n\n')
    L.append('这些属于 connectivity 私有栈内部、条件编译分支或非内核编译单元；'
             'ares-r-oss 的 `connectivity/` 只有 6 个空壳 adapter，不含真驱动，'
             'seriaTvT 裁剪树也未导出，故无法从该仓库补齐。'
             '由目标树部署时按实际启用的 chip/platform 分支确认。\n\n')
    for inc, n in internal.most_common(60):
        L.append('- `%s`（%d 处）\n' % (inc, n))
    L.append('\n## 6. 完整性校验\n\n')
    L.append('`tools/baseline-manifest.tsv` 逐文件记录 ares-r-oss 的 blob sha；'
             '下载时已用 git object hash 与 `git ls-tree` 比对，'
             '可用 `git cat-file -p <sha>` 在克隆中复核字节一致。\n')
    with open(args.report, 'w') as f:
        f.writelines(L)
    print('报告 -> %s' % args.report)
    print('未定位: %d, 歧义: %d' % (len(unresolved), len(ambig)))


if __name__ == '__main__':
    main()
