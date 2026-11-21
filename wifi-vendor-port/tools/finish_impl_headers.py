#!/usr/bin/env python3.11
# -*- coding: utf-8 -*-
"""
finish_impl_headers.py — 收尾：定点补齐 BFS 因「多 variant 歧义」未落地的头

complete_impl_headers.py 的 BFS 对同名多 variant 头会按 rank 选一个，但
`mach/mtk_cpufreq_api.h` 有 mt6853/mt6873/mt6877/mt6885 四份，且 include
写作 `mach/...`（需要 -I 指到 variant 的 include/ 层），静态判定容易漏；
`linux/usb/class-dual-role.h` 是 MTK 私有头（7.3 主线没有），同样需要显式
落到 baseline/platform/include/ 下。

本工具按固定路径从 lineage-23.2 保真下载这两个头，写盘并追加 manifest。

`asm/dma-iommu.h` 故意不补：源树里只有 arch/arm/include/asm/dma-iommu.h，
是 4.14 arm32 时代头，aarch64 目标下属于条件编译死分支，放进来反而污染。
"""
import argparse
import os
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import port_platform_impl as P  # noqa: E402

# (源树路径, 说明)
FORCE = [
    ('drivers/misc/mediatek/include/mt-plat/mt6873/'
     'include/mach/mtk_cpufreq_api.h',
     'MT6893 用 mt6873 平台分支；按原路径导出后 -I 命中 mach/...'),
    ('include/linux/usb/class-dual-role.h',
     'MTK 私有 USB dual-role 头，7.3 主线不含'),
]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/mt6893-lin232')
    ap.add_argument('--out', default='baseline')
    ap.add_argument('--manifest', default='tools/platform-impl-manifest.tsv')
    ap.add_argument('--report', default='reports/IMPL-HEADERS.md')
    args = ap.parse_args()

    commit = subprocess.run(['git', 'rev-parse', 'HEAD'], cwd=args.repo,
                            capture_output=True, text=True, check=True).stdout.strip()
    idx = P.build_blob_index(args.repo)
    got = P.make_fetcher(commit, idx, workers=4)([p for p, _ in FORCE])

    rows = []
    ok = 0
    for path, note in FORCE:
        if path not in got:
            print('FAIL 下载: %s' % path)
            continue
        sha, data = got[path]
        rel = os.path.join(args.out, 'platform', path)
        P.write_file(os.path.join(args.out, 'platform'), path, data)
        rows.append((os.path.join('platform', path), sha, str(len(data)),
                     'forced-header'))
        print('OK  %s (%d 字节, sha %s…)' % (path, len(data), sha[:12]))
        ok += 1

    # 追加 manifest（按 path 去重）
    existing = set()
    if os.path.exists(args.manifest):
        for line in open(args.manifest):
            existing.add(line.split('\t')[0])
    with open(args.manifest, 'a') as f:
        for row in rows:
            if row[0] not in existing:
                f.write('\t'.join(row) + '\n')
                existing.add(row[0])

    # 在报告末尾补一节
    tail = ['\n## 6. 定点补齐（finish_impl_headers.py）\n\n',
            'BFS 因多 variant 歧义未落地的头，按固定路径保真补齐：\n\n',
            '| 源树路径 | 说明 | 状态 |\n|---|---|---|\n']
    for path, note in FORCE:
        tail.append('| `%s` | %s | %s |\n'
                    % (path, note, '已导出' if os.path.isfile(
                        os.path.join(args.out, 'platform', path)) else '失败'))
    tail.append('\n`asm/dma-iommu.h` 故意不补：源树仅有 '
                '`arch/arm/include/asm/dma-iommu.h`（4.14 arm32），'
                'aarch64 目标下属条件编译死分支，导入反而污染。\n')
    if os.path.exists(args.report):
        with open(args.report, 'a', encoding='utf-8') as f:
            f.write(''.join(tail))

    print('定点补齐: %d/%d' % (ok, len(FORCE)))


if __name__ == '__main__':
    main()
