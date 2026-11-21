#!/usr/bin/env bash
# 一键复现「官方全量数据版 ares 板级 dts」：
#   v3 组装器（官方真值、0 桩）
#     → GPIO 契约标准化（15 个 *-gpios）
#     → 主线契约修复（status / 2-cell IRQ / 批次互斥 / binding 合规）
#     → 产出主 dts
#
# 用法: ./tools/build_official_dts.sh
set -euo pipefail

D="$(cd "$(dirname "$0")/.." && pwd)"
cd "$D"

GEN="$(mktemp /tmp/ares_official_XXXXXX.dts)"
GEN2="$(mktemp /tmp/ares_normalized_XXXXXX.dts)"
trap 'rm -f "$GEN" "$GEN2"' EXIT

echo "=== 1) 组装：官方全量真值（消除 6 个最小桩）==="
python3 tools/assemble_ares_dts_v3.py vendor-reference "$GEN"

echo
echo "=== 2) 标准化：补 15 个主线驱动所需的 *-gpios ==="
python3 port/normalize_dt_gpios.py "$GEN" "$GEN2" \
  port/dt-bindings/gpio-normalize-official.patch

echo
echo "=== 3) 主线契约修复（见 docs/ROUND3-FIXES.md）==="
python3 tools/fix_mainline_contract.py \
  "$GEN2" \
  mt6893-xiaomi-ares-official.dts \
  port/dt-bindings/mainline-contract.patch

echo
echo "=== 4) 结果 ==="
printf "  %-42s %s 行\n" "mt6893-xiaomi-ares-official.dts" \
  "$(wc -l < mt6893-xiaomi-ares-official.dts)"
printf "  %-42s %s 个\n" "gpio-normalize-official.patch 新增属性" \
  "$(grep -cE '^\+\s*(reset|irq|key|hall|sda|scl|therm)-gpios' \
      port/dt-bindings/gpio-normalize-official.patch)"
echo
echo "编译验证: ./install_official.sh <内核树> 后"
echo "          cd <内核树> && make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb"
