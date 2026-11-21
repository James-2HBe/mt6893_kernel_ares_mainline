#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
# 验证 wifi-mainline.dtsi 的语法与 label 引用自洽（用主线 dtc + stub 顶层）。
# 用法: ./tools/dts_verify.sh [内核源码树] [dts目录]
set -u
K="${1:-/workspace/linux-7.3.3}"
D="${2:-$(cd "$(dirname "$0")/../dts" && pwd)}"
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

cpp -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
  -I "$K/include" -I "$K/arch/arm64/boot/dts/mediatek" -I "$D" \
  "$D/wifi-mainline-verify.dts" > "$TMP/wifi.pp" 2> "$TMP/cpp.err" || {
    echo "FAIL: cpp 预处理"; cat "$TMP/cpp.err"; exit 1; }

dtc -I dts -O dtb -o "$TMP/wifi.dtb" "$TMP/wifi.pp" 2> "$TMP/dtc.err"
rc=$?
if [ "$rc" -ne 0 ]; then
  echo "FAIL: dtc 编译"; cat "$TMP/dtc.err"; exit "$rc"
fi
echo "PASS: wifi-mainline.dtsi 编译通过 ($(stat -c%s "$TMP/wifi.dtb") 字节)"
echo "--- 预期 warning（consys/wifi/bt 共用 0x18000000，原厂真实结构）---"
grep -c 'unique_unit_address' "$TMP/dtc.err" | xargs echo "unique_unit_address 条数:"
# 数字 phandle 残留必须为 0
dtc -I dtb -O dts "$TMP/wifi.dtb" 2>/dev/null > "$TMP/readback.dts"
leftover=0
for ph in 0xed 0x108 0x11e 0x11f 0x127 0x198 0x199; do
  n=$(grep -cE "(phandle|memory-region|pmic|supply) = <$ph>" "$TMP/readback.dts")
  leftover=$((leftover + n))
done
echo "数字 phandle 残留: $leftover (应为 0)"
[ "$leftover" -eq 0 ] || exit 1
echo "PASS: 全部数字 phandle 已还原为 label 引用"
