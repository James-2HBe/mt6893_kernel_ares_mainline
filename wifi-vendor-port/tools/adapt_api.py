#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
adapt_api.py — 把原厂 4.14/GKI-5.x WiFi 私有栈源码，适配成 Linux 7.3.x 主线可编译形态。

设计（与 compat/mtk_wifi_compat.h 分工）：
  - 纯符号删除（set_fs/timeval/do_gettimeofday/strlcpy/dma_zalloc/VERIFY_*/
    create_proc_entry）：由 compat 头补齐，本脚本只负责让每个 .c 引用到该头。
  - 签名/调用点变化（下列 R1..R7）：由本脚本以幂等正则落到源码，可 diff 审计。
  - 社区已用 KERNEL_VERSION(4,15/5,6) 分流的 timer_setup / proc_ops / timespec64
    分支：7.3 下走新分支，【不改】低版本 #else 死分支里的 init_timer/->function 等。

规则（均幂等，重复运行不叠加）：
  R1  class_create(THIS_MODULE, X)   -> class_create(X)          [7.3 去 owner]
  R2  access_ok(VERIFY_READ|WRITE, p, n) -> access_ok(p, n)      [7.3 去 VERIFY]
  R3  vfs_read/vfs_write(kernbuf...) -> kernel_read/kernel_write  [7.3 无 addr_limit]
  R4  strlcpy -> strscpy（源码侧也改，compat 头仅兜底）
  R5  dma_zalloc_coherent -> dma_alloc_coherent（源码侧也改）
  R6  do_gettimeofday(&tv) 保留（compat 头 inline 提供）；struct timeval 保留
      （compat 头补全为完整定义）。仅在 tv->tv_usec 语义处不动（= nsec/1000 等价）。
  R7  每个 .c 顶部插入 #include "mtk_wifi_compat.h"（用 -I 指向 compat/ 目录）。
      —— 若目标树已用 ccflags -include，可加 --no-inject 跳过。

用法：
  python3 adapt_api.py --src drivers --dst adapted --compat compat
  python3 adapt_api.py --src drivers --dst adapted --report reports/API-ADAPTATION.md
输出：
  adapted/            全量源码（保持原目录层级，仅被改文件有差异）
  reports/...         每个规则的命中文件数/次数、示例、边界说明
