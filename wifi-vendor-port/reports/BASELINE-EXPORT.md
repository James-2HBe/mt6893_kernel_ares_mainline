# ares-r-oss 平台基线层导出报告

由 `tools/build_baseline.py` 生成。来源：`MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`（固定 commit），按 `raw.githubusercontent.com` 逐 blob 下载，并用 `git ls-tree` 的 blob sha 做 git object hash 校验，保证字节保真。

> **分类原则**：只把 **MTK 平台私有头** 当基线导出。`linux/*` 等 4.14 主线头在 7.3 已删除，**不能**把原厂文件搬进 主线的 `linux/` 命名空间覆盖，那是内核 API 适配（见 §4）；私有栈内部头不在这棵空壳仓库里（见 §5）。

| 指标 | 值 |
|---|---|
| 种子缺失 include（去重） | 86 |
| ├ 平台基线类（导出对象） | 57 |
| ├ 4.14 主线头（API 适配） | 10 |
| └ 私有栈内部头 | 42 |
| BFS 轮次 | 3 |
| 导出平台层文件 | 36 |
| 生成的 -I 目录 | 22 |
| 已解析 include 映射 | 37 |
| 候选歧义（需人工确认） | 6 |
| 平台头未能在 ares-r-oss 定位 | 47 |

## 1. 平台基线头 → ares-r-oss 路径映射

