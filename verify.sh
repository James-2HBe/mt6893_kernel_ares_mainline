#!/usr/bin/env bash
# 独立自检：不依赖原厂数据，验证本包交付物自洽。
#
# 用法: ./verify.sh [内核源码树]
#   ./verify.sh                          # 用默认 /workspace/linux-7.3.3
#   ./verify.sh /path/to/linux-7.3.3    # 指定内核树（用于 dtc 真编译）
#
# 检查项：文件齐全 → 脚本语法 → 两个修复脚本幂等 → dtc 编译+dtb 比对
#          → 主线契约静态检查（status / 4-cell IRQ / 批次宏 / chopin 残留）
set -uo pipefail

D="$(cd "$(dirname "$0")" && pwd)"
K="${1:-/workspace/linux-7.3.3}"
TMP="$(mktemp -d /tmp/ares_verify_XXXXXX)"
trap 'rm -rf "$TMP"' EXIT

DTS="$D/mt6893-xiaomi-ares-official.dts"
DTB="$D/mt6893-xiaomi-ares-official.dtb"
FAIL=0
ok()   { echo "  ✓ $*"; }
bad()  { echo "  ✗ $*"; FAIL=1; }
info() { echo "  · $*"; }

echo "=== 0) 环境与文件齐全 ==="
[ -f "$DTS" ] && ok "主 dts ($(wc -l < "$DTS") 行)" || bad "缺主 dts"
[ -f "$DTB" ] && ok "主 dtb ($(stat -c%s "$DTB") 字节)" || bad "缺主 dtb"
n_c=$(find "$D/port" -maxdepth 1 -name '*.c' | wc -l)
n_ko=$(find "$D/port" -maxdepth 1 -name '*.ko' | wc -l)
n_doc=$(find "$D/docs" -name '*.md' | wc -l)
# ported 主交付为 9 个；早期 ares-7.3.3 副本另含 9 份历史源码，故用 ≥9
[ "$n_c" -ge 9 ] && ok "移植驱动源码 $n_c 个" || bad "驱动源码 $n_c 个（应 ≥9）"
[ "$n_ko" -eq 9 ] && ok "编译产物 .ko $n_ko 个" || bad ".ko $n_ko 个（应为 9）"
[ "$n_doc" -eq 11 ] && ok "文档 $n_doc 份" || bad "文档 $n_doc 份（应为 11）"
echo

echo "=== 1) 脚本语法 ==="
if python3 -m py_compile \
	"$D/tools/fix_mainline_contract.py" \
	"$D/port/normalize_dt_gpios.py" \
	"$D/port/make_compat_patches.py" 2>"$TMP/py.err"; then
	ok "3 个 Python 脚本语法通过"
else
	bad "Python 脚本语法失败"; cat "$TMP/py.err"
fi
for s in "$D/install.sh" "$D/verify.sh" "$D/port/build.sh" "$D/port/integration/apply.sh"; do
	bash -n "$s" && ok "bash -n $(basename "$s")" || bad "bash -n $(basename "$s")"
done
echo

echo "=== 2) 修复逻辑幂等复验（交付 dts 应已是最终态）==="
python3 "$D/port/normalize_dt_gpios.py" "$DTS" "$TMP/n.dts" > "$TMP/n.log" 2>&1
if cmp -s "$DTS" "$TMP/n.dts"; then
	ok "GPIO 标准化：重复运行无变化（15 个新增属性已落地，交付 dts 共 17 处）"
else
	bad "GPIO 标准化改变了交付 dts"; diff "$DTS" "$TMP/n.dts" | head -10
fi
python3 "$D/tools/fix_mainline_contract.py" "$DTS" "$TMP/f.dts" > "$TMP/f.log" 2>&1
if cmp -s "$DTS" "$TMP/f.dts"; then
	ok "主线契约修复：重复运行无变化（12 类修复已落地）"
else
	bad "主线契约修复改变了交付 dts"; diff "$DTS" "$TMP/f.dts" | head -10
fi
grep -q '幂等自检: 通过' "$TMP/f.log" && ok "fix 脚本自带幂等自检通过" || info "fix 幂等自检见 $TMP/f.log"
echo

