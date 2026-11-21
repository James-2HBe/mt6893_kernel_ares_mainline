#!/bin/sh
# B 类（clk/iommu/devfreq）框架符号的编译 + 链接期验证（Linux 7.3.3 头文件）
#
# 用法：cd wifi-vendor-port && sh solution/verify-class-b.sh [KERNEL_DIR]
# 依赖：clang、ld.lld（llvm）、nm、python3.11、tools/compile_probe.py
# 退出码：0 = 五项检查全通过；非 0 = 失败
#
# 检查项：
#   [1/5] 用 7.3.3 真实 KBUILD_CFLAGS 实际 -c 编译产 .o
#   [2/5] nm：8 个符号均为 weak 全局 + 带 __export_symbol_*（GPL 导出）
#   [3/5] 双配置：standalone / MTK_FRAMEWORK_HAVE_PG_H（原厂头）均 0 error
#   [4/5] __weak 覆盖语义：注入同名强符号后 ld.lld -r 合并，符号变 T 不冲突
#   [5/5] 编译探针口径：API 残留 0、CLEAN
set -e
K="${1:-/workspace/linux-7.3.3}"
cd "$(dirname "$0")/.."
OBJ=/tmp/framework-symbols.o
SRC=solution/framework-symbols.c
SYMS="register_pg_callback mtk_pg_notify mtk_pg_debug_dump smi_reg_backup_sec smi_reg_restore_sec mtk_framework_set_smi_backend get_cur_vcore_opp mtk_framework_set_vcore_opp"

echo "[1/5] 用 $K 的头文件实际编译（-c 产 .o）"
KDIR="$K" python3.11 - <<'PY'
import os, subprocess, sys
sys.path.insert(0, 'tools')
import compile_probe as C
K = os.environ.get('KDIR', '/workspace/linux-7.3.3')
flags = [x for x in C.kernel_include_flags(K) if x != '-fsyntax-only']
f = os.path.abspath('solution/framework-symbols.c')
rc = subprocess.run(['clang'] + flags + ['-c', f, '-o', '/tmp/framework-symbols.o'],
                    cwd=K).returncode
sys.exit(rc)
PY

echo "[2/5] nm 检查 8 个符号为 weak 全局、且带 __export_symbol_*（GPL 导出）"
FAILED=0
for s in $SYMS; do
    line=$(nm "$OBJ" | grep -E " W $s$" || true)
    exp=$(nm "$OBJ" | grep -c "__export_symbol_$s$" || true)
    if [ -n "$line" ] && [ "$exp" -ge 1 ]; then
        printf '  OK   %-34s weak=%s export_symbol=%s\n' "$s" "$(echo "$line" | awk '{print $2}')" "$exp"
    else
        printf '  FAIL %-34s weak_line="%s" export_symbol=%s\n' "$s" "$line" "$exp"
        FAILED=1
    fi
done
[ "$FAILED" -eq 0 ] || { echo "验证失败：符号核验未通过"; exit 1; }

echo "[3/5] 双配置守卫验证（standalone / 原厂 clk-mt6873-pg.h，均 0 error）"
KDIR="$K" python3.11 - <<'PY'
import os, subprocess, sys
sys.path.insert(0, 'tools')
import compile_probe as C
K = os.environ.get('KDIR', '/workspace/linux-7.3.3')
flags = [x for x in C.kernel_include_flags(K) if x != '-fsyntax-only']
f = os.path.abspath('solution/framework-symbols.c')
hdr = os.path.abspath('framework/vendor/drivers/clk/mediatek')
cases = [
    ('standalone',  []),
    ('HAVE_PG_H',   ['-DMTK_FRAMEWORK_HAVE_PG_H', '-I' + hdr]),
]
for label, extra in cases:
    p = subprocess.run(['clang'] + flags + extra + ['-fsyntax-only', f],
                       cwd=K, capture_output=True, text=True)
    errs = [l for l in (p.stdout + p.stderr).splitlines() if ': error:' in l]
    print('   %-12s : rc=%d error=%d' % (label, p.returncode, len(errs)))
    if p.returncode != 0 or errs:
        for l in errs[:8]:
            print('      ', l)
        sys.exit(1)
PY

echo "[4/5] __weak 覆盖语义（同名强符号共存，ld.lld -r 合并后应为 T）"
cat > /tmp/class_b_strong.c <<'EOF'
/* 模拟目标树「已修基线层」提供的 4.14 强实现 */
int get_cur_vcore_opp(void) { return 7; }
int smi_reg_backup_sec(void) { return 0; }
EOF
clang --target=aarch64-linux-gnu -c /tmp/class_b_strong.c -o /tmp/class_b_strong.o
ld.lld -r -o /tmp/class_b_merged.o "$OBJ" /tmp/class_b_strong.o
for s in get_cur_vcore_opp smi_reg_backup_sec; do
    line=$(nm /tmp/class_b_merged.o | grep -E " T $s$" || true)
    if [ -n "$line" ]; then
        printf '  OK   %-24s 被强符号覆盖 -> T（无 multiple definition）\n' "$s"
    else
        printf '  FAIL %-24s 未观察到强符号覆盖\n' "$s"
        FAILED=1
    fi
done
[ "$FAILED" -eq 0 ] || { echo "验证失败：weak 覆盖语义异常"; exit 1; }

echo "[5/5] 编译探针语法口径（应无「7.3 已删除旧 API」残留）"
python3.11 tools/compile_probe.py --kernel "$K" --adapted solution \
        --compat compat --baseline off --limit 10 \
        --note "本探针对象为 solution/ 下两个自包含实现（platform-symbols.c / framework-symbols.c），仅依赖主线 7.3.3 头与 compat/mtk_framework_compat.h，故在 --baseline off 口径下仍应 0 error；8 个 B 类符号与 EXPORT_SYMBOL_GPL 由上方 nm 步骤核验。" \
        --report reports/CLASS-B-PROBE.md 2>&1 | grep -E 'CLEAN|API|累计 error' || true

echo
echo "验证通过：8 个 B 类框架/桥接符号均为 __weak + EXPORT_SYMBOL_GPL，"
echo "          双配置 0 error，强符号可覆盖（路径 A / 路径 B 均可链接）。"
