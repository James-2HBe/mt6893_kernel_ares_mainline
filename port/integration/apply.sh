#!/usr/bin/env bash
# 把 port/*.c 接入目标内核树：复制源码 + 追加 Makefile 规则 + Kconfig/defconfig。
# 幂等：重复执行不会重复追加。
#
# 用法: ./apply.sh <内核源码目录> [--dry-run]
#   ./apply.sh /path/to/mt6893_kernel_ares_mainline --dry-run
#   ./apply.sh /path/to/mt6893_kernel_ares_mainline
set -euo pipefail

K="${1:?用法: ./apply.sh <内核源码目录> [--dry-run]}"
P="$(cd "$(dirname "$0")/../.." && pwd)"
DRY=0
[ "${2:-}" = "--dry-run" ] && DRY=1

[ -d "$K/drivers" ] || { echo "错误: $K 不是内核源码树"; exit 1; }

run() {
	if [ "$DRY" -eq 1 ]; then
		echo "  [dry-run] $*"
	else
		eval "$@"
	fi
}

MAP=$(cat <<'EOF'
regulator-wl2866d.c|drivers/regulator|CONFIG_REGULATOR_WL2866D
leds-aw2028.c|drivers/leds|CONFIG_LEDS_AW2028
haptic-aw8697.c|drivers/input/misc|CONFIG_HAPTIC_AW8697
input-gamekey.c|drivers/input/keyboard|CONFIG_KEYBOARD_ARES_GAMEKEY
touchscreen-gt9886.c|drivers/input/touchscreen|CONFIG_TOUCHSCREEN_GT9886
xmusb350-charger.c|drivers/power/supply|CONFIG_CHARGER_XMUSB350
et7480-audioswitch.c|drivers/misc|CONFIG_ET7480_AUDIOSWITCH
st21nfc.c|drivers/misc|CONFIG_NFC_ST21NFC_ARES
gf3658-spi.c|drivers/misc|CONFIG_SPI_GF3658
EOF
)

echo "内核树: $K"
echo "源码  : $P/port"
echo

while IFS='|' read -r src dir sym; do
	[ -z "$src" ] && continue
	obj="${src%.c}.o"
	rule="obj-\$($sym) += $obj"

	run "cp '$P/port/$src' '$K/$dir/$src'"
	echo "  ✓ $src -> $dir/$src"

	if ! grep -qF "$rule" "$K/$dir/Makefile" 2>/dev/null; then
		if [ "$DRY" -eq 1 ]; then
			echo "  [dry-run] echo '$rule' >> $dir/Makefile"
		else
			printf '\n%s\n' "$rule" >> "$K/$dir/Makefile"
		fi
		echo "  ✓ Makefile 规则: $rule"
	else
		echo "  · Makefile 规则已存在"
	fi
done <<< "$MAP"

# Kconfig：集中追加到 drivers/misc/Kconfig
if ! grep -q 'menu "Xiaomi ares (MT6893) vendor peripherals"' \
	"$K/drivers/misc/Kconfig" 2>/dev/null; then
	if [ "$DRY" -eq 1 ]; then
		echo "  [dry-run] cat Kconfig.fragment >> drivers/misc/Kconfig"
	else
		printf '\n' >> "$K/drivers/misc/Kconfig"
		cat "$(dirname "$0")/Kconfig.fragment" >> "$K/drivers/misc/Kconfig"
	fi
	echo "  ✓ Kconfig 条目已追加到 drivers/misc/Kconfig"
else
	echo "  · Kconfig 条目已存在"
fi

# 目标树可能已有同名 config（另一处带 select REGMAP_I2C），合并后会产生
# 「A select B + A depends on B」的 Kconfig 递归依赖，这里顺手排除。
FIXK="$P/tools/fix_kconfig_recursion.py"
if [ -f "$FIXK" ]; then
	if [ "$DRY" -eq 1 ]; then
		python3 "$FIXK" "$K" --check >/dev/null 2>&1 \
			|| echo "  ⚠ 检测到 Kconfig 递归依赖，正式安装时会自动修复"
	else
		python3 "$FIXK" "$K" --quiet \
			&& echo "  ✓ Kconfig 递归依赖已排除" \
			|| echo "  ⚠ Kconfig 递归依赖修复失败，请手工检查"
	fi
fi

# 安装主设备树（含 15 个新增标准 *-gpios 与主线契约修复）
T="$K/arch/arm64/boot/dts/mediatek"
if [ -d "$T" ]; then
	run "cp '$P/mt6893-xiaomi-ares-official.dts' '$T/'"
	echo "  ✓ 主 dts -> $T/mt6893-xiaomi-ares-official.dts"
	if ! grep -q 'mt6893-xiaomi-ares-official.dtb' "$T/Makefile" 2>/dev/null; then
		if [ "$DRY" -eq 1 ]; then
			echo "  [dry-run] 追加 dtb 条目 + DTC_FLAGS -@ 到 $T/Makefile"
		else
			printf '\ndtb-$(CONFIG_ARCH_MEDIATEK) += mt6893-xiaomi-ares-official.dtb\nDTC_FLAGS_mt6893-xiaomi-ares-official += -@\n' \
				>> "$T/Makefile"
		fi
		echo "  ✓ Makefile: official dtb + -@（保留 __symbols__）"
	else
		echo "  · official dtb 条目已存在"
	fi
fi

echo
echo "──────────────────────────────────────────────────"
echo "以下需按需手工合并："
echo "  · integration/defconfig.additions  → 追加到 defconfig"
echo "  · compat/*.patch                  → git apply（A 类 compatible 补丁）"
echo "  · 重新生成配置: make LLVM=1 ARCH=arm64 olddefconfig"
echo "  · 编译驱动:    make LLVM=1 ARCH=arm64 modules"
echo "  · 编译 dts:    make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb"
echo "  · 起机用 official dts（含标准 *-gpios 与主线契约修复）"
echo "──────────────────────────────────────────────────"