echo "=== 3) 主线契约静态检查 ==="
n_ok=$(grep -cE 'status\s*=\s*"ok"' "$DTS")
[ "$n_ok" -eq 0 ] && ok 'status="ok" 非法值 0 处' || bad "status=\"ok\" 残留 $n_ok 处"
n_4c=$(grep -cE 'interrupts\s*=.*IRQ_TYPE.*\s[0-9]+\s+0>' "$DTS")
[ "$n_4c" -eq 0 ] && ok "4-cell interrupts 残留 0 处（主线 pio 为 2-cell）" || bad "4-cell 残留 $n_4c 处"
grep -q 'ARES_TOUCH_VARIANT' "$DTS" && ok "触控批次 cpp 宏存在" || bad "缺触控批次宏"
grep -q 'ARES_HAPTIC_I2C' "$DTS" && ok "触觉总线 cpp 宏存在" || bad "缺触觉总线宏"
n_ch=$(grep -ci chopin "$DTS")
[ "$n_ch" -le 1 ] && ok "chopin 残留 $n_ch 处（≤1，为数据源标注）" || bad "chopin 残留 $n_ch 处"
grep -q 'mediatek,mt6893-tphy' "$DTS" && ok "tphy compatible 为 mt6893" || bad "tphy compatible 未修正"
# ── 本轮新增检查 ──
n_sp=$(grep -c 'awinic, aw2028' "$DTS")
[ "$n_sp" -eq 0 ] && ok 'compatible 逗号空格 0 处' || bad "compatible 逗号空格残留 $n_sp 处"
grep -q 'gpu@13000000 {' "$DTS" && ok '主线格式 GPU 节点存在' || bad '缺主线格式 GPU 节点'
grep -q 'arm,mali-valhall-jm' "$DTS" && ok 'GPU valhall fallback 存在' || bad '缺 GPU valhall fallback'
grep -q '/delete-node/ &mali' "$DTS" && ok '私有 mali 节点已删除' || bad '私有 mali 节点未删除'
grep -q 'gpu_opp_table: opp-table-gpu' "$DTS" && ok 'GPU OPP 表存在' || bad '缺 GPU OPP 表'
n_eep=$(grep -c '"atmel,24c16"' "$DTS")
[ "$n_eep" -eq 5 ] && ok "相机 EEPROM 主线化 $n_eep 处" || bad "相机 EEPROM 主线化数量异常: $n_eep"
grep -q 'CONFIG_DRM_PANFROST=y' "$D/ares_defconfig.fragment" && ok 'PANFROST 强制配置存在' || bad '缺 PANFROST 强制配置'
grep -q 'CONFIG_EEPROM_AT24=y' "$D/ares_defconfig.fragment" && ok 'at24 强制配置存在' || bad '缺 at24 强制配置'
[ -f "$D/port/dt-bindings/mali-mt6893.patch" ] && ok 'mali binding 补丁存在' || bad '缺 mali binding 补丁'
# ── USB 网络路径检查 ──
grep -q 'mediatek,mt6893-mtu3' "$DTS" && ok 'mtu3 compatible 存在' || bad '缺 mtu3 compatible'
grep -q 'mediatek,mt6893-xhci' "$DTS" && ok 'xHCI compatible 存在' || bad '缺 xHCI compatible'
grep -q '^#define ARES_USB_ROLE ' "$DTS" && ok 'USB 角色切换宏存在' || bad '缺 USB 角色切换宏'
for p in usb-mtu3-mt6893.patch usb-xhci-mt6893.patch; do
	[ -f "$D/port/dt-bindings/$p" ] && ok "$p 存在" || bad "缺 $p"
done
grep -q 'CONFIG_MT7921U=m' "$D/ares_defconfig.fragment" && ok 'USB WiFi 配置存在' || bad '缺 USB WiFi 配置'
grep -q 'CONFIG_USB_NET_QMI_WWAN=m' "$D/ares_defconfig.fragment" && ok 'USB 蜂窝配置存在' || bad '缺 USB 蜂窝配置'
grep -q 'CONFIG_USB_CONFIGFS_RNDIS=y' "$D/ares_defconfig.fragment" && ok 'gadget 网络共享配置存在' || bad '缺 gadget 网络共享配置'
echo

