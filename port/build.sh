#!/usr/bin/env bash
# 用 Linux 7.3.3 内核树对 ares B 类驱动做 arm64 真编译（LLVM/clang 交叉）。
# 不是语法检查：走内核 Kbuild，产出真实 .o / .ko，并经过 modpost。
#
# 用法: ./build.sh [内核源码目录]
set -uo pipefail
K="${1:-/workspace/linux-7.3.3}"
P="$(cd "$(dirname "$0")" && pwd)"
LOG="$P/build.log"

: > "$LOG"
echo "内核树 : $K"
echo "源码   : $P"
echo

# 清理旧产物，保证每次都是全量重编
find "$P" -maxdepth 1 \( -name '*.o' -o -name '*.ko' -o -name '.*.cmd' \
     -o -name '*.mod' -o -name '*.mod.c' -o -name 'Module.symvers' \
     -o -name 'modules.order' \) -delete 2>/dev/null

# 内核树配置必须与编译器一致：若之前用 `make ARCH=arm64 ...dtb`（不带
# LLVM=1）编译过设备树，.config 会被刷成 CONFIG_CC_IS_GCC=y，随后用 clang
# 编外部模块会报 `unknown argument: -fconserve-stack`。此处自动恢复。
if grep -q '^CONFIG_CC_IS_GCC=y' "$K/.config" 2>/dev/null; then
  echo "提示：内核树配置当前为 gcc，正在恢复为 LLVM/clang 状态 ..."
  make -C "$K" LLVM=1 ARCH=arm64 olddefconfig >/dev/null 2>&1
  make -C "$K" LLVM=1 ARCH=arm64 modules_prepare >/dev/null 2>&1
  echo "      已恢复（后续编译 dtb 请统一带 LLVM=1）"
  echo
fi

# 内核树尚无 Module.symvers 时，modpost 的 unresolved 降级为警告。
# CC（编译成 .o）阶段始终执行完整的语法/类型/API 检查，这才是验证点。
MODPOST_OPT=""
if [ ! -s "$K/Module.symvers" ]; then
  MODPOST_OPT="KBUILD_MODPOST_WARN=1"
  echo "提示：内核树尚无 Module.symvers，modpost unresolved 降级为警告"
  echo "      （CC 阶段仍是完整类型检查）。编过 vmlinux 后自动转为严格模式。"
  echo
fi

make -C "$K" LLVM=1 ARCH=arm64 M="$P" $MODPOST_OPT modules 2>&1 | tee -a "$LOG"
rc=${PIPESTATUS[0]}

n_ko=$(find "$P" -maxdepth 1 -name '*.ko' | wc -l)
n_err=$(grep -ciE 'error:' "$LOG")
n_warn=$(grep -ciE 'warning:' "$LOG")

{
  echo
  echo "=== 编译结果 ==="
  echo "  make exit : $rc"
  echo "  生成 .ko  : $n_ko"
  echo "  error 数  : $n_err"
  echo "  warning 数: $n_warn"

  if [ "$rc" -ne 0 ]; then
    echo
    echo "=== 错误明细（前 50）==="
    grep -inE 'error:|Error [0-9]|undefined|No such file' "$LOG" | head -50
  else
    echo
    echo "=== 产出的模块 ==="
    ls -1 "$P"/*.ko 2>/dev/null | sed 's/^/  /'
  fi
} | tee -a "$LOG"

exit "$rc"
