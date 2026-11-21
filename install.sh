#!/usr/bin/env bash
# 把 ares「官方数据版 v2」适配结果装进 James-2HBe/mt6893_kernel_ares_mainline
# 的 7.3.3-mt6893-xiaomi-chopin 分支工作树。
#
# 用法: ./install.sh <仓库本地路径>
#
# 设计原则：仓库中已有且真实可用的文件（SoC 层 mt6893.dtsi、时钟/pinctrl/PMIC
# 驱动、chopin 板级 dts）一律不覆盖。只新增 ares 板级 dts 与配置片段。
set -euo pipefail
REPO="${1:?用法: ./install.sh <仓库本地路径>}"
D="$(cd "$(dirname "$0")" && pwd)"
T="$REPO/arch/arm64/boot/dts/mediatek"

[ -d "$T" ] || { echo "错误: $T 不存在，确认仓库路径与分支"; exit 1; }
[ -f "$T/mt6893.dtsi" ] || { echo "错误: 缺少 $T/mt6893.dtsi（SoC 层），请切到 7.3.3-mt6893-xiaomi-chopin 分支"; exit 1; }

# 1) ares 板级 dts（官方数据版，单文件自包含，不覆盖任何已有文件）
cp "$D/mt6893-xiaomi-ares.dts" "$T/"
echo "✓ 安装 mt6893-xiaomi-ares.dts"

# 2) pinfunc 头：仓库若没有就补上（主线 7.3.3 自带该文件）
if [ ! -f "$T/mt6893-pinfunc.h" ]; then
  cp "$D/mt6893-pinfunc.h" "$T/"
  echo "✓ 补齐 mt6893-pinfunc.h"
fi

# 3) 接入 dts Makefile（含 -@，LK 叠加 dtbo 依赖 __symbols__）
if ! grep -q "mt6893-xiaomi-ares.dtb" "$T/Makefile"; then
  if ! git -C "$REPO" apply "$D/Makefile.patch" 2>/dev/null; then
    printf '\ndtb-$(CONFIG_ARCH_MEDIATEK) += mt6893-xiaomi-ares.dtb\nDTC_FLAGS_mt6893-xiaomi-ares += -@\n' >> "$T/Makefile"
  fi
  echo "✓ 接入 mediatek/Makefile"
fi

# 4) defconfig 片段与驱动源码、文档：复制到工程可见目录（不自动改 defconfig，
#    避免破坏仓库默认配置；由使用者按需手动合并）
echo
echo "────────────────────────────────────────────────────────"
echo "以下文件不自动写入仓库，请按需手动合并："
echo "  · ares_defconfig.fragment     → 追加到你的 defconfig（A 类主线驱动开关）"
echo "  · drivers/src/                → 官方 4.14 私有驱动源码（B 类，需重写后使用）"
echo "  · docs/DRIVER-PORTING.md      → 驱动甄别与移植要点"
echo "  · vendor-reference/official/  → 官方 ares 内核设备树原件（数据源存档）"
echo "────────────────────────────────────────────────────────"
echo
echo "编译验证："
echo "  cd $REPO"
echo "  make ARCH=arm64 defconfig"
echo "  make ARCH=arm64 mediatek/mt6893-xiaomi-ares.dtb"