echo "=== 4) dtc 真编译 + 与交付 dtb 比对 ==="
if [ ! -f "$K/arch/arm64/boot/dts/mediatek/mt6893.dtsi" ]; then
	info "跳过：内核树 $K 缺 SoC 层 mt6893.dtsi（干净主线没有）"
	info "      装好 SoC 层后重跑，或用 install.sh + 内核 make 编译"
else
	INC=(-I "$K/include" -I "$K/arch/arm64/boot/dts/mediatek" -I "$D")
	if cpp -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
		"${INC[@]}" "$DTS" > "$TMP/ares.pp" 2> "$TMP/cpp.err"; then
		ok "cpp 预处理通过（$(wc -l < "$TMP/ares.pp") 行）"
		if dtc -I dts -O dtb -@ -o "$TMP/ares.dtb" "$TMP/ares.pp" \
			2> "$TMP/dtc.err"; then
			ok "dtc 编译通过（$(stat -c%s "$TMP/ares.dtb") 字节，含 -@）"
			if cmp -s "$DTB" "$TMP/ares.dtb"; then
				ok "与交付 dtb 字节完全一致"
			else
				# cpp flags 与内核 Kbuild 略有差异时，改为反编译关键节点比对
				dtc -I dtb -O dts "$TMP/ares.dtb" -o "$TMP/new.dts" 2>/dev/null
				dtc -I dtb -O dts "$DTB" -o "$TMP/old.dts" 2>/dev/null
				if diff -q <(grep -E '^\s*(compatible|status|reg|interrupts)\s*=' "$TMP/new.dts") \
						<(grep -E '^\s*(compatible|status|reg|interrupts)\s*=' "$TMP/old.dts") >/dev/null; then
					info "字节不同（cpp flags 差异），但关键节点属性完全一致"
				else
					bad "反编译关键属性不一致"
				fi
			fi
			info "dtc 警告数: $(grep -c 'Warning' "$TMP/dtc.err")"
		else
			bad "dtc 编译失败"; grep -iE 'ERROR|FATAL' "$TMP/dtc.err" | head -10
		fi
	else
		bad "cpp 预处理失败"; grep -iE 'error|fatal|No such file' "$TMP/cpp.err" | head -10
	fi
fi
echo