| include | 引用次数 | ares-r-oss 路径 | 匹配方式 | 候选数 |
|---|---|---|---|---|
| `memory/mediatek/emi.h` | 17 | `include/memory/mediatek/emi.h` | suffix | 1 |
| `soc/mediatek/emi.h` | 15 | **未定位** | - | - |
| `helio-dvfsrc-opp.h` | 15 | `drivers/devfreq/helio-dvfsrc-opp.h` | suffix | 2 |
| `topo_ctrl.h` | 10 | `drivers/misc/mediatek/performance/include/topo_ctrl.h` | suffix | 1 |
| `conninfra_ext.h` | 7 | **未定位** | - | - |
| `include/pmic_regulator.h` | 7 | `drivers/misc/mediatek/pmic/include/pmic_regulator.h` | suffix | 1 |
| `mtk_clkbuf_ctl.h` | 6 | `drivers/misc/mediatek/base/power/include/mtk_clkbuf_ctl.h` | suffix | 2 |
| `mtk_iommu_ext.h` | 5 | `drivers/iommu/mtk_iommu_ext.h` | suffix | 1 |
| `mtk_battery_internal.h` | 5 | `drivers/power/supply/mediatek/battery/mtk_battery_internal.h` | suffix | 1 |
| `pmic_api_buck.h` | 4 | `drivers/misc/mediatek/pmic/include/pmic_api_buck.h` | suffix | 1 |
| `dt-bindings/memory/mt6885-larb-port.h` | 4 | `include/dt-bindings/memory/mt6885-larb-port.h` | suffix | 1 |
| `mtk_disp_notify.h` | 3 | **未定位** | - | - |
| `mtk_ppm_api.h` | 3 | `drivers/misc/mediatek/base/power/include/mtk_ppm_api.h` | suffix | 2 |
| `dt-bindings/memory/mt6877-larb-port.h` | 3 | **未定位** | - | - |
| `mtk_6306_gpio.h` | 2 | **未定位** | - | - |
| `mtk_pmic_api_buck.h` | 2 | `drivers/misc/mediatek/pmic/include/mt6357/mtk_pmic_api_buck.h` | suffix | 5 |
| `mtk_spm_resource_req.h` | 2 | `drivers/misc/mediatek/base/power/include/mtk_spm_resource_req.h` | suffix | 3 |
| `clk-mt6873-pg.h` | 2 | **未定位** | - | - |
| `mtk_hibernate_dpm.h` | 2 | **未定位** | - | - |
| `mt_clkbuf_ctl.h` | 2 | **未定位** | - | - |
| `mtk_freqhopping.h` | 2 | `drivers/misc/mediatek/freqhopping/mt6885/mtk_freqhopping.h` | suffix | 1 |
| `mtk_freqhopping_drv.h` | 2 | `drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.h` | suffix | 1 |
| `helio-dvfsrc.h` | 2 | `drivers/devfreq/helio-dvfsrc.h` | suffix | 2 |
| `mtk_low_battery_throttling.h` | 2 | **未定位** | - | - |
| `tc1_partition.h` | 2 | **未定位** | - | - |
| `helio-dvfsrc-opp-mt6885.h` | 2 | `drivers/devfreq/helio-dvfsrc-opp-mt6885.h` | suffix | 1 |
| `plat_dbg_info.h` | 2 | `drivers/misc/mediatek/debug_latch/plat_dbg_info/plat_dbg_info.h` | suffix | 1 |
| `dt-bindings/memory/mt6853-larb-port.h` | 2 | **未定位** | - | - |
| `dt-bindings/memory/mt6873-larb-port.h` | 2 | **未定位** | - | - |
| `clk-mt6885-pg.h` | 2 | `drivers/clk/mediatek/clk-mt6885-pg.h` | suffix | 1 |
| `dt-bindings/mmc/mt6885-msdc.h` | 2 | `include/dt-bindings/mmc/mt6885-msdc.h` | suffix | 1 |
| `dt-bindings/clock/mt6885-clk.h` | 2 | `include/dt-bindings/clock/mt6885-clk.h` | suffix | 1 |
| `mach/mt_reg_base.h` | 1 | **未定位** | - | - |
| `mach/mt_irq.h` | 1 | **未定位** | - | - |
| `mach/mt_clkmgr.h` | 1 | **未定位** | - | - |
| `port_ipc.h` | 1 | `drivers/misc/mediatek/eccci/port_ipc.h` | suffix | 1 |
| `ccci_ipc_task_ID.h` | 1 | `drivers/misc/mediatek/eccci/ccci_ipc_task_ID.h` | suffix | 1 |
| `platform_mt6885.h` | 1 | **未定位** | - | - |
| `platform_mt6983.h` | 1 | **未定位** | - | - |
| `reset_fsm.h` | 1 | **未定位** | - | - |
| `reset_fsm_def.h` | 1 | **未定位** | - | - |
| `emi_symbol_hook.h` | 1 | **未定位** | - | - |
| `pmic_lbat_service.h` | 1 | `drivers/misc/mediatek/pmic/include/pmic_lbat_service.h` | suffix | 1 |
| `helio-dvfsrc-opp-v6877.h` | 1 | **未定位** | - | - |
| `emi.h` | 1 | `include/memory/mediatek/emi.h` | suffix | 1 |
| `dramc.h` | 1 | `include/memory/mediatek/dramc.h` | suffix | 2 |
| `clk-mt6833-pg.h` | 1 | **未定位** | - | - |
| `clk-mt6853-pg.h` | 1 | **未定位** | - | - |
| `clk-mt6877-pg.h` | 1 | **未定位** | - | - |
| `mach/emi_mpu.h` | 1 | **未定位** | - | - |
| `mach/power_loss_test.h` | 1 | **未定位** | - | - |
| `dt-bindings/mmc/mt6873-msdc.h` | 1 | **未定位** | - | - |
| `dt-bindings/clock/mt6873-clk.h` | 1 | **未定位** | - | - |
| `dt-bindings/mmc/mt6877-msdc.h` | 1 | **未定位** | - | - |
| `dt-bindings/clock/mt6877-clk.h` | 1 | **未定位** | - | - |
| `dt-bindings/mmc/mt6853-msdc.h` | 1 | **未定位** | - | - |
| `dt-bindings/clock/mt6853-clk.h` | 1 | **未定位** | - | - |

## 2. 候选歧义（打分接近，建议人工确认平台 variant）

