#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
force_defconfig.py — 把 ares 所需的 CONFIG 项「强制」写入内核树的
arch/arm64/configs/defconfig。

与「追加片段」的区别（本脚本的"强制"语义）：
  1. 先删除 defconfig 里**任意位置**已存在的同名赋值
     （包括 `CONFIG_X=...` 与 `# CONFIG_X is not set`）；
  2. 再删除历史 ares 标记块（auto_install.sh 追加块 / 本脚本旧块）；
  3. 最后在文件末尾写入唯一一份标记块，值以本包为准。

结果是每个符号在 defconfig 里**恰好出现一次、值唯一**，因此：
  - 不会触发 `override: reassigning to symbol X` 警告；
  - 不会被目标树原有的 `=m` / 未设置状态悄悄降级；
  - 每次 `make ... defconfig` 都会稳定带上，不依赖 .config 是否残留。

配置来源（后者覆盖前者的同名项）：
  ares_defconfig.fragment                    A 类 75 项（其中 29 项 =m）
  port/integration/defconfig.additions      B 类 9 项
  ares_build_config.additions               KVM / NTSYNC 等额外强制项
  ares_builtin_config.additions             仅 --builtin：6 个父级框架 =y
  --extra NAME=VALUE                        命令行临时追加（可多次）

两种构建形态：
  默认（模块版）：9 个 B 类驱动为 =m，另编出 .ko 供 insmod。
  --builtin（内建版）：把清单里所有 =m 提升为 =y，驱动直接编进 Image，
      不产生 .ko。用于「一份驱动全部进内核、无需加载模块」的内核。
      注意这只覆盖本包强制清单里的 38 个 =m 项；主线 defconfig 自带的大量 =m
      不受影响，CONFIG_MODULES 保持 =y（见 README「内建版内核」章节）。

用法:
    python3 force_defconfig.py <内核树>                 # 强制写入（自动备份）
    python3 force_defconfig.py <内核树> --builtin       # 内建版：=m → =y
    python3 force_defconfig.py <内核树> --dry-run      # 只报告
    python3 force_defconfig.py <内核树> --check        # 校验；不符返回 1
    python3 force_defconfig.py <内核树> --list         # 打印强制项清单
    python3 force_defconfig.py <内核树> --quiet