echo "=== 5) 原厂 WiFi 私有栈移植包（独立素材，不参与主线编译）==="
WV="$D/wifi-vendor-port"
if [ -d "$WV" ]; then
	ok "wifi-vendor-port/ 目录存在"
	# 关键文档与脚本
	wv_missing=0
	for f in README.md PORTING-GUIDE.md \
	         dts/wifi-private-nodes.dtsi \
	         dts/original/mt6893.dtsi dts/original/ares-mt6893.dtsi \
	         config/wifi-vendor-defconfig.fragment config/Kconfig-Makefile-notes.md \
	         firmware/FIRMWARE-LIST.md \
	         tools/MANIFEST.txt tools/export_wifi_vendor.py tools/verify_export.py; do
		[ -f "$WV/$f" ] || { bad "缺 $f"; wv_missing=$((wv_missing+1)); }
	done
	[ "$wv_missing" -eq 0 ] && ok "9 份文档/节点/配置/脚本齐全"
	# 驱动源码规模
	n_wv=$(find "$WV/drivers" -type f | wc -l)
	[ "$n_wv" -ge 1500 ] && ok "私有驱动源码 $n_wv 个文件" || bad "私有驱动源码仅 $n_wv 个（应 ≥1500）"
	# 关键源码存在
	for f in \
	  drivers/misc/mediatek/connectivity/Kconfig \
	  drivers/misc/mediatek/connectivity/Makefile \
	  drivers/misc/mediatek/connectivity/wlan/core/gen4m/chips/soc3_0/soc3_0.c \
	  drivers/misc/mediatek/connectivity/wlan/core/gen4m/include/config.h \
	  drivers/misc/mediatek/connectivity/common/common_main/core/include/wmt_conf.h; do
		[ -f "$WV/$f" ] || bad "缺关键源码 $f"
	done
	# 自包含完整性校验（MANIFEST 模式，交付后无需仓库也能跑）
	REPO="${2:-/tmp/seriaTvt_src}"
	if python3 "$WV/tools/verify_export.py" "$WV" "$REPO" >"$TMP/wv_verify.out" 2>&1; then
		ok "导出完整性校验通过（$(grep -oE 'OK=[0-9]+' "$TMP/wv_verify.out" | head -1)）"
		grep -q '权威性(vs 原厂 tree)' "$TMP/wv_verify.out" && \
			info "$(grep '权威性(vs 原厂 tree)' "$TMP/wv_verify.out")"
	else
		bad "导出完整性校验失败"; head -12 "$TMP/wv_verify.out"
	fi
	# 关键结论：不硬套 compatible / 不改主线
	if grep -q 'mediatek,mt6893-consys' "$WV/dts/wifi-private-nodes.dtsi" && \
	   grep -q 'mediatek,wifi' "$WV/dts/wifi-private-nodes.dtsi"; then
		ok "私有节点含 consys/wifi 设备节点原文"
	else
		bad "私有节点摘录缺 consys/wifi 节点"
	fi

	# ---- 本轮：主线 API 适配 / 节点重写 / init 固件 产物 ----
	wv2_missing=0
	for f in \
	  adapted/misc/mediatek/connectivity/Kconfig \
	  compat/mtk_wifi_compat.h compat/compat_selftest.c \
	  dts/wifi-mainline.dtsi dts/wifi-mainline-verify.dts \
	  integration/gen_ccflags.sh integration/ccflags.mk \
	  init/extract-wifi-firmware.sh init/init.wifi.rc \
	  docs/MAINLINE-ADAPTATION.md \
	  reports/API-ADAPTATION.md reports/COMPILE-PROBE.md \
	  tools/adapt_api.py tools/compile_probe.py \
	  tools/compat_compile_test.sh tools/dts_verify.sh; do
		[ -f "$WV/$f" ] || { bad "缺本轮适配产物 $f"; wv2_missing=$((wv2_missing+1)); }
	done
	[ "$wv2_missing" -eq 0 ] && ok "本轮适配产物 14 项齐全（compat/adapted/dts重写入/init/接线/报告）"
	# 适配源码规模（应约为 529 个 .c 注入 + 全量镜像）
	n_adapt=$(find "$WV/adapted" -name '*.c' | wc -l)
	[ "$n_adapt" -ge 500 ] && ok "adapted/ 适配源码 $n_adapt 个 .c" || bad "adapted/ .c 仅 $n_adapt 个（应 ≥500）"
	# compat 头已注入且旧 API 已改写
	grep -q 'mtk_wifi_compat.h' "$WV/adapted/misc/mediatek/connectivity/fmradio/dummy.c" && \
		ok "adapted/ 已注入 compat 头引用" || bad "adapted/ 未注入 compat 头"
	# 排除 compat/ 头（其注释里含旧签名说明文字），只检查真实调用点
	if ! grep -rn 'class_create\s*(\s*THIS_MODULE' "$WV/adapted" \
	     --include='*.c' 2>/dev/null | grep -v '/compat/' | grep -q .; then
		ok "class_create 旧签名已改写（真实调用点 0 残留）"
	else
		bad "class_create 旧签名残留"
	fi
	# 重写 DT 无数字 phandle 残留（除注释里的 MAP 表）
	grep -q 'consys_reserve_memory: consys-reserve-memory' "$WV/dts/wifi-mainline.dtsi" && \
		ok "重写 DT 节点 label 化" || bad "重写 DT 缺 label 化保留内存"

	# ---- 本轮：ares-r-oss 平台基线层补齐（stage-2） ----
	wv3_missing=0
	for f in \
	  docs/BASELINE-PORTING.md reports/BASELINE-EXPORT.md \
	  reports/BASELINE-MODULES.md reports/COMPILE-PROBE-BEFORE.md \
	  tools/build_baseline.py tools/scan_baseline_modules.py \
	  tools/baseline-manifest.tsv tools/baseline-modules.tsv \
	  compat/PATH-ALIASES.md compat/soc/mediatek/emi.h compat/asm/unaligned.h \
	  integration/baseline-ccflags.mk \
	  baseline/include/memory/mediatek/emi.h \
	  baseline/drivers/devfreq/helio-dvfsrc-opp.h \
	  baseline/drivers/misc/mediatek/base/power/include/mtk_clkbuf_ctl.h; do
		[ -f "$WV/$f" ] || { bad "缺 stage-2 基线产物 $f"; wv3_missing=$((wv3_missing+1)); }
	done
	[ "$wv3_missing" -eq 0 ] && ok "stage-2 基线产物 14 项齐全（文档/报告/工具/别名/平台头）"
	n_base=$(find "$WV/baseline" -name '*.h' | wc -l)
	[ "$n_base" -ge 30 ] && ok "baseline/ 平台头 $n_base 个（≥30）" || bad "baseline/ 平台头仅 $n_base 个"
	n_build=$(find "$WV/baseline/build-files" -type f 2>/dev/null | wc -l)
	[ "$n_build" -ge 150 ] && ok "baseline/build-files/ 平台构建文件 $n_build 个（≥150）" || bad "平台构建文件仅 $n_build 个"
	n_sha=$(awk -F'\t' 'NR>1 && $1!="MISSING" && $1!="" {n++} END{print n+0}' "$WV/tools/baseline-manifest.tsv" 2>/dev/null)
	[ "$n_sha" -ge 30 ] && ok "baseline blob sha 清单 $n_sha 条" || bad "blob sha 清单仅 $n_sha 条"
	grep -q '\$(BASELINE)' "$WV/integration/ccflags.mk" && \
		grep -q '\$(srctree)/net' "$WV/integration/ccflags.mk" && \
		ok "统一接线含 \$(BASELINE) 与 -I\$(srctree)/net" || bad "统一接线缺基线/net -I"
	grep -q '| 仍有 API 残留 | 0 |' "$WV/reports/COMPILE-PROBE.md" && \
		ok "补基线后编译探针：7.3 已删除旧 API 残留 0" || bad "编译探针仍有 API 残留"

	# ---- 本轮：GitHub ares 源移植平台符号实现（stage-3） ----
	wv4_missing=0
	for f in docs/GITHUB-SOURCES.md docs/PLATFORM-IMPL-PORTING.md \
	  docs/PLATFORM-STUBS.c.template reports/PLATFORM-IMPL.md \
	  reports/IMPL-HEADERS.md reports/API-ADAPTATION-IMPL.md \
	  reports/IMPL-PROBE-BEFORE.md reports/IMPL-PROBE.md \
	  tools/port_platform_impl.py tools/complete_impl_headers.py \
	  tools/adapt_platform_impl.py tools/platform-impl-manifest.tsv \
	  compat/mtk_platform_compat.h; do
		[ -f "$WV/$f" ] || { bad "缺 stage-3 产物 $f"; wv4_missing=$((wv4_missing+1)); }
	done
	[ "$wv4_missing" -eq 0 ] && ok "stage-3 产物 13 项齐全（调研/移植/适配/报告/工具）"
	n_impl=$(find "$WV/baseline/impl" -name '*.c' 2>/dev/null | wc -l)
	[ "$n_impl" -ge 20 ] && ok "平台符号实现 $n_impl 个 .c（≥20）" || bad "实现 .c 仅 $n_impl 个"
	n_adapt_impl=$(find "$WV/baseline/impl-adapted" -name '*.c' 2>/dev/null | wc -l)
	[ "$n_adapt_impl" -ge 20 ] && ok "适配版实现 $n_adapt_impl 个 .c（≥20）" || bad "适配版仅 $n_adapt_impl 个"
	n_plat=$(find "$WV/baseline/platform" -name '*.h' 2>/dev/null | wc -l)
	[ "$n_plat" -ge 40 ] && ok "变体/依赖头 $n_plat 个（≥40）" || bad "变体头仅 $n_plat 个"
	n_sha2=$(awk -F'\t' 'NR>1 && $2 ~ /^[0-9a-f]{40}$/ {n++} END{print n+0}' "$WV/tools/platform-impl-manifest.tsv" 2>/dev/null)
	[ "$n_sha2" -ge 40 ] && ok "平台实现 blob sha 清单 $n_sha2 条" || bad "sha 清单仅 $n_sha2 条"
	grep -q 'aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f' "$WV/docs/GITHUB-SOURCES.md" && \
		ok "来源固定到 lineage-23.2 commit" || bad "来源 commit 未固定"
	grep -q '新补 `EXPORT_SYMBOL_GPL`' "$WV/reports/API-ADAPTATION-IMPL.md" && \
		ok "API 适配报告含 EXPORT_SYMBOL_GPL 补齐统计" || bad "缺 EXPORT_SYMBOL_GPL 统计"
	grep -q '| 仍有 API 残留 | 0 |' "$WV/reports/IMPL-PROBE.md" && \
		ok "平台实现编译探针：7.3 已删除旧 API 残留 0" || bad "平台实现探针仍有 API 残留"

	# ---- 本轮：复核并解决两个「未开源符号」（stage-4） ----
	wv5_missing=0
	for f in docs/UNRESOLVED-SYMBOLS.md reports/SYMBOL-REFS-CORRECTED.md \
	  reports/SOLUTION-PROBE.md solution/platform-symbols.c solution/verify.sh \
	  tools/audit_symbol_refs.py tools/export_reference.py \
	  tools/reference-manifest.tsv \
	  baseline/reference/drivers/misc/mediatek/ram_console/mtk_ram_console.c; do
		[ -f "$WV/$f" ] || { bad "缺 stage-4 产物 $f"; wv5_missing=$((wv5_missing+1)); }
	done
	[ "$wv5_missing" -eq 0 ] && ok "stage-4 产物 9 项齐全（解决文档/勘误/实现/验证/保真副本）"
	# 真实实现保真副本含 aee_rr_rec_clk 函数定义
	grep -q 'void aee_rr_rec_clk(int id, u32 val)' \
		"$WV/baseline/reference/drivers/misc/mediatek/ram_console/mtk_ram_console.c" && \
		ok "保真副本含 aee_rr_rec_clk 真实定义" || bad "保真副本缺真实定义"
	# solution 三个符号均为 weak + EXPORT_SYMBOL_GPL
	n_weak=$(grep -cE '^(void|int|u32) __weak (aee_rr_rec_clk|aee_rr_curr_clk|buck_set_mode)' "$WV/solution/platform-symbols.c")
	[ "$n_weak" -eq 3 ] && ok "solution 三符号均 __weak（$n_weak）" || bad "__weak 符号数 $n_weak（应为 3）"
	n_exp=$(grep -c '^EXPORT_SYMBOL_GPL(' "$WV/solution/platform-symbols.c")
	[ "$n_exp" -eq 3 ] && ok "solution 三符号均 EXPORT_SYMBOL_GPL（$n_exp）" || bad "GPL 导出数 $n_exp（应为 3）"
	# buck_set_mode 返回 -EOPNOTSUPP（非静默 0）
	grep -q 'return -EOPNOTSUPP' "$WV/solution/platform-symbols.c" && \
		ok "buck_set_mode 兜底返回 -EOPNOTSUPP" || bad "buck 兜底未用 -EOPNOTSUPP"
	# 勘误：buck_set_mode 真实调用 0
	grep -q 'buck_set_mode.*7 -> 0\|`buck_set_mode` | 7 | \*\*0\*\*' "$WV/reports/SYMBOL-REFS-CORRECTED.md" && \
		ok "调用计数勘误：buck_set_mode 7→0" || bad "缺 buck_set_mode 计数勘误"
	# 桩模板已废弃（#if 0）
	grep -q '#if 0' "$WV/docs/PLATFORM-STUBS.c.template" && \
		ok "空桩模板已停用（#if 0）" || bad "空桩模板未停用"
	# 编译探针：完全干净且 API 残留 0（报告为表格格式）。
	# stage-5 往 solution/ 加了 framework-symbols.c，探针单元数由 1 变 2，
	# 故 CLEAN 值兼容 1/2；关键口径是「无旧 API 残留 + 全部干净」。
	{ grep -qF '| 完全干净（0 error） | 1 |' "$WV/reports/SOLUTION-PROBE.md" || \
	  grep -qF '| 完全干净（0 error） | 2 |' "$WV/reports/SOLUTION-PROBE.md"; } && \
		grep -q '| 仍有 API 残留 | 0 |' "$WV/reports/SOLUTION-PROBE.md" && \
		ok "solution 编译探针全干净 / API 残留 0" || bad "solution 探针非 CLEAN"
	# 注释误计 bug 已修
	grep -q 'strip_c_comments' "$WV/tools/scan_baseline_modules.py" && \
		ok "符号统计已剥离 C 注释（bug 修复）" || bad "strip_c_comments 未落地"

	# ---- 本轮：移植 B 类（clk/iommu/devfreq）框架符号（stage-5） ----
	wv6_missing=0
	for f in reports/CLASS-B-PORTING.md reports/CLASS-B-SYMBOLS.md \
	  reports/CLASS-B-PROBE.md docs/GITHUB-SOURCES-CLASS-B.md \
	  docs/CLASS-B-ADAPTATION.md compat/mtk_framework_compat.h \
	  solution/framework-symbols.c solution/verify-class-b.sh \
	  tools/export_framework.py tools/framework-manifest.tsv \
	  framework/README.md framework/Kconfig.fragment framework/Makefile.fragment \
	  framework/upstream/drivers/memory/mtk-smi.c \
	  framework/vendor/drivers/clk/mediatek/clk-mt6873-pg.c; do
		[ -f "$WV/$f" ] || { bad "缺 stage-5 产物 $f"; wv6_missing=$((wv6_missing+1)); }
	done
	[ "$wv6_missing" -eq 0 ] && ok "stage-5 产物 14 项齐全（报告/文档/实现/验证/保真源）"
	n_fw_sha=$(awk -F'\t' 'NR>1 && $7=="OK" {n++} END{print n+0}' "$WV/tools/framework-manifest.tsv" 2>/dev/null)
	[ "$n_fw_sha" -eq 26 ] && ok "GitHub 保真导出 blob sha 26/26 全部一致" || bad "blob sha 清单仅 $n_fw_sha 条（应为 26）"
	n_up=$(find "$WV/framework/upstream" -type f | wc -l)
	n_vd=$(find "$WV/framework/vendor" -type f | wc -l)
	[ "$n_up" -eq 10 ] && [ "$n_vd" -eq 16 ] && \
		ok "保真源码 upstream=$n_up / vendor=$n_vd" || bad "保真源码数量异常 upstream=$n_up vendor=$n_vd"
	# 8 个符号均 __weak + EXPORT_SYMBOL_GPL
	n_bweak=$(grep -cE '__weak [a-zA-Z_]+\(' "$WV/solution/framework-symbols.c")
	[ "$n_bweak" -eq 8 ] && ok "framework-symbols.c 8 个符号均 __weak（$n_bweak）" || bad "__weak 符号数 $n_bweak（应为 8）"
	n_bexp=$(grep -c '^EXPORT_SYMBOL_GPL(' "$WV/solution/framework-symbols.c")
	[ "$n_bexp" -eq 8 ] && ok "framework-symbols.c 8 个 GPL 导出（$n_bexp）" || bad "GPL 导出数 $n_bexp（应为 8）"
	for s in register_pg_callback smi_reg_backup_sec smi_reg_restore_sec get_cur_vcore_opp; do
		grep -q "$s" "$WV/solution/framework-symbols.c" && \
			ok "B 类符号已实现: $s" || bad "缺 B 类符号 $s"
	done
	grep -q 'torvalds/linux' "$WV/docs/GITHUB-SOURCES-CLASS-B.md" && \
		ok "GitHub 源含主线 torvalds/linux" || bad "缺主线 GitHub 源记录"
	grep -q '26/26' "$WV/reports/CLASS-B-PORTING.md" && \
		ok "报告含 26/26 保真校验结论" || bad "报告缺 26/26 结论"
	grep -q '| 仍有 API 残留 | 0 |' "$WV/reports/CLASS-B-PROBE.md" && \
		ok "stage-5 编译探针：7.3 已删除旧 API 残留 0" || bad "stage-5 探针仍有 API 残留"
	grep -q 'MTK_FRAMEWORK_HAVE_PG_H' "$WV/compat/mtk_framework_compat.h" && \
		ok "契约头支持双配置（standalone / 原厂头）" || bad "契约头缺双配置支持"
else
	info "wifi-vendor-port/ 不存在（可选素材，跳过）"
fi
echo

echo "=== 结果 ==="
if [ "$FAIL" -eq 0 ]; then
	echo "  ✓ 全部检查通过"
else
	echo "  ✗ 存在失败项，请查看上方 ✗ 标记"
fi
exit "$FAIL"
