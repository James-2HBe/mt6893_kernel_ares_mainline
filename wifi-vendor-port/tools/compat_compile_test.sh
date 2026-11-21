#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0
# 用目标主线内核的 KBUILD_CFLAGS 真编译 compat/compat_selftest.c，
# 证明 mtk_wifi_compat.h 的补齐与 7.3.x 头文件自洽。
# 用法: ./compat_compile_test.sh [内核源码树路径]   (默认 /workspace/linux-7.3.3)
set -u
K="${1:-/workspace/linux-7.3.3}"
HERE="$(cd "$(dirname "$0")" && pwd)"
C="$HERE/../compat/compat_selftest.c"
H="$HERE/../compat"

if [ ! -f "$K/include/generated/autoconf.h" ]; then
  echo "SKIP: 内核树未配置（缺 $K/include/generated/autoconf.h）"; exit 0
fi

cd "$K" || exit 2
clang -fsyntax-only -nostdinc \
  -I./arch/arm64/include -I./arch/arm64/include/generated -I./include \
  -I./arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi \
  -I./include/uapi -I./include/generated/uapi \
  -I"$H" \
  -include ./include/linux/compiler-version.h \
  -include ./include/linux/kconfig.h \
  -include ./include/linux/compiler_types.h \
  -D__KERNEL__ --target=aarch64-linux-gnu -mlittle-endian -fshort-wchar \
  -funsigned-char -fno-common -fno-PIE -fno-strict-aliasing -std=gnu11 \
  -fms-extensions -Wno-gnu -Wno-microsoft-anon-tag \
  -Wno-unused-command-line-argument -mgeneral-regs-only -O2 \
  "$C"
rc=$?
if [ "$rc" -eq 0 ]; then
  echo "PASS: compat 头在 $(make -s kernelversion 2>/dev/null || echo 7.3.3) 头文件下 -fsyntax-only 通过"
else
  echo "FAIL: compat 头编译未通过 (exit=$rc)"
fi
exit "$rc"