"""
import argparse
import os
import re
import shutil
import sys
from collections import OrderedDict

INCLUDE_LINE = '#include "mtk_wifi_compat.h"'

# 非内核编译单元：bootloader / virtual-OS / 测试桩。这些目录下的同名
# vfs_read/vfs_write 是自定义函数（参数顺序都不同），且依赖 uboot 头文件，
# 根本不进 Linux kbuild。整体跳过改写与 compat 注入，保持逐字节原样。
EXCLUDE_PARTS = ('uboot_driver', '/uboot/', 'virtual_os', 'CFG_VIRTUAL_OS')


def excluded(rel):
    r = '/' + rel.replace(os.sep, '/')
    return any(p in r for p in EXCLUDE_PARTS)


# (编号, 说明, 正则, 替换, 备注)
def rules():
    return [
        ("R1", "class_create 去 owner 参数",
         re.compile(r'class_create\s*\(\s*THIS_MODULE\s*,\s*'),
         'class_create(',
         "7.3: struct class *class_create(const char *name)"),
        ("R2", "access_ok 去 VERIFY 首参",
         re.compile(r'access_ok\s*\(\s*VERIFY_(?:READ|WRITE)\s*,\s*'),
         'access_ok(',
         "7.3: access_ok(addr, size); 变量含义在 adapt 前已确认"),
        # 只匹配内核 vfs_read/vfs_write 调用：形如 vfs_read(file, data, size, &offset)
        # 其中第 2 参是缓冲、第 4 参是 &pos。用 (?!\*) 排除自定义函数
        # vfs_read(*image, file, 0, *code_len)（首参解引用，uboot 侧）。
        ("R3a", "vfs_read -> kernel_read",
         re.compile(r'\bvfs_read(\s*\(\s*)(?!\*)'),
         r'kernel_read\1',
         "7.3: 内核缓冲场景，kernel_read(file, buf, count, pos)；排除 *image 首参"),
        ("R3b", "vfs_write -> kernel_write",
         re.compile(r'\bvfs_write(\s*\(\s*)(?!\*)'),
         r'kernel_write\1',
         "7.3: 同上；排除 *image 首参"),
        ("R4", "strlcpy -> strscpy",
         re.compile(r'\bstrlcpy\s*\('),
         'strscpy(',
         "7.3 删除 strlcpy；返回值语义差异见报告"),
        ("R5", "dma_zalloc_coherent -> dma_alloc_coherent",
         re.compile(r'\bdma_zalloc_coherent\s*\('),
         'dma_alloc_coherent(',
         "7.3 删除前者，后者自带零初始化"),
    ]


# 需要原样带进 adapted/ 的构建/元数据文件（无 .c/.h 后缀但接线必需）
BUILD_FILES = ('Kconfig', 'Makefile', 'Kbuild', '*.mk', '*.patch',
               '*.dts', '*.dtsi', '*.txt', '*.md', '*.sh', '*.py',
               '*.bin', '*.dat', '*.cfg', '*.h')


def _is_build_file(fn):
    import fnmatch
    if fn.endswith(('.c', '.h')):
        return True
    return any(fnmatch.fnmatch(fn, pat) for pat in BUILD_FILES)


def iter_sources(src):
    for dp, _, fns in os.walk(src):
        for fn in sorted(fns):
            if _is_build_file(fn):
                yield os.path.join(dp, fn)


def transform(text, is_c, do_inject, stats):
    changed = False
    for rid, desc, rx, rep, note in rules():
        text2, n = rx.subn(rep, text)
        if n:
            stats[rid][0] += 1          # 命中文件数
            stats[rid][1] += n          # 命中次数
            changed = True
            text = text2

    # R7: 给 .c 注入 compat 头（放在首个 #include 之前，或文件最前）
    if is_c and do_inject and INCLUDE_LINE not in text:
        lines = text.splitlines(True)
        # 找到第一个 #include 行；若存在则插在它前面，否则插在开头
        idx = next((i for i, l in enumerate(lines) if l.lstrip().startswith('#include')), None)
        inject = (f'/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */\n'
                  f'{INCLUDE_LINE}\n')
        if idx is None:
            lines.insert(0, inject)
        else:
            lines.insert(idx, inject)
        text = ''.join(lines)
        stats["R7"][0] += 1
        stats["R7"][1] += 1
        changed = True
    return text, changed


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--src', required=True, help='原厂源码目录（drivers/）')
    ap.add_argument('--dst', required=True, help='输出目录（adapted/）')
    ap.add_argument('--report', default=None, help='Markdown 报告路径')
    ap.add_argument('--no-inject', action='store_true',
                    help='不注入 #include（目标树用 ccflags -include 时）')
    ap.add_argument('--clean', action='store_true', help='先清空 dst')
    args = ap.parse_args()

    src = os.path.abspath(args.src)
    dst = os.path.abspath(args.dst)
    if not os.path.isdir(src):
        print(f'源目录不存在: {src}', file=sys.stderr); sys.exit(2)
    if args.clean and os.path.isdir(dst):
        shutil.rmtree(dst)
    os.makedirs(dst, exist_ok=True)

    stats = OrderedDict()
    for rid, desc, rx, rep, note in rules():
        stats[rid] = [0, 0, desc, note]
    stats["R7"] = [0, 0, "注入 compat 头引用", "用 -I compat/ 或 ccflags -include"]

    total = modified = skipped = 0
    examples = {}
    for full in iter_sources(src):
        total += 1
        rel = os.path.relpath(full, src)
        out = os.path.join(dst, rel)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        # 非内核编译单元、以及非 .c/.h 的构建/元数据文件：原样复制
        if excluded(rel) or not full.endswith(('.c', '.h')):
            shutil.copy2(full, out)
            skipped += 1
            continue
        raw = open(full, 'rb').read()
        try:
            text = raw.decode('utf-8')
        except UnicodeDecodeError:
            text = raw.decode('latin-1')
        before = text
        new, changed = transform(text, full.endswith('.c'), not args.no_inject, stats)
        if changed:
            modified += 1
            # 记录首个改动示例（截断）
            if len(examples) < 12:
                import difflib
                d = list(difflib.unified_diff(
                    before.splitlines(), new.splitlines(), lineterm='', n=0))
                for line in d:
                    if line.startswith('+') and not line.startswith('+++'):
                        examples.setdefault(rel, []).append(line[:100])
        # 用原始字节写出未改文件（保持保真），改动文件用文本写出
        if not changed:
            shutil.copy2(full, out)
        else:
            with open(out, 'w', encoding='utf-8', errors='replace') as f:
                f.write(new)

    print(f'扫描 {total} 个源文件，改动 {modified} 个，跳过非内核 {skipped} 个 -> {dst}')
    for rid, (fc, cnt, desc, note) in stats.items():
        print(f'  {rid:4s} {desc:42s} 文件 {fc:4d}  次数 {cnt:4d}')

    if args.report:
        os.makedirs(os.path.dirname(os.path.abspath(args.report)), exist_ok=True)
        L = []
        L.append('# API 适配改写报告（原厂 4.14/GKI-5.x → Linux 7.3.3）\n')
        L.append('由 `tools/adapt_api.py` 生成，幂等可复跑。源文件 `drivers/` 保持保真原件，'
                 '适配结果在 `adapted/`（同目录层级）。\n')
        L.append(f'- 扫描源文件：**{total}**\n- 改动文件：**{modified}**\n'
                 f'- 原样跳过（uboot/virtual-OS 非内核编译单元）：**{skipped}**\n')
        L.append('\n## 改写规则与命中\n')
        L.append('| 规则 | 说明 | 命中文件 | 命中次数 | 依据 |\n|---|---|---|---|---|\n')
        for rid, (fc, cnt, desc, note) in stats.items():
            L.append(f'| {rid} | {desc} | {fc} | {cnt} | {note} |\n')
        L.append('\n## 由 compat 头补齐（不在本脚本改动）\n\n')
        L.append('`set_fs/get_fs/get_ds/mm_segment_t/KERNEL_DS/USER_DS`、`struct timeval`、'
                 '`do_gettimeofday()`、`VERIFY_READ/VERIFY_WRITE`、`strlcpy` 兜底、'
                 '`dma_zalloc_coherent` 兜底、`create_proc_entry` 编译桩。'
                 '详见 `compat/mtk_wifi_compat.h`，已用 7.3.3 头文件 `-fsyntax-only` 验证。\n')
        L.append('\n## 社区已分流、本轮不改（7.3 走新分支）\n\n')
        L.append('`init_timer`→`timer_setup` + 回调 `from_timer`（12 处，11 处在 '
                 '`KERNEL_VERSION(4,15,0)` 的 `#else` 死分支）、'
                 '`file_operations`→`proc_ops`（85 处 fops，5.6 版本宏分流）、'
                 '`timespec64`（BT/wlan 部分已适配）。这些在 7.3 的 `LINUX_VERSION_CODE` '
                 '下编译的是新分支，盲改反而会破坏版本兼容。\n')
        if examples:
            L.append('\n## 改动示例（截取新增行）\n')
            for rel, lines in list(examples.items())[:10]:
                L.append(f'\n**`{rel}`**\n\n```c\n')
                L += [x + '\n' for x in lines[:3]]
                L.append('```\n')
        open(args.report, 'w').write(''.join(L))
        print(f'报告已写: {args.report}')


if __name__ == '__main__':
    main()
