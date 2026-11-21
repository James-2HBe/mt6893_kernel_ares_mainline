#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# ares (Redmi K40 Gaming / MT6893) → Linux 7.3.3 一键自动安装
#
# 把交付包里「所有需要生效的文件」一次性装进内核源码树：
#   1. 主 dts + 起机 dtb + pinfunc 头 + dts Makefile 条目(-@)
#   2. 9 个移植驱动源码 + 9 条 Makefile 规则 + Kconfig 菜单
#   3. 2 个 A 类 compatible 补丁（bq25980 / bq27xxx）
#   4. 2 份 defconfig 片段（A 类 29 个 + B 类 9 个开关）
#   5. 安装后逐项自检
#
# 用法:
#   ./auto_install.sh                          # 自动探测内核树
#   ./auto_install.sh /path/to/linux-7.3.3     # 指定内核树
#   ./auto_install.sh <内核树> --dry-run       # 只预演，不改动任何文件
#   ./auto_install.sh <内核树> --build         # 安装后编译 dtb + 9 个模块
#   ./auto_install.sh <内核树> --with-artifacts# 另把 dtb/.ko/docs 放进内核树 ares-port/
#   ./auto_install.sh <内核树> --rollback      # 撤销全部安装，回到首次安装前
#
# 选项可组合；--dry-run 优先级最高。重复执行安全（幂等）。
# 每次真实安装都会在 <内核树>/.ares-port-backup/<时间戳>/ 留一份快照，
# 回滚基准固定为首次安装前的那份（幂等复跑不会覆盖它）。
# ═══════════════════════════════════════════════════════════════════════
set -euo pipefail

D="$(cd "$(dirname "$0")" && pwd)"
K=""
DRY=0
BUILD=0
ARTIFACTS=0
ROLLBACK=0

usage() { awk 'NR==1{next} /^#/{sub(/^# ?/,""); print; next} {exit}' "$0"; }

for a in "$@"; do
	case "$a" in
		--dry-run) DRY=1 ;;
		--build) BUILD=1 ;;
		--with-artifacts) ARTIFACTS=1 ;;
		--rollback) ROLLBACK=1 ;;
		-h|--help) usage; exit 0 ;;
		-*) echo "未知选项: $a"; usage; exit 2 ;;
		*) [ -z "$K" ] && K="$a" ;;
	esac
done

# ── 目标内核树探测 ────────────────────────────────────────────────────
if [ -z "$K" ]; then
	for c in "$PWD" /workspace/linux-7.3.3; do
		if [ -f "$c/Makefile" ] && [ -d "$c/arch/arm64/boot/dts/mediatek" ]; then
			K="$c"; break
		fi
	done
fi
if [ -z "$K" ]; then
	echo "错误: 未找到内核源码树，请显式指定：./auto_install.sh <内核树>"
	exit 2
fi
if [ ! -f "$K/Makefile" ] || [ ! -d "$K/arch/arm64/boot/dts/mediatek" ]; then
	echo "错误: $K 不是有效的 arm64 内核源码树"
	exit 2
fi
K="$(cd "$K" && pwd)"
T="$K/arch/arm64/boot/dts/mediatek"
BACKUP_ROOT="$K/.ares-port-backup"

step() { echo; echo "── $* ──"; }
ok()   { echo "  ✓ $*"; }
skip() { echo "  · $*（已存在，跳过）"; }
warn() { echo "  ⚠ $*"; }
die()  { echo "  ✗ $*" >&2; exit 1; }