- `helio-dvfsrc-opp.h`: `drivers/devfreq/helio-dvfsrc-opp.h`, `drivers/devfreq/helio-dvfsrc-v4/helio-dvfsrc-opp.h`
- `helio-dvfsrc.h`: `drivers/devfreq/helio-dvfsrc.h`, `drivers/devfreq/helio-dvfsrc-v4/helio-dvfsrc.h`
- `mtk_clkbuf_ctl.h`: `drivers/misc/mediatek/base/power/include/mtk_clkbuf_ctl.h`, `drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h`
- `mtk_pmic_api_buck.h`: `drivers/misc/mediatek/pmic/include/mt6357/mtk_pmic_api_buck.h`, `drivers/misc/mediatek/pmic/include/mt6358/mtk_pmic_api_buck.h`, `drivers/misc/mediatek/pmic/include/mt6359/mtk_pmic_api_buck.h`, `drivers/misc/mediatek/pmic/include/mt6359p/mtk_pmic_api_buck.h`, `drivers/misc/mediatek/pmic/include/mt6390/mtk_pmic_api_buck.h`
- `mtk_ppm_api.h`: `drivers/misc/mediatek/base/power/include/mtk_ppm_api.h`, `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h`
- `mtk_spm_resource_req.h`: `drivers/misc/mediatek/base/power/include/mtk_spm_resource_req.h`, `drivers/misc/mediatek/base/power/include/spm/mtk_spm_resource_req.h`, `drivers/misc/mediatek/base/power/include/spm_v4/mtk_spm_resource_req.h`

## 3. 平台类中未能在 ares-r-oss 定位的 include

均为 PLATFORM 类但 ares-r-oss 无对应路径，原因有三：**非 MT6893 平台 variant 头**（该仓库只带 `mt6885-*` 系列）、**4.14 老 mt-plat 路径**（`mach/*` 等，平台层已演进）、**connectivity 私有栈内部头**（空壳仓库不含真驱动）。都不计入平台基线导出；逐条归因与落点见 `compat/PATH-ALIASES.md` §3。

- `soc/mediatek/emi.h`（15 处）
- `conninfra_ext.h`（7 处）
- `mtk_disp_notify.h`（3 处）
- `dt-bindings/memory/mt6877-larb-port.h`（3 处）
- `mtk_6306_gpio.h`（2 处）
- `clk-mt6873-pg.h`（2 处）
- `mtk_hibernate_dpm.h`（2 处）
- `mt_clkbuf_ctl.h`（2 处）
- `mtk_low_battery_throttling.h`（2 处）
- `tc1_partition.h`（2 处）
- `dt-bindings/memory/mt6853-larb-port.h`（2 处）
- `dt-bindings/memory/mt6873-larb-port.h`（2 处）
- `mach/mt_reg_base.h`（1 处）
- `mach/mt_irq.h`（1 处）
- `mach/mt_clkmgr.h`（1 处）
- `platform_mt6885.h`（1 处）
- `platform_mt6983.h`（1 处）
- `reset_fsm.h`（1 处）
- `reset_fsm_def.h`（1 处）
- `emi_symbol_hook.h`（1 处）
- `helio-dvfsrc-opp-v6877.h`（1 处）
- `clk-mt6833-pg.h`（1 处）
- `clk-mt6853-pg.h`（1 处）
- `clk-mt6877-pg.h`（1 处）
- `mach/emi_mpu.h`（1 处）
- `mach/power_loss_test.h`（1 处）
- `dt-bindings/mmc/mt6873-msdc.h`（1 处）
- `dt-bindings/clock/mt6873-clk.h`（1 处）
- `dt-bindings/mmc/mt6877-msdc.h`（1 处）
- `dt-bindings/clock/mt6877-clk.h`（1 处）
- `dt-bindings/mmc/mt6853-msdc.h`（1 处）
- `dt-bindings/clock/mt6853-clk.h`（1 处）
- `helio-dvfsrc-opp-mt6775.h`（1 处）
- `helio-dvfsrc-opp-mt6771.h`（1 处）
- `helio-dvfsrc-opp-mt6768.h`（1 处）
- `helio-dvfsrc-opp-mt6765.h`（1 处）
- `helio-dvfsrc-opp-mt6785.h`（1 处）
- `helio-dvfsrc-opp-mt6873.h`（1 处）
- `helio-dvfsrc-opp-mt6833.h`（1 处）
- `helio-dvfsrc-opp-mt6853.h`（1 处）
- `spm_v2/mtk_spm_resource_req.h`（1 处）
- `spm_v3/mtk_spm_resource_req.h`（1 处）
- `helio-dvfsrc-mt6768.h`（1 处）
- `helio-dvfsrc-mt6765.h`（1 处）
- `helio-dvfsrc-mt67xx.h`（1 处）
- `helio-dvfsrc-mt6775.h`（1 处）
- `helio-dvfsrc-mt6771.h`（1 处）

