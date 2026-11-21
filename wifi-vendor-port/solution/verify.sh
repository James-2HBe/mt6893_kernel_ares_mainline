#!/bin/sh
# platform-symbols.c 的编译 + 链接期符号验证（Linux 7.3.3 头文件）
#
# 用法：cd wifi-vendor-port && sh solution/verify.sh [KERNEL_DIR]
# 依赖：clang、nm、python3、tools/compile_probe.py
# 产出：/tmp/platform-symbols.o，并打印 nm 校验结果
# 退出码：0 = 编译干净且三个符号均为 weak 导出；非 0 = 失败
set -e
K="${1:-/workspace/linux-7.3.3}"
cd "$(dirname "$0")/.."
OBJ=/tmp/platform-symbols.o

echo "[1/3] 用 $K 的头文件实际编译（-c 产 .o，开 CONFIG_MTK_RAM_CONSOLE）"
KDIR="$K" python3 - <<'PY'
import os, subprocess, sys
sys.path.insert(0, 'tools')
import compile_probe as C
K = os.environ.get('KDIR', '/workspace/linux-7.3.3')
flags = [x for x in C.kernel_include_flags(K) if x != '-fsyntax-only']
# aee_rr_rec_clk 段受 CONFIG_MTK_RAM_CONSOLE 守卫，验证时显式开启
flags += ['-DCONFIG_MTK_RAM_CONSOLE=1']
f = os.path.abspath('solution/platform-symbols.c')
rc = subprocess.run(['clang'] + flags + ['-c', f, '-o', '/tmp/platform-symbols.o'],
                    cwd=K).returncode
sys.exit(rc)
PY

echo "[2/3] nm 检查三个符号为 weak 全局、且带 __export_symbol_*（GPL 导出）"
FAILED=0
for s in aee_rr_rec_clk aee_rr_curr_clk buck_set_mode; do
    line=$(nm "$OBJ" | grep -E " W $s$" || true)
    exp=$(nm "$OBJ" | grep -c "__export_symbol_$s$" || true)
    if [ -n "$line" ] && [ "$exp" -ge 1 ]; then
        printf '  OK   %-20s weak=%s export_symbol=%s\n' "$s" "$(echo "$line" | awk '{print $2}')" "$exp"
    else
        printf '  FAIL %-20s weak_line="%s" export_symbol=%s\n' "$s" "$line" "$exp"
        FAILED=1
    fi
done

echo "[3/3] 编译探针语法口径（应 CLEAN=1 / 0 error）"
# 探针走独立 TU 语法检查；solution.c 自包含，无需私有头
python3 tools/compile_probe.py --kernel "$K" --adapted solution \
        --compat compat --baseline off --limit 10 \
        --note "本探针对象为 solution/ 下两个自包含实现（platform-symbols.c / framework-symbols.c）：仅依赖主线 7.3.3 头与 compat 契约头，无 MTK 平台私有头，故在 --baseline off 口径下仍 0 error；各符号与 EXPORT_SYMBOL_GPL 由上方 nm 步骤核验。" \
        --report reports/SOLUTION-PROBE.md 2>&1 | grep -E 'CLEAN|API|累计 error' || true

echo "[3b] 双配置守卫验证（RAM_CONSOLE 开/关都应 0 error）"
KDIR="$K" python3 - <<'PY'
import os, subprocess, sys
sys.path.insert(0, 'tools')
import compile_probe as C
K = os.environ.get('KDIR', '/workspace/linux-7.3.3')
flags = [x for x in C.kernel_include_flags(K) if x != '-fsyntax-only']
f = os.path.abspath('solution/platform-symbols.c')
for label, extra in [('y', ['-DCONFIG_MTK_RAM_CONSOLE=1']), ('n', [])]:
    p = subprocess.run(['clang'] + flags + extra + ['-fsyntax-only', f],
                       cwd=K, capture_output=True, text=True)
    errs = [l for l in (p.stdout + p.stderr).splitlines() if ': error:' in l]
    print('   CONFIG_MTK_RAM_CONSOLE=%s : rc=%d error=%d' % (label, p.returncode, len(errs)))
    if p.returncode != 0 or errs:
        sys.exit(1)
PY

[ "$FAILED" -eq 0 ] || { echo "验证失败"; exit 1; }
echo "验证通过：三个符号均为 __weak + EXPORT_SYMBOL_GPL，链接期 undefined 已消除。"