# ── 回滚 ──────────────────────────────────────────────────────────────
if [ "$ROLLBACK" -eq 1 ]; then
	step "回滚（撤销全部安装，回到首次安装前状态）"
	[ -f "$BACKUP_ROOT/latest" ] || die "没有可回滚的安装记录（$BACKUP_ROOT/latest 不存在）"
	STAMP="$(cat "$BACKUP_ROOT/latest")"
	B="$BACKUP_ROOT/$STAMP"
	[ -f "$B/manifest.txt" ] || die "备份不完整: $B/manifest.txt 缺失"
	if [ "$DRY" -eq 1 ]; then
		echo "  [dry-run] 将用 $B/manifest.txt 恢复 $(grep -c '^M ' "$B/manifest.txt") 个原有文件、"
		echo "            删除 $(grep -c '^A ' "$B/manifest.txt") 个新增文件与 $(grep -c '^D ' "$B/manifest.txt") 个新增目录"
		exit 0
	fi
	nM=0; nA=0; nD=0
	while read -r kind rel; do
		[ -z "${kind:-}" ] && continue
		case "$kind" in
			M) [ -f "$B/files/$rel" ] && cp -a "$B/files/$rel" "$K/$rel" && nM=$((nM+1)) ;;
			A) rm -f "$K/$rel" && nA=$((nA+1)) ;;
			D) rm -rf "$K/$rel" && nD=$((nD+1)) ;;
		esac
	done < "$B/manifest.txt"
	rm -f "$BACKUP_ROOT/latest"
	ok "恢复 $nM 个原有文件 / 删除 $nA 个新增文件 / 删除 $nD 个新增目录"
	ok "备份仍保留在 $B，可手工比对"
	exit 0
fi

# ── 前置检查 ──────────────────────────────────────────────────────────
echo "内核树  : $K"
echo "交付包  : $D"
echo "模式    : $([ "$DRY" -eq 1 ] && echo 'DRY-RUN（不改动文件）' || echo '真实安装')"
echo

command -v git >/dev/null 2>&1 || die "缺 git（用于应用 compatible 补丁）"
if [ ! -f "$T/mt6893.dtsi" ]; then
	warn "内核树缺 SoC 层 mt6893.dtsi（干净主线没有）"
	warn "请先合入目标仓库已有的 SoC 层与 clk-mt6893，否则 dtb 编译会失败"
fi

# ── 备份本次可能改动/新增的全部路径 ───────────────────────────────────
TARGETS=(
	arch/arm64/boot/dts/mediatek/Makefile
	arch/arm64/boot/dts/mediatek/mt6893-xiaomi-ares-official.dts
	arch/arm64/boot/dts/mediatek/mt6893-xiaomi-ares-official.dtb
	arch/arm64/boot/dts/mediatek/mt6893-pinfunc.h
	arch/arm64/configs/defconfig
	drivers/misc/Kconfig
	drivers/misc/Makefile
	drivers/regulator/Kconfig
	drivers/regulator/Makefile
	drivers/leds/Kconfig
	drivers/input/misc/Kconfig
	drivers/input/keyboard/Kconfig
	drivers/input/touchscreen/Kconfig
	drivers/power/supply/Kconfig
	drivers/leds/Makefile
	drivers/input/misc/Makefile
	drivers/input/keyboard/Makefile
	drivers/input/touchscreen/Makefile
	drivers/power/supply/Makefile
	drivers/power/supply/bq25980_charger.c
	drivers/power/supply/bq27xxx_battery_i2c.c
	drivers/regulator/regulator-wl2866d.c
	drivers/leds/leds-aw2028.c
	drivers/input/misc/haptic-aw8697.c
	drivers/input/keyboard/input-gamekey.c
	drivers/input/touchscreen/touchscreen-gt9886.c
	drivers/power/supply/xmusb350-charger.c
	drivers/misc/et7480-audioswitch.c
	drivers/misc/st21nfc.c
	drivers/misc/gf3658-spi.c
)

