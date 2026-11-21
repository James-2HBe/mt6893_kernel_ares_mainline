# 平台实现 API 适配与符号导出报告

由 `tools/adapt_platform_impl.py` 生成。输入 `baseline/impl/`（保真，勿改），输出 `baseline/impl-adapted/`。

| 指标 | 值 |
|---|---|
| 适配实现 `.c` | 21 |
| 注入 `mtk_platform_compat.h`（最小接线头） | 21 |
| 平台符号总数 | 33 |
| 已有 `EXPORT_SYMBOL*` | 19 |
| **新补 `EXPORT_SYMBOL_GPL`** | **12** |
| 需目标树基线层/桩模板 | 2 |

## 1. 签名改写（复用 adapt_api.py R1..R5）

| 规则 | 命中次数 |
|---|---|
| R1 class_create 去 owner 参数 | 1 |
| R2 access_ok 去 VERIFY 首参 | 0 |
| R3a vfs_read -> kernel_read | 0 |
| R3b vfs_write -> kernel_write | 0 |
| R4 strlcpy -> strscpy | 0 |
| R5 dma_zalloc_coherent -> dma_alloc_coherent | 0 |

命中为 0 是**预期结果**：源树 `lineage-23.2` 已把高版本 API 回移，18 个实现 `.c` 的真实扫描里没有 `set_fs` / `strlcpy` / `access_ok(VERIFY_*)` / `class_create(THIS_MODULE)` 残留；2 处 `init_timer` 位于 `#if` 低版本死分支，7.3 预处理后不可达（编译探针 `API=0` 印证）。因此不做猜测性重写。

> **兼容头为什么是空的**：初版曾让 `mtk_platform_compat.h` 直接 `#include "mtk_wifi_compat.h"` 复用 WiFi 栈的 7.3 补符号，实测反而把 `struct timeval` / `strlcpy` 宏 / `mm_segment_t` 灌进平台实现，与 7.3 主线头冲突（探针错误从 20 条涨到 171 条）。现已改为最小接线头：不定义任何宏，仅保留一个集中扩展点。真需要补符号时按 `KERNEL_VERSION` 分支在此添加。

## 2. 新补的 EXPORT_SYMBOL_GPL

| 符号 | 落地文件 |
|---|---|
| `get_cur_vcore_opp` | `drivers/devfreq/helio-dvfsrc-opp.c` |
| `smi_reg_restore_sec` | `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u_v2.c` |
| `smi_reg_backup_sec` | `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u_v2.c` |
| `clk_buf_show_status_info` | `drivers/misc/mediatek/base/power/clkbuf_v1/mt6873/mtk_clkbuf_hw.c` |
| `pmic_ldo_vcn18_lp` | `drivers/misc/mediatek/pmic/mt6357/v1/pmic_lp_api.c` |
| `pmic_ldo_vfe28_lp` | `drivers/misc/mediatek/pmic/mt6357/v1/pmic_lp_api.c` |
| `register_pg_callback` | `drivers/clk/mediatek/clk-mt6873-pg.c` |
| `mt_ppm_userlimit_freq_limit_by_others` | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_dummy.c` |
| `ppm_game_mode_change_cb` | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_main.c` |
| `mt_ppm_userlimit_cpu_freq` | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_dummy.c` |
| `force_get_tbat` | `drivers/misc/mediatek/power/mt6873/mtk_battery_intf.c` |
| `vcore_pmic_set_mode` | `drivers/misc/mediatek/pmic/mt6390/v1/pmic.c` |

## 3. 链接期符号状态（stage-4 已复核）

- `buck_set_mode`：**已剔除**。7 次「调用」全是注释、MT6335 专用废 API（MT6893 为 MT6359P）、真实调用 0；`solution/platform-symbols.c` 给 `-EOPNOTSUPP` 兜底。
- `aee_rr_rec_clk`：**已解决**。真实实现在 `ram_console/mtk_ram_console.c:1129`（保真副本 `baseline/reference/`）；函数级移植 + `EXPORT_SYMBOL_GPL` 见 `solution/platform-symbols.c`。

> stage-4 说明：这两个符号曾在本报告被判为「无实现、需空桩」，本轮已复核推翻（详见 `docs/UNRESOLVED-SYMBOLS.md`、`reports/SYMBOL-REFS-CORRECTED.md`）。链接期 `undefined symbol` 由 `solution/platform-symbols.c` 的 `__weak + EXPORT_SYMBOL_GPL` 消除，并经 7.3.3 头文件实际编译与 `nm` 核验。除这两个符号外，全量链接仍需目标树「已修基线层」提供其余平台导出符号的实现。

## 4. 复现

```bash
python3.11 tools/adapt_platform_impl.py \
    --src baseline/impl --dst baseline/impl-adapted \
    --impl-report reports/PLATFORM-IMPL.md \
    --modules reports/BASELINE-MODULES.md \
    --stub docs/PLATFORM-STUBS.c.template \
    --report reports/API-ADAPTATION-IMPL.md

# 编译探针（同一 7.3.3 头文件口径）
python3.11 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
    --adapted baseline/impl-adapted --compat compat \
    --baseline auto --limit 30 --report reports/IMPL-PROBE.md
```
