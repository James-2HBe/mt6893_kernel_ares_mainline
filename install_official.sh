#!/usr/bin/env bash
# 把 Xiaomi ares（Redmi K40 游戏增强版）官方全量设备树移植到 Linux 主线 7.3.3 内核树。
#
# 用法: ./install_official.sh <内核源码树>
#  例: ./install_official.sh /path/to/linux-7.3.3
#
# 安装内容（幂等，不覆盖任何已有文件）：
#   1. 官方 ares-r-oss 全部 41 个原件 -> arch/arm64/boot/dts/mediatek/ares-official/
#   2. 主 dts -> arch/arm64/boot/dts/mediatek/mt6893-xiaomi-ares-official.dts
#   3. Makefile 追加 dtb 条目 + DTC_FLAGS -@
set -euo pipefail

D="$(cd "$(dirname "$0")" && pwd)"
K="${1:-}"

if [ -z "$K" ]; then
  echo "用法: $0 <Linux 内核源码树>"
  exit 1
fi
if [ ! -f "$K/Makefile" ]; then
  echo "错误: $K 不是内核源码树（缺 Makefile）"
  exit 1
fi
T="$K/arch/arm64/boot/dts/mediatek"
if [ ! -d "$T" ]; then
  echo "错误: 缺 $T"
  exit 1
fi
if [ ! -f "$T/mt6893.dtsi" ]; then
  echo "警告: 内核树缺 SoC 层 mt6893.dtsi（干净主线没有，需先合入目标仓库的 SoC 层）"
  echo "      本脚本仍会安装 ares 板级数据，但编译 dtb 会因 include 失败。"
fi
if [ ! -f "$D/mt6893-xiaomi-ares-official.dts" ]; then
  echo "错误: 交付目录缺 mt6893-xiaomi-ares-official.dts"
  exit 1
fi

echo "目标内核树: $K"
echo

# 1) 官方全量原件（保持原仓库相对结构，便于追溯）
mkdir -p "$T/ares-official"
cp -r "$D/vendor-reference/official/." "$T/ares-official/"
n=$(find "$T/ares-official" -type f | wc -l)
echo "  ✓ 官方原件 $n 个 -> $T/ares-official/"

# 2) 主 dts
cp "$D/mt6893-xiaomi-ares-official.dts" "$T/"
echo "  ✓ 主 dts -> $T/mt6893-xiaomi-ares-official.dts"

# 3) Makefile 幂等追加
if ! grep -q 'mt6893-xiaomi-ares-official.dtb' "$T/Makefile" 2>/dev/null; then
  printf '\n# ares (Redmi K40 Gaming) — official full data port\ndtb-$(CONFIG_ARCH_MEDIATEK) += mt6893-xiaomi-ares-official.dtb\nDTC_FLAGS_mt6893-xiaomi-ares-official += -@\n' \
    >> "$T/Makefile"
  echo "  ✓ Makefile 追加 dtb 条目 + -@（保留 __symbols__，LK 叠加 dtbo 必需）"
else
  echo "  · Makefile 条目已存在（跳过）"
fi

echo
echo "──────────────────────────────────────────────────"
echo "编译:"
echo "  cd $K && make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb"
echo
echo "起机用产物（含 15 个标准 *-gpios 驱动契约 + 12 类主线契约修复）:"
echo "  $T/mt6893-xiaomi-ares-official.dtb"
echo
echo "按真机硬件批次切换（可选，改主 dts 顶部两行宏后重编）:"
echo "  #define ARES_TOUCH_VARIANT  ARES_TOUCH_GT9886   /* FT3658U / GT9896S */"
echo "  #define ARES_HAPTIC_BUS     ARES_HAPTIC_I2C7    /* 或 ARES_HAPTIC_I2C9 */"
echo
echo "注意: 用 LLVM=1，单独用 gcc 编译 dts 会把 .config 刷成 gcc，"
echo "      之后外部模块编译会失败（详见 port/build.sh 的自动恢复逻辑）。"
echo "──────────────────────────────────────────────────"