## 4. 4.14 主线头（7.3 已删除）—— 属 API 适配，不当基线搬

这些 `#include <linux/...>` 在 7.3.3 的 `include/` 下不存在。把 ares-r-oss 的 4.14 原文件放进 `linux/` 会与主线类型定义冲突，因此**本工具不导出**。落地方式：在 `compat/legacy-headers/` 提供最小声明桩（仅编译期），或把 `adapted/` 的调用点改写到 7.3 等价 API。

- `linux/of_gpio.h`（19 处）
- `linux/soc/mediatek/mtk-pm-qos.h`（8 处）
- `linux/hie.h`（5 处）
- `linux/earlysuspend.h`（3 处）
- `linux/mfd/mt6397/rtc_misc.h`（2 处）
- `linux/wakelock.h`（2 处）
- `linux/irqchip/mtk-gic-extend.h`（1 处）
- `asm/arch/pxa2xx-gpio.h`（1 处）
- `linux/ftrace_event.h`（1 处）
- `linux/genhd.h`（1 处）

## 5. 私有栈内部头（非平台基线）

这些属于 connectivity 私有栈内部、条件编译分支或非内核编译单元；ares-r-oss 的 `connectivity/` 只有 6 个空壳 adapter，不含真驱动，seriaTvT 裁剪树也未导出，故无法从该仓库补齐。由目标树部署时按实际启用的 chip/platform 分支确认。

- `gps_mock_hal.h`（5 处）
- `rt_config.h`（5 处）
- `m4u_port.h`（5 处）
- `kernel_to_ctp.h`（4 处）
- `gps_dl_ctp_osal.h`（2 处）
- `gps_mock_mvcd.h`（2 处）
- `pinctrl-mtk-common-v2_debug.h`（1 处）
- `gps_dl_ctp.h`（1 处）
- `gps_reroute.h`（1 处）
- `gps_reroute_emi.h`（1 处）
- `gps_util.h`（1 处）
- `wmt_task.h`（1 处）
- `cos_api.h`（1 处）
- `gps_dl_ctp_log.h`（1 处）
- `conap_scp.h`（1 处）
- `wpa_supp/wpaSuppCmdEvt.h`（1 处）
- `../met/mt6757/platform/mt6757/met_drv_udtl.h`（1 处）
- `../met/mt6799/platform/mt6799/met_drv_udtl.h`（1 处）
- `mmc/host/cmdq_hci.h`（1 处）
- `module_hrtimer.h`（1 处）
- `mt6353/mtk_pmic_api_buck.h`（1 处）
- `mt6335/mtk_pmic_api_buck.h`（1 处）
- `mt6355/mtk_pmic_api_buck.h`（1 处）
- `mt6356/mtk_pmic_api_buck.h`（1 处）
- `mt6358/mtk_pmic_api_buck.h`（1 处）
- `mt6359/mtk_pmic_api_buck.h`（1 处）
- `mt6359p/mtk_pmic_api_buck.h`（1 处）
- `mt6390/mtk_pmic_api_buck.h`（1 处）
- `mt6799/mtk_clkbuf_hw.h`（1 处）
- `mt6775/mtk_clkbuf_hw.h`（1 处）
- `mt6758/mtk_clkbuf_hw.h`（1 处）
- `mt6759/mtk_clkbuf_hw.h`（1 处）
- `mt6763/mtk_clkbuf_hw.h`（1 处）
- `mt6739/mtk_clkbuf_hw.h`（1 处）
- `mt6771/mtk_clkbuf_hw.h`（1 处）
- `mt6765/mtk_clkbuf_hw.h`（1 处）
- `mt6768/mtk_clkbuf_hw.h`（1 处）
- `mt6785/mtk_clkbuf_hw.h`（1 处）
- `mt8168/mtk_clkbuf_hw.h`（1 处）
- `mt6873/mtk_clkbuf_hw.h`（1 处）
- `mt6885/mtk_clkbuf_hw.h`（1 处）
- `mt6853/mtk_clkbuf_hw.h`（1 处）

## 6. 完整性校验

`tools/baseline-manifest.tsv` 逐文件记录 ares-r-oss 的 blob sha；下载时已用 git object hash 与 `git ls-tree` 比对，可用 `git cat-file -p <sha>` 在克隆中复核字节一致。
