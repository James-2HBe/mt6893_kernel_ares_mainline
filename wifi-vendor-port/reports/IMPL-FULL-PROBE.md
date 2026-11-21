# 路径 B 21 个 4.14 私有驱动 — 全量编译探针

由 `tools/probe_impl_full.py` 生成。对 `baseline/impl-adapted/` 全部 21 个 `.c` 用 7.3.3 真实 KBUILD_CFLAGS 做 `-fsyntax-only`。

| 指标 | 结果 |
|---|---|
| 编译单元 | 21 |
| 无「7.3 已删除旧 API」残留 | **21/21** |
| 完全干净（0 error） | 21 |
| 仅缺头（fatal file not found） | 0 |
| 旧 API 残留 | 0 |
| 平台符号/类型/其它 | 0 |
| 累计 error/fatal 条 | 0 |

## 每文件状态

| 文件 | 类别 | errors |
|---|---|---|
| `drivers/clk/mediatek/clk-mt6873-pg.c` | CLEAN | 0 |
| `drivers/devfreq/helio-dvfsrc-opp.c` | CLEAN | 0 |
| `drivers/memory/mediatek/emimpu.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/clkbuf_v1/mt6853/mtk_clkbuf_hw.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/clkbuf_v1/mt6873/mtk_clkbuf_hw.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/clkbuf_v1/mt6885/mtk_clkbuf_hw.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_dummy.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_main.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_policy_sys_boost.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/spm/common/mtk_spm_resource_req.c` | CLEAN | 0 |
| `drivers/misc/mediatek/base/power/spm_v4/mtk_spm_resource_req.c` | CLEAN | 0 |
| `drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.c` | CLEAN | 0 |
| `drivers/misc/mediatek/performance/boost_ctrl/topo_ctrl/topo_ctrl.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pmic/common/upmu_lbat_service.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pmic/common/upmu_lbat_service_v2.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pmic/mt6357/v1/pmic_lp_api.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pmic/mt6390/v1/pmic.c` | CLEAN | 0 |
| `drivers/misc/mediatek/power/mt6873/mtk_battery_intf.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c` | CLEAN | 0 |
| `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u_v2.c` | CLEAN | 0 |
| `drivers/usb/mtu3/mtu3_dr.c` | CLEAN | 0 |

详细错误见 `reports/IMPL-FULL-ERRORS.txt`。