STAMP="$(date +%Y%m%d-%H%M%S)"
B="$BACKUP_ROOT/$STAMP"
MANIFEST="$B/manifest.txt"
if [ "$DRY" -eq 0 ]; then
	mkdir -p "$B/files"
	: > "$MANIFEST"
	for rel in "${TARGETS[@]}"; do
		if [ -e "$K/$rel" ]; then
			mkdir -p "$B/files/$(dirname "$rel")"
			cp -a "$K/$rel" "$B/files/$rel"
			echo "M $rel" >> "$MANIFEST"
		else
			echo "A $rel" >> "$MANIFEST"
		fi
	done
	[ "$ARTIFACTS" -eq 1 ] && echo "D ares-port" >> "$MANIFEST"
	# 回滚基准固定为「首次安装前」的快照：幂等复跑不刷新 latest，
	# 否则 --rollback 只会退回「上一次安装后」而非干净树。
	if [ -f "$BACKUP_ROOT/latest" ]; then
		ok "改动前快照 -> .ares-port-backup/$STAMP/（回滚基准仍为首次快照）"
	else
		echo "$STAMP" > "$BACKUP_ROOT/latest"
		ok "改动前快照 -> .ares-port-backup/$STAMP/（回滚基准）"
	fi
else
	echo "  · [dry-run] 跳过备份快照"
fi

TMP="$(mktemp -d /tmp/ares_install_XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

# ── 1) 设备树 + pinfunc 头 + dts Makefile 条目 ────────────────────────
step "1/6 安装设备树与 Kbuild 条目（install.sh）"
if [ "$DRY" -eq 1 ]; then
	"$D/install.sh" "$K" --dry-run
else
	"$D/install.sh" "$K" > "$TMP/install.log" 2>&1 || { cat "$TMP/install.log"; die "install.sh 失败"; }
	ok "install.sh 完成（日志: $B/install.log）"
	cp "$TMP/install.log" "$B/install.log" 2>/dev/null || true
fi
# 起机 dtb 一起装（install.sh 只装 dts，靠 make 生成；这里直接给现成产物）
if [ "$DRY" -eq 1 ]; then
	echo "    [dry-run] cp $D/mt6893-xiaomi-ares-official.dtb $T/"
else
	cp "$D/mt6893-xiaomi-ares-official.dtb" "$T/"
	ok "起机 dtb -> $T/mt6893-xiaomi-ares-official.dtb"
fi

# ── 2) 9 个驱动源码 + 9 条 Makefile 规则 + Kconfig ────────────────────
step "2/6 接入 9 个移植驱动（apply.sh）"
if [ "$DRY" -eq 1 ]; then
	"$D/port/integration/apply.sh" "$K" --dry-run
	echo "  （注：apply.sh 末尾提示的 compat 补丁 / defconfig 手工项，"
	echo "    由本脚本步骤 3/4 自动完成，无需手工操作）"
else
	"$D/port/integration/apply.sh" "$K" > "$TMP/apply.log" 2>&1 || { cat "$TMP/apply.log"; die "apply.sh 失败"; }
	ok "apply.sh 完成（9 个 .c + 9 条 Makefile 规则 + Kconfig 菜单）"
	cp "$TMP/apply.log" "$B/apply.log" 2>/dev/null || true
fi

# ── 2b) 消除目标树已有同名 Kconfig 定义合并后的递归依赖 ───────────────
# 典型报错：symbol REGMAP_I2C is selected by REGULATOR_WL2866D /
#           symbol REGULATOR_WL2866D depends on REGMAP_I2C
# 目标树（尤其 mt6893_kernel_ares_mainline）常自带一份 `select REGMAP_I2C`
# 的 WL2866D 定义，与本包旧版 Kconfig 块的 `depends on REGMAP_I2C` 合并成环。
step "2b/6 Kconfig 递归依赖自检与修复"
FIXK="$D/tools/fix_kconfig_recursion.py"
if [ -f "$FIXK" ]; then
	if [ "$DRY" -eq 1 ]; then
		python3 "$FIXK" "$K" --check \
			|| warn "检测到 Kconfig 递归依赖，正式安装时会自动修复"
	else
		python3 "$FIXK" "$K" --quiet \
			|| die "Kconfig 递归依赖修复失败（详见上方输出）"
		ok "Kconfig 递归依赖已排除（同名定义合并检查）"
	fi
