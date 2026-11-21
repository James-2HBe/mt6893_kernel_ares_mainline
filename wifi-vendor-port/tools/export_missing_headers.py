#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
export_missing_headers.py — 从本地 lineage 仓库 (aeb1327b) 用 git show 保真取出
路径 B 21 个驱动缺的 MTK 平台头，落到 baseline/platform/<相对路径>。
相对路径沿用 lineage 原路径，使 integration 的 -I 祖先扫描能命中
  #include "xxx.h" / "include/xxx.h" 两类写法。

用法：python3 tools/export_missing_headers.py --repo /tmp/mt6893-lin232
"""
import argparse
import os
import subprocess
import sys

# (lineage 内相对路径, 本包落点相对 baseline/platform/ 的路径)
# 落点保持 lineage 原路径；pmic_api_buck.h 的 .c 用 "include/pmic_api_buck.h" 写法，
# 落点需放在 .../pmic/include/ 下，由 -I .../pmic 祖先命中。
HEADERS = [
    ("drivers/devfreq/helio-dvfsrc.h",
     "drivers/devfreq/helio-dvfsrc.h"),
    ("drivers/misc/mediatek/base/power/include/mtk_clkbuf_ctl.h",
     "drivers/misc/mediatek/base/power/include/mtk_clkbuf_ctl.h"),
    ("drivers/misc/mediatek/base/power/include/mtk_ppm_api.h",
     "drivers/misc/mediatek/base/power/include/mtk_ppm_api.h"),
    ("drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.h",
     "drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.h"),
    ("drivers/misc/mediatek/performance/include/topo_ctrl.h",
     "drivers/misc/mediatek/performance/include/topo_ctrl.h"),
    ("drivers/misc/mediatek/pmic/include/pmic_lbat_service.h",
     "drivers/misc/mediatek/pmic/include/pmic_lbat_service.h"),
    ("drivers/misc/mediatek/pmic/include/pmic_api_buck.h",
     "drivers/misc/mediatek/pmic/include/pmic_api_buck.h"),
    ("drivers/power/supply/mediatek/battery/mtk_gauge_class.h",
     "drivers/power/supply/mediatek/battery/mtk_gauge_class.h"),
    # --- 级联：被上面头引用、或原厂同目录变体 ---
    ("drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h",
     "drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h"),
    ("drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h",
     "drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h"),
    ("drivers/devfreq/helio-dvfsrc_v1.h",
     "drivers/devfreq/helio-dvfsrc_v1.h"),
    # freqhopping 顶层 mtk_freqhopping.h 在源树不存在（构建期按变体拷贝），
    # 用 mt6873 变体落到 freqhopping/ 顶层，凑齐 #include "mtk_freqhopping.h"
    ("drivers/misc/mediatek/freqhopping/mt6873/mtk_freqhopping.h",
     "drivers/misc/mediatek/freqhopping/mtk_freqhopping.h"),
    # battery_ares 变体
    ("drivers/power/supply/mediatek/battery_ares/mtk_battery_internal.h",
     "drivers/power/supply/mediatek/battery/mtk_battery_internal.h"),
]


def git_show(repo, commit, path):
    return subprocess.run(
        ['git', '-C', repo, 'show', f'{commit}:{path}'],
        capture_output=True, text=True, timeout=60)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/tmp/mt6893-lin232')
    ap.add_argument('--commit', default='aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f')
    ap.add_argument('--dst', default='baseline/platform')
    args = ap.parse_args()

    dst = os.path.abspath(args.dst)
    ok, fail = 0, 0
    for src, rel in HEADERS:
        r = git_show(args.repo, args.commit, src)
        if r.returncode != 0 or not r.stdout:
            print(f'FAIL {src}: {r.stderr.strip()[:120]}')
            fail += 1
            continue
        out = os.path.join(dst, rel)
        os.makedirs(os.path.dirname(out), exist_ok=True)
        open(out, 'w').write(r.stdout)
        sha = subprocess.run(['git', '-C', args.repo, 'rev-parse', f'{args.commit}:{src}'],
                             capture_output=True, text=True).stdout.strip()
        print(f'OK   {rel}  (blob {sha[:12]})')
        ok += 1
    print(f'\n导出 {ok} 个成功 / {fail} 个失败 -> {dst}')


if __name__ == '__main__':
    main()
