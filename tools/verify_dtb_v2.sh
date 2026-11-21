#!/usr/bin/env bash
# 用 Linux 7.3.3 内核树 + cpp + dtc 真编译「官方数据版」ares 设备树，产出 dtb。
# 与 v1 区别：v2 的 dts 含官方 4.14 的 #ifdef 条件编译（由 cpp 依据头部 #define 处理），
#             且基础节点来自官方 mt6893.dts / mt6360.dtsi，不再是 chopin 反编译数据。
# 用法: ./tools/verify_dtb_v2.sh [内核源码目录] [工程目录]
set -u
K="${1:-/workspace/linux-7.3.3}"
D="${2:-/workspace/ares-7.3.3}"
T="${D}/.build"

mkdir -p "$T"
cp "$D/vendor-reference/mt6893.dtsi" "$T/"
cp "$D/vendor-reference/mt6893-xiaomi-chopin.dts" "$T/"
cp "$D/mt6893-xiaomi-ares.dts" "$T/"
cp "$K/arch/arm64/boot/dts/mediatek/mt6893-pinfunc.h" "$T/"
cp "$K/include/dt-bindings/pinctrl/mt65xx.h" "$T/"

INCLUDES=(
  -I "$K/include"
  -I "$K/include/dt-bindings/pinctrl"
  -I "$K/arch/arm64/boot/dts"
  -I "$K/arch/arm64/boot/dts/mediatek"
  -I "$T"
)

preprocess() {
  local name="$1"
  cpp -nostdinc -undef -D__DTS__ -x assembler-with-cpp \
      "${INCLUDES[@]}" "$name.dts" > "$name.pp" 2> "$name.cpp.err"
  local rc=$?
  echo "  cpp  $name: exit=$rc lines=$(wc -l < "$name.pp")"
  if [ -s "$name.cpp.err" ]; then
    echo "  cpp 诊断（前 12）:"
    grep -iE "error|fatal|No such file" "$name.cpp.err" | head -12 | sed 's/^/    /'
  fi
  return $rc
}

compile() {
  local name="$1"
  dtc -I dts -O dtb -o "$name.dtb" "$name.pp" 2> "$name.dtc.err"
  local rc=$?
  echo "  dtc  $name: exit=$rc  dtb=$(stat -c%s "$name.dtb" 2>/dev/null || echo 0) bytes"
  if [ $rc -ne 0 ]; then
    echo "  ---- ERROR（前 40）----"
    grep -iE "ERROR|FATAL" "$name.dtc.err" | head -40 | sed 's/^/    /'
  else
    echo "  警告数: $(grep -c 'Warning' "$name.dtc.err")"
    echo "  警告类型: $(grep -oE 'Warning \(([^)]+)\)' "$name.dtc.err" | sort | uniq -c | sort -rn | tr '\n' ' ')"
  fi
  return $rc
}

cd "$T" || exit 1
echo "=== 基线：chopin（仓库原有，应通过）==="
preprocess mt6893-xiaomi-chopin
compile mt6893-xiaomi-chopin
echo
echo "=== 目标：ares（官方数据版 v2）==="
preprocess mt6893-xiaomi-ares
compile mt6893-xiaomi-ares