"""
import argparse
import datetime
import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
PKG = os.path.dirname(HERE)

FRAGMENTS = [
    os.path.join(PKG, 'ares_defconfig.fragment'),
    os.path.join(PKG, 'port', 'integration', 'defconfig.additions'),
    os.path.join(PKG, 'ares_build_config.additions'),
]
# 仅 --builtin 加载：提升 ASoC / 无线 / WWAN / USB 串口父级框架，解除 tristate 上限
BUILTIN_FRAGMENT = os.path.join(PKG, 'ares_builtin_config.additions')

MARK_BEGIN = '# >>> ares build (force_defconfig.py) >>>'
MARK_END = '# <<< ares build (force_defconfig.py) <<<'
OLD_MARKS = (
    '# >>> ares port (auto_install.sh) >>>',
    '# <<< ares port (auto_install.sh) <<<',
)

RE_ASSIGN = re.compile(r'^CONFIG_([A-Za-z0-9_]+)=(.*)$')
RE_UNSET = re.compile(r'^# CONFIG_([A-Za-z0-9_]+) is not set\s*$')
RE_CFG_LINE = re.compile(r'^(CONFIG_[A-Za-z0-9_]+)=([^\s#]*)')


def load_forced(extras, builtin=False):
    """按优先级收集 {符号: 值}；builtin 模式下把所有 =m 提升为 =y。"""
    forced = {}
    paths = list(FRAGMENTS)
    if builtin:
        paths.append(BUILTIN_FRAGMENT)
    for path in paths:
        if not os.path.isfile(path):
            print('警告: 缺配置片段 %s' % path, file=sys.stderr)
            continue
        with open(path, 'r', encoding='utf-8', errors='replace') as f:
            for line in f:
                m = RE_CFG_LINE.match(line.strip())
                if m:
                    forced[m.group(1)] = m.group(2)
    for item in extras:
        if '=' not in item:
            print('错误: --extra 需为 NAME=VALUE 形式: %s' % item, file=sys.stderr)
            sys.exit(2)
        name, val = item.split('=', 1)
        forced[name.strip()] = val.strip()
    if builtin:
        for name, val in forced.items():
            if val == 'm':
                forced[name] = 'y'
    return forced


def strip_existing(lines, forced):
    """删除同名赋值行与历史 ares 标记块，返回剩余行。"""
    out, removed = [], []
    in_old_block = False
    for line in lines:
        s = line.rstrip('\r\n')
        if s.strip() == OLD_MARKS[0] or s.strip() == MARK_BEGIN:
            in_old_block = True
            continue
        if s.strip() == OLD_MARKS[1] or s.strip() == MARK_END:
            in_old_block = False
            continue
        if in_old_block:
            continue
        m = RE_ASSIGN.match(s) or RE_UNSET.match(s)
        if m and ('CONFIG_' + m.group(1)) in forced:
            removed.append(s)
            continue
        out.append(line)
    return out, removed


def build_block(forced):
    block = ['\n', MARK_BEGIN + '\n']
    for name in sorted(forced):
        block.append('%s=%s\n' % (name, forced[name]))
    block.append(MARK_END + '\n')
    return block


def main():
    ap = argparse.ArgumentParser(description='强制写入 ares defconfig 配置项')
    ap.add_argument('kernel_tree', help='内核源码树根目录')
    ap.add_argument('--dry-run', action='store_true', help='只报告，不改动文件')
    ap.add_argument('--check', action='store_true', help='校验；有不符返回 1')
    ap.add_argument('--list', action='store_true', help='打印强制项清单后退出')
    ap.add_argument('--quiet', action='store_true', help='减少输出')
    ap.add_argument('--builtin', action='store_true',
                    help='内建版：把强制清单里所有 =m 提升为 =y（不编模块）')
    ap.add_argument('--extra', action='append', default=[],
                    metavar='NAME=VALUE', help='临时追加/覆盖一项，可多次')
    args = ap.parse_args()

    forced = load_forced(args.extra, args.builtin)
    if args.list:
        for name in sorted(forced):
            print('%s=%s' % (name, forced[name]))
        print('共 %d 项' % len(forced))
        return 0

    root = os.path.abspath(args.kernel_tree)
    dc = os.path.join(root, 'arch', 'arm64', 'configs', 'defconfig')
    if not os.path.isfile(dc):
        print('错误: 找不到 %s' % dc, file=sys.stderr)
        return 2

    with open(dc, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()

    # ── --check：逐项核对「恰好一次且值正确」 ──
    if args.check:
        seen, bad = {}, []
        for line in lines:
            s = line.strip()
            m = RE_ASSIGN.match(s)
            if m:
                name = 'CONFIG_' + m.group(1)
                seen[name] = seen.get(name, []) + [m.group(2)]
                continue
            m = RE_UNSET.match(s)
            if m:
                name = 'CONFIG_' + m.group(1)
                seen[name] = seen.get(name, []) + ['<unset>']
        for name, want in forced.items():
            got = seen.get(name, [])
            if got != [want]:
                bad.append((name, want, got))
        if bad:
            if not args.quiet:
                print('defconfig 强制项校验：%d/%d 不符' % (len(bad), len(forced)))
                for name, want, got in bad:
                    print('  %s 期望 %s 实际 %s' % (name, want, got or '<缺失>'))
            return 1
        if not args.quiet:
            print('defconfig 强制项校验：%d 项全部唯一且值正确 ✔' % len(forced))
        return 0

    # ── 写入 ──
    kept, removed = strip_existing(lines, forced)
    new_lines = kept + build_block(forced)

    if not args.quiet:
        print('强制项 : %d 条（A 类 + B 类 + KVM/NTSYNC）' % len(forced))
        print('清理   : 删除 %d 行同名旧赋值 / 历史 ares 块' % len(removed))
        if removed:
            for s in removed[:8]:
                print('         - %s' % s.strip())
            if len(removed) > 8:
                print('         ... 其余 %d 行' % (len(removed) - 8))

    if args.dry_run:
        print('[dry-run] 未改动文件；实际运行会写入 %d 条唯一赋值。' % len(forced))
        return 0

    stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    rel = os.path.relpath(dc, root)
    backup = os.path.join(root, '.ares-port-backup',
                          'force-defconfig-' + stamp, rel)
    os.makedirs(os.path.dirname(backup), exist_ok=True)
    shutil.copy2(dc, backup)
    with open(dc, 'w', encoding='utf-8') as f:
        f.writelines(new_lines)

    if not args.quiet:
        print('已写入 : %s' % dc)
        print('备份   : %s' % os.path.relpath(backup, root))
        print('下一步 : make LLVM=1 ARCH=arm64 defconfig')
    return 0


if __name__ == '__main__':
    sys.exit(main())