else
	warn "缺 tools/fix_kconfig_recursion.py，跳过 Kconfig 递归依赖修复"
fi

# ── 3) A 类 compatible 补丁 + dt-bindings 补丁 ───────────────────────
step "3/6 应用 A 类 compatible / dt-bindings 补丁"
apply_compat() {
	local patch="$1" needle="$2" dest="$3" name
	name="$(basename "$patch")"
	if grep -qF "$needle" "$K/$dest" 2>/dev/null; then
		skip "$name"; return 0
	fi
	# 补丁文件头部带说明注释，从第一个 "--- a/" 起截取为纯 diff
	local clean="$TMP/$name"
	awk 'BEGIN{s=0} /^--- a\//{s=1} s{print}' "$patch" > "$clean"
	if [ "$DRY" -eq 1 ]; then
		echo "    [dry-run] git apply $name -> $dest"; return 0
	fi
	( cd "$K" && git apply --check "$clean" >/dev/null 2>&1 ) \
		|| die "$name 无法应用（目标文件版本不匹配），请手工核对 $dest"
	( cd "$K" && git apply "$clean" ) || die "git apply $name 失败"
	grep -qF "$needle" "$K/$dest" || die "$name 应用后未检测到 $needle"
	ok "$name -> $dest"
}
apply_compat "$D/port/compat/bq25980-add-ares-compatible.patch" \
	'ti,bq2597x-master' drivers/power/supply/bq25980_charger.c
apply_compat "$D/port/compat/bq27xxx-add-ares-compatible.patch" \
	'ti,bq27z561_i2c0' drivers/power/supply/bq27xxx_battery_i2c.c
# GPU：让 arm,mali-bifrost.yaml 接受 mediatek,mt6893-mali（dtbs_check 需要）
apply_compat "$D/port/dt-bindings/mali-mt6893.patch" \
	'mediatek,mt6893-mali' Documentation/devicetree/bindings/gpu/arm,mali-bifrost.yaml
# USB host：让 mtu3 / mtk-xhci 的 compatible enum 接受 mt6893（dtbs_check 需要）
apply_compat "$D/port/dt-bindings/usb-mtu3-mt6893.patch" \
	'mediatek,mt6893-mtu3' Documentation/devicetree/bindings/usb/mediatek,mtu3.yaml
apply_compat "$D/port/dt-bindings/usb-xhci-mt6893.patch" \
	'mediatek,mt6893-xhci' Documentation/devicetree/bindings/usb/mediatek,mtk-xhci.yaml

# ── 4) defconfig 片段合并 ─────────────────────────────────────────────
step "4/6 合并 defconfig 配置片段"
DC="$K/arch/arm64/configs/defconfig"
MARK='# >>> ares port (auto_install.sh) >>>'
if grep -qF "$MARK" "$DC" 2>/dev/null; then
	skip "defconfig 已含 ares 配置块"
else
	{
		echo
		echo "$MARK"
		# 只取 CONFIG_ 赋值行，去掉行内注释（defconfig 不接受行内 # 注释），并去重；
		# 再跳过目标 defconfig 里已存在的「完全同值」项，避免重复赋值触发
		# "override: reassigning to symbol X" 警告（不同值仍会追加，以本包为准）。
		grep -hE '^CONFIG_[A-Za-z0-9_]+=' \
			"$D/ares_defconfig.fragment" \
			"$D/port/integration/defconfig.additions" \
			| sed -E 's/[[:space:]]+#.*$//; s/[[:space:]]+$//' \
			| awk '!seen[$0]++' \
			| while IFS= read -r line; do
				grep -qFx -- "$line" "$DC" || printf '%s\n' "$line"
			  done
		echo '# <<< ares port (auto_install.sh) <<<'
	} > "$TMP/defconfig.block"
	# grep -c 无匹配时退出码为 1；set -e 下会直接终止脚本。
	# 若 defconfig 已由 force_defconfig.py 写成唯一块，这里可能 0 条待追加。
	n_cfg=$(grep -c '^CONFIG_' "$TMP/defconfig.block" || true); [ -z "$n_cfg" ] && n_cfg=0
	if [ "$n_cfg" -eq 0 ]; then
		skip "defconfig 已含全部所需配置（0 条待追加，不写空块）"
	elif [ "$DRY" -eq 1 ]; then
		echo "    [dry-run] 追加 $n_cfg 条 CONFIG 到 $DC（用标记块包裹）"
	else
		cat "$TMP/defconfig.block" >> "$DC"
		ok "追加 $n_cfg 条 CONFIG -> $DC（标记块，便于回滚）"
	fi
fi

# ── 5) 可选附加产物 ───────────────────────────────────────────────────
step "5/6 附加产物（--with-artifacts）"
if [ "$ARTIFACTS" -eq 0 ]; then
	echo "  · 未启用；dtb 已在步骤 1 装入 dts 目录，.ko/docs 留在交付包内"
else
	A="$K/ares-port"
	if [ "$DRY" -eq 1 ]; then
		echo "    [dry-run] mkdir -p $A && 复制 dtb / 9 个 .ko / 7 份 docs / 2 份 fragment"
	else
		mkdir -p "$A"
		cp "$D/mt6893-xiaomi-ares-official.dtb" "$A/"
		cp "$D"/port/*.ko "$A/"
		cp -r "$D/docs" "$A/"
		cp "$D/ares_defconfig.fragment" "$D/port/integration/defconfig.additions" "$A/"
		ok "dtb + $(find "$A" -name '*.ko' | wc -l) 个 .ko + docs + 2 份 fragment -> $A/"
	fi
fi

# ── 6) 安装后自检 ─────────────────────────────────────────────────────
step "6/6 安装后自检"
if [ "$DRY" -eq 1 ]; then
	echo "  · dry-run 未改动文件，跳过自检"
else
	PASS=0; FAILCNT=0
	chk() {
		if eval "$2" >/dev/null 2>&1; then ok "$1"; PASS=$((PASS+1));
		else echo "  ✗ $1"; FAILCNT=$((FAILCNT+1)); fi
	}
	chk "主 dts"            "[ -f '$T/mt6893-xiaomi-ares-official.dts' ]"
	chk "起机 dtb"          "[ -f '$T/mt6893-xiaomi-ares-official.dtb' ]"
	chk "pinfunc 头"        "[ -f '$T/mt6893-pinfunc.h' ]"
	chk "dts Makefile 条目 + -@" "grep -q 'mt6893-xiaomi-ares-official.dtb' '$T/Makefile' && grep -q 'DTC_FLAGS_mt6893-xiaomi-ares-official' '$T/Makefile'"
	chk "Kconfig ares 菜单" "grep -q 'Xiaomi ares (MT6893) vendor peripherals' '$K/drivers/misc/Kconfig'"
	chk "Kconfig 无递归依赖" "[ -f '$FIXK' ] && python3 '$FIXK' '$K' --check --quiet"
	chk "bq25980 compatible" "grep -q 'ti,bq2597x-master' '$K/drivers/power/supply/bq25980_charger.c'"
	chk "bq27xxx compatible" "grep -q 'ti,bq27z561_i2c0' '$K/drivers/power/supply/bq27xxx_battery_i2c.c'"
	# defconfig 可能已被 build_workflow.sh 的 force_defconfig.py 改写成它自己的
	# 唯一标记块（复跑时本脚本 0 条待追加，不再写空块）。两种标记都算配置落地，
	# 再用两条关键 CONFIG 兜底，避免「标记在但配置没了」的假通过。
	chk "defconfig 配置块"  "grep -qE '^# >>> ares (port|build)' '$DC' && grep -q '^CONFIG_REGULATOR_WL2866D=' '$DC' && grep -q '^CONFIG_HAPTIC_AW8697=' '$DC'"
	# 9 个源码 + 9 条 Makefile 规则
	MAP='regulator-wl2866d.c|drivers/regulator|CONFIG_REGULATOR_WL2866D
leds-aw2028.c|drivers/leds|CONFIG_LEDS_AW2028
haptic-aw8697.c|drivers/input/misc|CONFIG_HAPTIC_AW8697
input-gamekey.c|drivers/input/keyboard|CONFIG_KEYBOARD_ARES_GAMEKEY
touchscreen-gt9886.c|drivers/input/touchscreen|CONFIG_TOUCHSCREEN_GT9886
xmusb350-charger.c|drivers/power/supply|CONFIG_CHARGER_XMUSB350
et7480-audioswitch.c|drivers/misc|CONFIG_ET7480_AUDIOSWITCH
st21nfc.c|drivers/misc|CONFIG_NFC_ST21NFC_ARES
gf3658-spi.c|drivers/misc|CONFIG_SPI_GF3658'
	n_src=0; n_rule=0
	while IFS='|' read -r src dir sym; do
		[ -z "$src" ] && continue
		[ -f "$K/$dir/$src" ] && n_src=$((n_src+1))
		grep -qF "obj-\$($sym) += ${src%.c}.o" "$K/$dir/Makefile" 2>/dev/null && n_rule=$((n_rule+1))
	done <<< "$MAP"
	[ "$n_src" -eq 9 ]  && { ok "9 个驱动源码就位"; PASS=$((PASS+1)); } || { echo "  ✗ 驱动源码仅 $n_src/9"; FAILCNT=$((FAILCNT+1)); }
	[ "$n_rule" -eq 9 ] && { ok "9 条 Makefile 规则就位"; PASS=$((PASS+1)); } || { echo "  ✗ Makefile 规则仅 $n_rule/9"; FAILCNT=$((FAILCNT+1)); }
	echo
	echo "  自检: $PASS 通过 / $FAILCNT 失败"
	[ "$FAILCNT" -gt 0 ] && die "安装未完全成功，请检查上方 ✗ 项；可用 --rollback 撤销"
fi

# ── 可选：编译验证 ────────────────────────────────────────────────────
if [ "$BUILD" -eq 1 ] && [ "$DRY" -eq 0 ]; then
	step "编译验证（--build）"
	( cd "$K" && make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb ) \
		> "$TMP/dtb.log" 2>&1 && ok "dtb 编译通过 ($(stat -c%s "$T/mt6893-xiaomi-ares-official.dtb") 字节)" \
		|| { tail -20 "$TMP/dtb.log"; warn "dtb 编译失败（若缺 SoC 层属预期）"; }
	( cd "$D/port" && ./build.sh "$K" ) > "$TMP/mod.log" 2>&1 \
		&& ok "9 个驱动模块编译通过" \
		|| { tail -20 "$TMP/mod.log"; warn "驱动模块编译失败，详见 port/build.log"; }
fi

# ── 汇总 ──────────────────────────────────────────────────────────────
echo
echo "════════════════════════════════════════════════════════════════"
if [ "$DRY" -eq 1 ]; then
	echo " dry-run 结束，未改动任何文件。确认后去掉 --dry-run 正式安装。"
	[ "$BUILD" -eq 1 ] && echo " （--build 在 dry-run 下不执行；正式安装时才编译 dtb 与 9 个模块）"
else
	echo " 安装完成。后续步骤："
	echo "   # 使新增 CONFIG 生效（会重新生成 .config，注意备份自定义配置）"
	echo "   cd $K && make LLVM=1 ARCH=arm64 defconfig"
	echo "   # 编译设备树与模块"
	echo "   make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb"
	echo "   make LLVM=1 ARCH=arm64 modules"
	echo
	echo " 起机 dtb: $T/mt6893-xiaomi-ares-official.dtb"
	echo " 撤销全部安装: $D/auto_install.sh $K --rollback"
	echo " 改动前快照: $B/"
fi
echo "════════════════════════════════════════════════════════════════"
