# ares-r-oss 平台基线层：实现模块与链接期符号清单

由 `tools/scan_baseline_modules.py` 生成。`build_baseline.py` 补的是**头文件**（编译期）；本文件补的是**实现落点**（链接期）。

| 指标 | 值 |
|---|---|
| 保真导出的平台层 Makefile/Kconfig | 182 |
| 解析出的 kbuild 目标（obj-y / obj-$(CONFIG_X)） | 709 |
| 涉及 Kconfig 开关 | 179 |
| baseline/ 头中声明的函数 | 410 |
| 其中在 adapted/ 被调用（链接期需提供） | 33 |

## 1. 平台驱动模块（按目录分组）

### `drivers/clk/mediatek`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-apmixed.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-cpumux.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-fixup-div.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-gate.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-mtk.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-mux.o` |  |
| `CONFIG_COMMON_CLK_MEDIATEK` | `clk-pll.o` |  |
| `CONFIG_COMMON_CLK_MT2701` | `clk-mt2701.o` | Clock driver for Mediatek MT2701 |
| `CONFIG_COMMON_CLK_MT2701_BDPSYS` | `clk-mt2701-bdp.o` | Clock driver for Mediatek MT2701 bdpsys |
| `CONFIG_COMMON_CLK_MT2701_ETHSYS` | `clk-mt2701-eth.o` | Clock driver for Mediatek MT2701 ethsys |
| `CONFIG_COMMON_CLK_MT2701_HIFSYS` | `clk-mt2701-hif.o` | Clock driver for Mediatek MT2701 hifsys |
| `CONFIG_COMMON_CLK_MT2701_IMGSYS` | `clk-mt2701-img.o` | Clock driver for Mediatek MT2701 imgsys |
| `CONFIG_COMMON_CLK_MT2701_MMSYS` | `clk-mt2701-mm.o` | Clock driver for Mediatek MT2701 mmsys |
| `CONFIG_COMMON_CLK_MT2701_VDECSYS` | `clk-mt2701-vdec.o` | Clock driver for Mediatek MT2701 vdecsys |
| `CONFIG_COMMON_CLK_MT6739` | `clk-fixed-factor.o` | Clock driver for Mediatek MT6739 |
| `CONFIG_COMMON_CLK_MT6739` | `clk-mt6739.o` | Clock driver for Mediatek MT6739 |
| `CONFIG_COMMON_CLK_MT6739` | `clk-mt6739-pg.o` | Clock driver for Mediatek MT6739 |
| `CONFIG_COMMON_CLK_MT6739` | `clk-mtk-v1.o` | Clock driver for Mediatek MT6739 |
| `CONFIG_COMMON_CLK_MT6739` | `mt6739_clkmgr.o` | Clock driver for Mediatek MT6739 |
| `CONFIG_COMMON_CLK_MT6765` | `clk-mt6765.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6765` | `clk-mt6765-pg.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6765` | `clk-mtk-v1.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6765` | `clkchk-mt6765.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6765` | `clkdbg.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6765` | `clkdbg_mt6765.o` | Clock driver for Mediatek MT6765 |
| `CONFIG_COMMON_CLK_MT6768` | `clk-mt6768.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `clk-mt6768-pg.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `clk-mtk-v1.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `clkchk-mt6768.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `clkdbg.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `clkdbg-mt6768.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6768` | `mt6768_clkmgr.o` | Clock driver for Mediatek MT6768 |
| `CONFIG_COMMON_CLK_MT6771` | `clk-mt6771.o` | Clock driver for Mediatek MT6771 |
| `CONFIG_COMMON_CLK_MT6771` | `clk-mt6771-pg.o` | Clock driver for Mediatek MT6771 |
| `CONFIG_COMMON_CLK_MT6771` | `clk-mtk-v1.o` | Clock driver for Mediatek MT6771 |
| `CONFIG_COMMON_CLK_MT6771` | `clkdbg.o` | Clock driver for Mediatek MT6771 |
| `CONFIG_COMMON_CLK_MT6785` | `clk-mt6785.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clk-mt6785-pg.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clk-mtk-v1.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clkchk.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clkchk-mt6785.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clkdbg.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6785` | `clkdbg-mt6785.o` | Clock driver for Mediatek MT6785 |
| `CONFIG_COMMON_CLK_MT6797` | `clk-mt6797.o` | Clock driver for Mediatek MT6797 |
| `CONFIG_COMMON_CLK_MT6797_IMGSYS` | `clk-mt6797-img.o` | Clock driver for Mediatek MT6797 imgsys |
| `CONFIG_COMMON_CLK_MT6797_MMSYS` | `clk-mt6797-mm.o` | Clock driver for Mediatek MT6797 mmsys |
| `CONFIG_COMMON_CLK_MT6797_VDECSYS` | `clk-mt6797-vdec.o` | Clock driver for Mediatek MT6797 vdecsys |
| `CONFIG_COMMON_CLK_MT6797_VENCSYS` | `clk-mt6797-venc.o` | Clock driver for Mediatek MT6797 vencsys |
| `CONFIG_COMMON_CLK_MT6833` | `clk-bringup.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clk-fmeter-mt6833.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clk-mt6833.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clk-mt6833-pg.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clk-mtk-v1.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clkchk.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clkchk-mt6833.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clkdbg.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833` | `clkdbg-mt6833.o` | Clock driver for MediaTek MT6833 |
| `CONFIG_COMMON_CLK_MT6833_AUDIO` | `clk-mt6833-audsys.o` | Clock driver for MediaTek MT6833 audio |
| `CONFIG_COMMON_CLK_MT6833_CAMSYS_MAIN` | `clk-mt6833-cam_m.o` | Clock driver for MediaTek MT6833 camsys_main |
| `CONFIG_COMMON_CLK_MT6833_CAMSYS_RAWA` | `clk-mt6833-cam_ra.o` | Clock driver for MediaTek MT6833 camsys_rawa |

### `drivers/devfreq`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_ARM_EXYNOS_BUS_DEVFREQ` | `exynos-bus.o` | ARM EXYNOS Generic Memory Bus DEVFREQ Driver |
| `CONFIG_ARM_RK3399_DMC_DEVFREQ` | `rk3399_dmc.o` | ARM RK3399 DMC DEVFREQ Driver |
| `CONFIG_ARM_TEGRA_DEVFREQ` | `tegra-devfreq.o` | Tegra DEVFREQ Driver |
| `CONFIG_DEVFREQ_GOV_PASSIVE` | `governor_passive.o` | Passive |
| `CONFIG_DEVFREQ_GOV_PERFORMANCE` | `governor_performance.o` | Performance |
| `CONFIG_DEVFREQ_GOV_POWERSAVE` | `governor_powersave.o` | Powersave |
| `CONFIG_DEVFREQ_GOV_SIMPLE_ONDEMAND` | `governor_simpleondemand.o` | Simple Ondemand |
| `CONFIG_DEVFREQ_GOV_USERSPACE` | `governor_userspace.o` | Userspace |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-ipi.o` |  |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-v2/helio-dvfsrc.o` |  |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-v2/helio-dvfsrc-mt6765.o` |  |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-v2/helio-dvfsrc-opp.o` |  |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-v2/helio-dvfsrc-opp-mt6765.o` |  |
| `CONFIG_MACH_MT6765` | `helio-dvfsrc-v2/helio-dvfsrc-sysfs.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-ipi.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-v2/helio-dvfsrc.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-v2/helio-dvfsrc-mt6768.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-v2/helio-dvfsrc-opp.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-v2/helio-dvfsrc-opp-mt6768.o` |  |
| `CONFIG_MACH_MT6768` | `helio-dvfsrc-v2/helio-dvfsrc-sysfs.o` |  |
| `CONFIG_MACH_MT6771` | `helio-dvfsrc.o` |  |
| `CONFIG_MACH_MT6771` | `helio-dvfsrc-mt6771.o` |  |
| `CONFIG_MACH_MT6771` | `helio-dvfsrc-opp-mt6771.o` |  |
| `CONFIG_MACH_MT6771` | `helio-dvfsrc-sysfs.o` |  |
| `CONFIG_PM_DEVFREQ` | `devfreq.o` |  |
| `CONFIG_PM_DEVFREQ_EVENT` | `devfreq-event.o` |  |

### `drivers/devfreq/event`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_DEVFREQ_EVENT_EXYNOS_NOCP` | `exynos-nocp.o` | EXYNOS NoC (Network On Chip) Probe DEVFREQ event Driver |
| `CONFIG_DEVFREQ_EVENT_EXYNOS_PPMU` | `exynos-ppmu.o` | EXYNOS PPMU (Platform Performance Monitoring Unit) DEVFREQ e |
| `CONFIG_DEVFREQ_EVENT_ROCKCHIP_DFI` | `rockchip-dfi.o` | ROCKCHIP DFI DEVFREQ event Driver |

### `drivers/devfreq/helio-dvfsrc-v3`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6785` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6785` | `helio-dvfsrc-mt6785.o` |  |
| `CONFIG_MACH_MT6785` | `helio-dvfsrc-opp-mt6785.o` |  |
| `CONFIG_MACH_MT6833` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6833` | `helio-dvfsrc-mt6833.o` |  |
| `CONFIG_MACH_MT6833` | `helio-dvfsrc-opp-mt6833.o` |  |
| `CONFIG_MACH_MT6833` | `helio-dvfsrc-smc-control.o` |  |
| `CONFIG_MACH_MT6853` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6853` | `helio-dvfsrc-mt6853.o` |  |
| `CONFIG_MACH_MT6853` | `helio-dvfsrc-opp-mt6853.o` |  |
| `CONFIG_MACH_MT6853` | `helio-dvfsrc-smc-control.o` |  |
| `CONFIG_MACH_MT6873` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6873` | `helio-dvfsrc-mt6873.o` |  |
| `CONFIG_MACH_MT6873` | `helio-dvfsrc-opp-mt6873.o` |  |
| `CONFIG_MACH_MT6873` | `helio-dvfsrc-smc-control.o` |  |
| `CONFIG_MACH_MT6885` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6885` | `helio-dvfsrc-mt6885.o` |  |
| `CONFIG_MACH_MT6885` | `helio-dvfsrc-opp-mt6885.o` |  |
| `CONFIG_MACH_MT6885` | `helio-dvfsrc-smc-control.o` |  |
| `CONFIG_MACH_MT6893` | `helio-dvfsrc-ip-v2.o` |  |
| `CONFIG_MACH_MT6893` | `helio-dvfsrc-mt6893.o` |  |
| `CONFIG_MACH_MT6893` | `helio-dvfsrc-opp-mt6893.o` |  |
| `CONFIG_MACH_MT6893` | `helio-dvfsrc-smc-control.o` |  |
| `CONFIG_MTK_DVFSRC_MT6833_PRETEST` | `helio-dvfsrc-ip-v2.o` | MTK_DVFSRC_MT6833_PRETEST |
| `CONFIG_MTK_DVFSRC_MT6833_PRETEST` | `helio-dvfsrc-mt6833.o` | MTK_DVFSRC_MT6833_PRETEST |
| `CONFIG_MTK_DVFSRC_MT6833_PRETEST` | `helio-dvfsrc-opp-mt6833.o` | MTK_DVFSRC_MT6833_PRETEST |
| `CONFIG_MTK_DVFSRC_MT6833_PRETEST` | `helio-dvfsrc-smc-control.o` | MTK_DVFSRC_MT6833_PRETEST |
| `obj-y` | `helio-dvfsrc-opp.o` |  |
| `obj-y` | `helio-dvfsrc-qos.o` |  |
| `obj-y` | `helio-dvfsrc-sysfs.o` |  |

### `drivers/devfreq/helio-dvfsrc-v4`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT8168` | `helio-dvfsrc.o` |  |
| `CONFIG_MACH_MT8168` | `helio-dvfsrc-mt8168.o` |  |
| `CONFIG_MACH_MT8168` | `helio-dvfsrc-opp.o` |  |
| `CONFIG_MACH_MT8168` | `helio-dvfsrc-sysfs.o` |  |

### `drivers/iommu`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_AMD_IOMMU` | `amd_iommu.o` | AMD IOMMU support |
| `CONFIG_AMD_IOMMU` | `amd_iommu_init.o` | AMD IOMMU support |
| `CONFIG_AMD_IOMMU_V2` | `amd_iommu_v2.o` | AMD IOMMU Version 2 driver |
| `CONFIG_ARM_SMMU` | `arm-smmu.o` | ARM Ltd. System MMU (SMMU) Support |
| `CONFIG_ARM_SMMU_V3` | `arm-smmu-v3.o` | ARM Ltd. System MMU Version 3 (SMMUv3) Support |
| `CONFIG_DMAR_TABLE` | `dmar.o` |  |
| `CONFIG_EXYNOS_IOMMU` | `exynos-iommu.o` | Exynos IOMMU Support |
| `CONFIG_FSL_PAMU` | `fsl_pamu.o` | Freescale IOMMU support |
| `CONFIG_FSL_PAMU` | `fsl_pamu_domain.o` | Freescale IOMMU support |
| `CONFIG_INTEL_IOMMU` | `intel-iommu.o` | Support for Intel IOMMU using DMA Remapping Devices |
| `CONFIG_INTEL_IOMMU_SVM` | `intel-svm.o` | Support for Shared Virtual Memory with Intel IOMMU |
| `CONFIG_IOMMU_API` | `iommu.o` |  |
| `CONFIG_IOMMU_API` | `iommu-sysfs.o` |  |
| `CONFIG_IOMMU_API` | `iommu-traces.o` |  |
| `CONFIG_IOMMU_DMA` | `dma-iommu.o` |  |
| `CONFIG_IOMMU_IOVA` | `iova.o` |  |
| `CONFIG_IOMMU_IO_PGTABLE` | `io-pgtable.o` |  |
| `CONFIG_IOMMU_IO_PGTABLE_ARMV7S` | `io-pgtable-arm-v7s.o` | ARMv7/v8 Short Descriptor Format |
| `CONFIG_IOMMU_IO_PGTABLE_LPAE` | `io-pgtable-arm.o` | ARMv7/v8 Long Descriptor Format |
| `CONFIG_IPMMU_VMSA` | `ipmmu-vmsa.o` | Renesas VMSA-compatible IPMMU |
| `CONFIG_IRQ_REMAP` | `intel_irq_remapping.o` | Support for Interrupt Remapping |
| `CONFIG_IRQ_REMAP` | `irq_remapping.o` | Support for Interrupt Remapping |
| `CONFIG_MSM_IOMMU` | `msm_iommu.o` | MSM IOMMU Support |
| `CONFIG_MTK_IOMMU` | `mtk_iommu.o` | MTK IOMMU Support |
| `CONFIG_MTK_IOMMU_V1` | `mtk_iommu_v1.o` | MTK IOMMU Version 1 (M4U gen1) Support |
| `CONFIG_MTK_IOMMU_V2` | `mtk_iommu_ext.o` | MTK IOMMU Version 2 |
| `CONFIG_MTK_IOMMU_V2` | `mtk_iommu_v2.o` | MTK IOMMU Version 2 |
| `CONFIG_OF_IOMMU` | `of_iommu.o` |  |
| `CONFIG_OMAP_IOMMU` | `omap-iommu.o` | OMAP IOMMU Support |
| `CONFIG_OMAP_IOMMU_DEBUG` | `omap-iommu-debug.o` | Export OMAP IOMMU internals in DebugFS |
| `CONFIG_QCOM_IOMMU` | `qcom_iommu.o` | Qualcomm IOMMU Support |
| `CONFIG_ROCKCHIP_IOMMU` | `rockchip-iommu.o` | Rockchip IOMMU Support |
| `CONFIG_S390_IOMMU` | `s390-iommu.o` |  |
| `CONFIG_TEGRA_IOMMU_GART` | `tegra-gart.o` | Tegra GART IOMMU Support |
| `CONFIG_TEGRA_IOMMU_SMMU` | `tegra-smmu.o` | NVIDIA Tegra SMMU Support |

### `drivers/misc/mediatek/base/power/brisket_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_brisket.o` |  |

### `drivers/misc/mediatek/base/power/clkbuf_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_clkbuf_ctl.o` |  |

### `drivers/misc/mediatek/base/power/clkbuf_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_clkbuf_hw.o` |  |

### `drivers/misc/mediatek/base/power/cm_mgr_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cm_mgr.o` |  |
| `obj-y` | `mtk_cm_mgr_mt8168.o` |  |

### `drivers/misc/mediatek/base/power/cm_mgr_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cm_mgr_platform.o` |  |

### `drivers/misc/mediatek/base/power/cm_mgr_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cm_mgr_platform.o` |  |

### `drivers/misc/mediatek/base/power/cpufreq_v1/src`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cpufreq_api.o` |  |
| `obj-y` | `mtk_cpufreq_debug.o` |  |
| `obj-y` | `mtk_cpufreq_hybrid.o` |  |
| `obj-y` | `mtk_cpufreq_interface.o` |  |

### `drivers/misc/mediatek/base/power/cpufreq_v1/src/mach/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cpufreq_platform.o` |  |

### `drivers/misc/mediatek/base/power/cpufreq_v1/src/mach/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cpufreq_platform.o` |  |

### `drivers/misc/mediatek/base/power/cpufreq_v2/src`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cpufreq_api.o` |  |
| `obj-y` | `mtk_cpufreq_debug.o` |  |
| `obj-y` | `mtk_cpufreq_hybrid.o` |  |
| `obj-y` | `mtk_cpufreq_interface.o` |  |

### `drivers/misc/mediatek/base/power/cpuhotplug`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6771` | `mtk_cpuhp_mt6771.o` |  |
| `CONFIG_MACH_MT6775` | `mtk_cpuhp_mt6771.o` |  |
| `obj-y` | `mtk_cpuhp_core.o` |  |
| `obj-y` | `mtk_cpuhp_platform.o` |  |
| `obj-y` | `mtk_cpuhp_ppm.o` |  |

### `drivers/misc/mediatek/base/power/cpuidle_v3`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cpuidle.o` |  |

### `drivers/misc/mediatek/base/power/cpumssv_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mssv.o` |  |

### `drivers/misc/mediatek/base/power/cpumssv_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mssv.o` |  |

### `drivers/misc/mediatek/base/power/credit_didt_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_credit_didt.o` |  |

### `drivers/misc/mediatek/base/power/dcm_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6739` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6885` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_dcm.o` |  |
| `CONFIG_MACH_MT8168` | `mtk_dcm.o` |  |

### `drivers/misc/mediatek/base/power/dcm_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_dcm_autogen.o` |  |
| `obj-y` | `mtk_dcm_internal.o` |  |

### `drivers/misc/mediatek/base/power/dcm_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_dcm_autogen.o` |  |
| `obj-y` | `mtk_dcm_internal.o` |  |

### `drivers/misc/mediatek/base/power/dvfsrc`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `dvfsrc-boost-driver.o` |  |

### `drivers/misc/mediatek/base/power/eem_v2/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_eem.o` |  |
| `obj-y` | `mtk_eem_api.o` |  |
| `obj-y` | `mtk_eem_internal.o` |  |
| `obj-y` | `mtk_eem_platform.o` |  |
| `obj-y` | `mtk_picachu.o` |  |

### `drivers/misc/mediatek/base/power/eem_v2/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_eem.o` |  |
| `obj-y` | `mtk_eem_api.o` |  |
| `obj-y` | `mtk_eem_internal.o` |  |
| `obj-y` | `mtk_eem_platform.o` |  |

### `drivers/misc/mediatek/base/power/eemgpu_v2/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_eemgpu.o` |  |
| `obj-y` | `mtk_eemgpu_internal.o` |  |
| `obj-y` | `mtk_eemgpu_platform.o` |  |

### `drivers/misc/mediatek/base/power/eemgpu_v2/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_eemgpu.o` |  |
| `obj-y` | `mtk_eemgpu_internal.o` |  |
| `obj-y` | `mtk_eemgpu_platform.o` |  |

### `drivers/misc/mediatek/base/power/gpufreq_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_gpufreq_common.o` |  |

### `drivers/misc/mediatek/base/power/gpufreq_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_gpufreq_plat.o` |  |

### `drivers/misc/mediatek/base/power/gpufreq_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_gpufreq_plat.o` |  |

### `drivers/misc/mediatek/base/power/hps_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mt_hotplug_strategy_algo.o` |  |
| `obj-y` | `mt_hotplug_strategy_api.o` |  |
| `obj-y` | `mt_hotplug_strategy_core.o` |  |
| `obj-y` | `mt_hotplug_strategy_cpu.o` |  |
| `obj-y` | `mt_hotplug_strategy_main.o` |  |
| `obj-y` | `mt_hotplug_strategy_procfs.o` |  |

### `drivers/misc/mediatek/base/power/koro/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_koro.o` |  |

### `drivers/misc/mediatek/base/power/koro/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_koro.o` |  |

### `drivers/misc/mediatek/base/power/leakage_table_v2`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6739` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6739` | `mtk_static_power_mt6739.o` |  |
| `CONFIG_MACH_MT6758` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6758` | `mtk_static_power_mt6758.o` |  |
| `CONFIG_MACH_MT6759` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6759` | `mtk_static_power_mt6759.o` |  |
| `CONFIG_MACH_MT6763` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6763` | `mtk_static_power_mt6763.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_static_power_mt6765.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_static_power_mt6768.o` |  |
| `CONFIG_MACH_MT6771` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6771` | `mtk_static_power_mt6771.o` |  |
| `CONFIG_MACH_MT6775` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6775` | `mtk_static_power_mt6775.o` |  |
| `CONFIG_MACH_MT6785` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6785` | `mtk_static_power_mt6785.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_static_power_mt6833.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_static_power_mt6853.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_static_power_mt6873.o` |  |
| `CONFIG_MACH_MT6885` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_static_power_mt6893.o` |  |
| `CONFIG_MACH_MT8168` | `mtk_static_power.o` |  |
| `CONFIG_MACH_MT8168` | `mtk_static_power_mt8168.o` |  |
| `obj-y` | `mtk_static_power_mt6885.o` |  |
| `obj-y` | `mtk_static_power_mt6893.o` |  |

### `drivers/misc/mediatek/base/power/mcdi`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mcdi_api.o` |  |
| `obj-y` | `mtk_mcdi_governor.o` |  |
| `obj-y` | `mtk_mcdi_governor_hint.o` |  |
| `obj-y` | `mtk_mcdi_governor_lib.o` |  |

### `drivers/misc/mediatek/base/power/mcdi/mcdi_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mcdi_cpc.o` |  |
| `obj-y` | `mtk_mcdi_main.o` |  |
| `obj-y` | `mtk_mcdi_mcupm.o` |  |
| `obj-y` | `mtk_mcdi_profile.o` |  |
| `obj-y` | `mtk_mcdi_util.o` |  |

### `drivers/misc/mediatek/base/power/mcdi/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mcdi_plat.o` |  |
| `obj-y` | `mtk_mcdi_state.o` |  |

### `drivers/misc/mediatek/base/power/mcdi/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mcdi_plat.o` |  |
| `obj-y` | `mtk_mcdi_state.o` |  |

### `drivers/misc/mediatek/base/power/mdpm_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mdpm_common.o` |  |

### `drivers/misc/mediatek/base/power/mdpm_v2`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mdpm_common.o` |  |

### `drivers/misc/mediatek/base/power/mdpm_v2/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mdpm_platform.o` |  |

### `drivers/misc/mediatek/base/power/mdpm_v2/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_mdpm_platform.o` |  |

### `drivers/misc/mediatek/base/power/pbm_v3`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_pbm.o` |  |

### `drivers/misc/mediatek/base/power/pbm_v4`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_pbm.o` |  |

### `drivers/misc/mediatek/base/power/power_gs_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6739` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6739` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6771` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6771` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6785` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6785` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6885` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6885` | `mtk_power_gs_api.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_power_gs.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_power_gs_api.o` |  |

### `drivers/misc/mediatek/base/power/power_gs_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cg_array.o` |  |
| `obj-y` | `mtk_dcm_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_1_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_2_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_3_array.o` |  |
| `obj-y` | `mtk_power_gs_6359_array.o` |  |
| `obj-y` | `mtk_power_gs_compare.o` |  |
| `obj-y` | `mtk_power_gs_dump_pmic.o` |  |
| `obj-y` | `mtk_power_gs_sp.o` |  |

### `drivers/misc/mediatek/base/power/power_gs_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_cg_array.o` |  |
| `obj-y` | `mtk_dcm_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_1_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_2_array.o` |  |
| `obj-y` | `mtk_power_gs_6315_3_array.o` |  |
| `obj-y` | `mtk_power_gs_6359_array.o` |  |
| `obj-y` | `mtk_power_gs_compare.o` |  |
| `obj-y` | `mtk_power_gs_dump_pmic.o` |  |
| `obj-y` | `mtk_power_gs_sp.o` |  |

### `drivers/misc/mediatek/base/power/power_throttling`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_md_power_throttling.o` |  |

### `drivers/misc/mediatek/base/power/ppm_v3/src`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_ppm_api.o` |  |
| `obj-y` | `mtk_ppm_cpi.o` |  |
| `obj-y` | `mtk_ppm_interface.o` |  |
| `obj-y` | `mtk_ppm_ipi.o` |  |
| `obj-y` | `mtk_ppm_policy_dlpt.o` |  |
| `obj-y` | `mtk_ppm_policy_force_limit.o` |  |
| `obj-y` | `mtk_ppm_policy_hard_user_limit.o` |  |
| `obj-y` | `mtk_ppm_policy_lcm_off.o` |  |
| `obj-y` | `mtk_ppm_policy_ptpod.o` |  |
| `obj-y` | `mtk_ppm_policy_pwr_thro.o` |  |
| `obj-y` | `mtk_ppm_policy_sys_boost.o` |  |
| `obj-y` | `mtk_ppm_policy_thermal.o` |  |
| `obj-y` | `mtk_ppm_policy_user_limit.o` |  |
| `obj-y` | `mtk_ppm_policy_ut.o` |  |
| `obj-y` | `mtk_ppm_profile.o` |  |

### `drivers/misc/mediatek/base/power/ppm_v3/src/mach/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_ppm_cobra_algo.o` |  |
| `obj-y` | `mtk_ppm_platform.o` |  |
| `obj-y` | `mtk_ppm_power_data.o` |  |

### `drivers/misc/mediatek/base/power/ppm_v3/src/mach/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_ppm_cobra_algo.o` |  |
| `obj-y` | `mtk_ppm_platform.o` |  |
| `obj-y` | `mtk_ppm_power_data.o` |  |

### `drivers/misc/mediatek/base/power/ptp3_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_ptp3_adcc.o` |  |
| `obj-y` | `mtk_ptp3_brisket2.o` |  |
| `obj-y` | `mtk_ptp3_cinst.o` |  |
| `obj-y` | `mtk_ptp3_ctt.o` |  |
| `obj-y` | `mtk_ptp3_drcc.o` |  |
| `obj-y` | `mtk_ptp3_dt.o` |  |
| `obj-y` | `mtk_ptp3_fll.o` |  |
| `obj-y` | `mtk_ptp3_iglre.o` |  |
| `obj-y` | `mtk_ptp3_main.o` |  |
| `obj-y` | `mtk_ptp3_pdp.o` |  |

### `drivers/misc/mediatek/base/power/qos`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_qos_bound.o` |  |
| `obj-y` | `mtk_qos_ipi.o` |  |
| `obj-y` | `mtk_qos_ipi_v2.o` |  |
| `obj-y` | `mtk_qos_main.o` |  |
| `obj-y` | `mtk_qos_sram.o` |  |
| `obj-y` | `mtk_qos_sysfs.o` |  |

### `drivers/misc/mediatek/base/power/ses_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_ses.o` |  |

### `drivers/misc/mediatek/base/power/spm/common`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_idle.o` |  |
| `obj-y` | `mtk_idle_export_legacy.o` |  |
| `obj-y` | `mtk_idle_internal.o` |  |
| `obj-y` | `mtk_idle_module.o` |  |
| `obj-y` | `mtk_idle_profile.o` |  |
| `obj-y` | `mtk_idle_select.o` |  |
| `obj-y` | `mtk_idle_twam.o` |  |
| `obj-y` | `mtk_sleep.o` |  |
| `obj-y` | `mtk_sleep_init.o` |  |
| `obj-y` | `mtk_spm_resource_req.o` |  |
| `obj-y` | `mtk_vcore_dvfs.o` |  |

### `drivers/misc/mediatek/base/power/spm/common/mtk_idle_fs`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_idle_sysfs.o` |  |
| `obj-y` | `mtk_lp_kernfs.o` |  |
| `obj-y` | `mtk_lp_sysfs.o` |  |
| `obj-y` | `mtk_lp_sysfs_procfs.o` |  |

### `drivers/misc/mediatek/base/power/spm_v4`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_idle.o` |  |
| `obj-y` | `mtk_idle.o` |  |
| `obj-y` | `mtk_idle_mt6739.o` |  |
| `obj-y` | `mtk_idle_mt6763.o` |  |
| `obj-y` | `mtk_idle_mt6771.o` |  |
| `obj-y` | `mtk_idle_profile.o` |  |
| `obj-y` | `mtk_idle_profile.o` |  |
| `obj-y` | `mtk_idle_sysfs.o` |  |
| `obj-y` | `mtk_idle_sysfs.o` |  |
| `obj-y` | `mtk_lp_kernfs.o` |  |
| `obj-y` | `mtk_lp_kernfs.o` |  |
| `obj-y` | `mtk_lp_sysfs.o` |  |
| `obj-y` | `mtk_lp_sysfs.o` |  |
| `obj-y` | `mtk_lp_sysfs_procfs.o` |  |
| `obj-y` | `mtk_lp_sysfs_procfs.o` |  |
| `obj-y` | `mtk_sleep.o` |  |
| `obj-y` | `mtk_sleep.o` |  |
| `obj-y` | `mtk_spm.o` |  |
| `obj-y` | `mtk_spm.o` |  |
| `obj-y` | `mtk_spm_dpidle.o` |  |
| `obj-y` | `mtk_spm_dpidle.o` |  |
| `obj-y` | `mtk_spm_dpidle_mt6739.o` |  |
| `obj-y` | `mtk_spm_dpidle_mt6763.o` |  |
| `obj-y` | `mtk_spm_dpidle_mt6771.o` |  |
| `obj-y` | `mtk_spm_fs.o` |  |
| `obj-y` | `mtk_spm_fs.o` |  |
| `obj-y` | `mtk_spm_init.o` |  |
| `obj-y` | `mtk_spm_init.o` |  |
| `obj-y` | `mtk_spm_internal.o` |  |
| `obj-y` | `mtk_spm_internal.o` |  |
| `obj-y` | `mtk_spm_pmic_wrap.o` |  |
| `obj-y` | `mtk_spm_resource_req.o` |  |
| `obj-y` | `mtk_spm_resource_req.o` |  |
| `obj-y` | `mtk_spm_sleep.o` |  |
| `obj-y` | `mtk_spm_sleep.o` |  |
| `obj-y` | `mtk_spm_sleep_mt6739.o` |  |
| `obj-y` | `mtk_spm_sleep_mt6763.o` |  |
| `obj-y` | `mtk_spm_sleep_mt6771.o` |  |
| `obj-y` | `mtk_spm_sodi.o` |  |
| `obj-y` | `mtk_spm_sodi.o` |  |
| `obj-y` | `mtk_spm_sodi3p0.o` |  |
| `obj-y` | `mtk_spm_sodi3p0.o` |  |
| `obj-y` | `mtk_spm_sodi_mt6739.o` |  |
| `obj-y` | `mtk_spm_sodi_mt6763.o` |  |
| `obj-y` | `mtk_spm_sodi_mt6771.o` |  |
| `obj-y` | `mtk_spm_vcorefs.o` |  |
| `obj-y` | `mtk_spm_vcorefs_ipi_mt6771.o` |  |
| `obj-y` | `mtk_spm_vcorefs_mt6771.o` |  |

### `drivers/misc/mediatek/base/power/srclken_rc_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_srclken_rc.o` |  |

### `drivers/misc/mediatek/base/power/srclken_rc_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_srclken_rc_hw.o` |  |

### `drivers/misc/mediatek/base/power/swpm_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_swpm.o` |  |
| `obj-y` | `mtk_swpm_common.o` |  |

### `drivers/misc/mediatek/base/power/swpm_v1/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_swpm_platform.o` |  |
| `obj-y` | `mtk_swpm_sp_platform.o` |  |

### `drivers/misc/mediatek/base/power/swpm_v1/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_swpm_platform.o` |  |
| `obj-y` | `mtk_swpm_sp_platform.o` |  |

### `drivers/misc/mediatek/base/power/udi_v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_udi.o` |  |

### `drivers/misc/mediatek/base/power/udi_v2`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_udi.o` |  |

### `drivers/misc/mediatek/base/power/upower_v2`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MACH_MT6739` | `mtk_unified_power_mt6739.o` |  |
| `CONFIG_MACH_MT6758` | `mtk_unified_power_mt6758.o` |  |
| `CONFIG_MACH_MT6759` | `mtk_unified_power_mt6759.o` |  |
| `CONFIG_MACH_MT6763` | `mtk_unified_power_mt6763.o` |  |
| `CONFIG_MACH_MT6765` | `mtk_unified_power_mt6765.o` |  |
| `CONFIG_MACH_MT6768` | `mtk_unified_power_mt6768.o` |  |
| `CONFIG_MACH_MT6771` | `mtk_unified_power_mt6771.o` |  |
| `CONFIG_MACH_MT6775` | `mtk_unified_power_mt6775.o` |  |
| `CONFIG_MACH_MT6785` | `mtk_unified_power_mt6785.o` |  |
| `CONFIG_MACH_MT6833` | `mtk_unified_power_mt6833.o` |  |
| `CONFIG_MACH_MT6853` | `mtk_unified_power_mt6853.o` |  |
| `CONFIG_MACH_MT6873` | `mtk_unified_power_mt6873.o` |  |
| `CONFIG_MACH_MT6893` | `mtk_unified_power_mt6893.o` |  |
| `CONFIG_MACH_MT8168` | `mtk_unified_power_mt8168.o` |  |
| `obj-y` | `mtk_unified_power.o` |  |
| `obj-y` | `mtk_unified_power_api.o` |  |
| `obj-y` | `mtk_unified_power_internal.o` |  |
| `obj-y` | `mtk_unified_power_mt6885.o` |  |
| `obj-y` | `mtk_unified_power_mt6893.o` |  |

### `drivers/misc/mediatek/base/power/vcorefs_v3`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_vcorefs_governor.o` |  |
| `obj-y` | `mtk_vcorefs_manager.o` |  |

### `drivers/misc/mediatek/connectivity`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `$(MODULE_NAME).o` |  |

### `drivers/misc/mediatek/debug_latch/cache_parity`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `cache_parity.o` |  |

### `drivers/misc/mediatek/debug_latch/l2c_parity`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `l2c_parity.o` |  |

### `drivers/misc/mediatek/debug_latch/lastbus`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `lastbus.o` |  |

### `drivers/misc/mediatek/debug_latch/plat_dbg_info`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `plat_dbg_info.o` |  |
| `obj-y` | `plat_sram_flag.o` |  |

### `drivers/misc/mediatek/devmpu`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_DEVMPU` | `devmpu.o` | Enable MTK Device MPU driver |
| `CONFIG_MTK_DEVMPU_EMI` | `devmpu_emi.o` | Enable MTK Device MPU with EMI MPU |

### `drivers/misc/mediatek/eccci/mixdev`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mix_event_lib.o` |  |

### `drivers/misc/mediatek/eccci/udc`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_ECCCI_DRIVER` | `udc.o` | Enable Evolved CCCI Driver as interface for modem |

### `drivers/misc/mediatek/emi/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `../emi_ctrl_v1.o` |  |
| `obj-y` | `../submodule/bwl_v1.o` |  |
| `obj-y` | `../submodule/elm_v2.o` |  |
| `obj-y` | `../submodule/mpu_v1.o` |  |
| `obj-y` | `../submodule/pasr_api_v1.o` |  |
| `obj-y` | `bwl_platform.o` |  |
| `obj-y` | `mpu_platform.o` |  |
| `obj-y` | `plat_debug_api.o` |  |

### `drivers/misc/mediatek/m4u/2.0`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `m4u.o` |  |
| `obj-y` | `m4u_debug.o` |  |
| `obj-y` | `m4u_mva.o` |  |
| `obj-y` | `m4u_pgtable.o` |  |
| `obj-y` | `m4u_sec_gp.o` |  |
| `obj-y` | `m4u_sec_gp.o` |  |

### `drivers/misc/mediatek/m4u/2.4`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `m4u.o` |  |
| `obj-y` | `m4u_debug.o` |  |
| `obj-y` | `m4u_mva.o` |  |
| `obj-y` | `m4u_pgtable.o` |  |
| `obj-y` | `m4u_sec_gp.o` |  |
| `obj-y` | `m4u_vpu_ut.o` |  |

### `drivers/misc/mediatek/m4u/3.2`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `m4u.o` |  |
| `obj-y` | `m4u_debug.o` |  |
| `obj-y` | `m4u_mva.o` |  |
| `obj-y` | `m4u_pgtable.o` |  |
| `obj-y` | `m4u_sec_gp.o` |  |
| `obj-y` | `m4u_sec_gz.o` |  |

### `drivers/misc/mediatek/m4u/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_M4U` | `m4u_hw.o` | MediaTek M4U driver |

### `drivers/misc/mediatek/m4u/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_M4U` | `m4u_hw.o` | MediaTek M4U driver |

### `drivers/misc/mediatek/mddp`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_MDDP_SUPPORT` | `mddp.o` | Support Mediatek MD Direct Path |

### `drivers/misc/mediatek/performance`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `perfmgr_main.o` |  |

### `drivers/misc/mediatek/performance/base`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `utility.o` |  |

### `drivers/misc/mediatek/performance/boost_ctrl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `boostctrl_main.o` |  |

### `drivers/misc/mediatek/performance/boost_ctrl/cpu_ctrl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_CPU_CTRL_CFP` | `cpu_ctrl_cfp.o` | CPU CTRL Ceiling-Fool-Proof |
| `obj-y` | `cpu_ctrl.o` |  |
| `obj-y` | `cpu_ctrl.o` |  |
| `obj-y` | `cpu_ctrl_dummy.o` |  |
| `obj-y` | `cpu_ctrl_dummy.o` |  |

### `drivers/misc/mediatek/performance/boost_ctrl/dram_ctrl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `dram_ctrl.o` |  |

### `drivers/misc/mediatek/performance/boost_ctrl/eas_ctrl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `eas_ctrl.o` |  |

### `drivers/misc/mediatek/performance/boost_ctrl/topo_ctrl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `topo_ctrl.o` |  |

### `drivers/misc/mediatek/performance/eara_thermal`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `platform/$(MTK_PLATFORM)/thermal_budget_platform.o` |  |
| `obj-y` | `src/thermal_base.o` |  |
| `obj-y` | `src/thermal_budget.o` |  |
| `obj-y` | `src/thermal_prethrottle.o` |  |

### `drivers/misc/mediatek/performance/fpsgo_v3`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `fpsgo_main.o` |  |

### `drivers/misc/mediatek/performance/fpsgo_v3/composer`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `src/fps_composer.o` |  |

### `drivers/misc/mediatek/performance/fpsgo_v3/eara_job`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `src/eara_job.o` |  |

### `drivers/misc/mediatek/performance/fpsgo_v3/fbt`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `platform/$(MTK_PLATFORM)/fbt_cpu_platform.o` |  |
| `obj-y` | `platform/mt6893/fbt_cpu_platform.o` |  |

### `drivers/misc/mediatek/performance/fpsgo_v3/fstb`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `fstb.o` |  |

### `drivers/misc/mediatek/performance/gbe`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `gbe1_main.o` |  |
| `obj-y` | `gbe2_main.o` |  |
| `obj-y` | `gbe_common.o` |  |
| `obj-y` | `gbe_sysfs.o` |  |

### `drivers/misc/mediatek/performance/load_track`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `load_track.o` |  |

### `drivers/misc/mediatek/performance/observer`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `eara_thrm_notify.o` |  |
| `obj-y` | `fpsgo_notify.o` |  |
| `obj-y` | `nn_notify.o` |  |
| `obj-y` | `pob_main.o` |  |
| `obj-y` | `qos_notify.o` |  |
| `obj-y` | `xpu_notify.o` |  |

### `drivers/misc/mediatek/performance/observer/platform/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `qos_pfm.o` |  |

### `drivers/misc/mediatek/performance/observer/platform/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `qos_pfm.o` |  |

### `drivers/misc/mediatek/performance/perf_ioctl`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `perf_ioctl.o` |  |

### `drivers/misc/mediatek/performance/resym`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `rs_base.o` |  |
| `obj-y` | `rs_main.o` |  |
| `obj-y` | `rs_state.o` |  |
| `obj-y` | `rs_trace.o` |  |
| `obj-y` | `rs_usage.o` |  |

### `drivers/misc/mediatek/performance/resym/platform/mt6885`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `rs_mdla_pfm.o` |  |
| `obj-y` | `rs_vpu_pfm.o` |  |

### `drivers/misc/mediatek/performance/resym/platform/mt6893`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `rs_mdla_pfm.o` |  |
| `obj-y` | `rs_vpu_pfm.o` |  |

### `drivers/misc/mediatek/performance/syslimiter`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `syslimiter.o` |  |

### `drivers/misc/mediatek/performance/tchbst`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `tchbst_main.o` |  |

### `drivers/misc/mediatek/performance/tchbst/kernel`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `ktch.o` |  |

### `drivers/misc/mediatek/performance/tchbst/user`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `utch.o` |  |

### `drivers/misc/mediatek/performance/uload_ind`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `uload_ind.o` |  |

### `drivers/misc/mediatek/pmic`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `obj-y` | `mtk_battery_adc_intf.o` |  |

### `drivers/misc/mediatek/pmic/common`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_PMIC_COMMON` | `upmu.o` | MediaTek Power Management Common driver |
| `CONFIG_MTK_PMIC_COMMON` | `upmu_debugfs.o` | MediaTek Power Management Common driver |
| `CONFIG_MTK_PMIC_COMMON` | `upmu_initial_setting.o` | MediaTek Power Management Common driver |
| `CONFIG_MTK_PMIC_COMMON` | `upmu_regulator.o` | MediaTek Power Management Common driver |
| `obj-y` | `upmu_lbat_service.o` |  |
| `obj-y` | `upmu_lbat_service_v2.o` |  |
| `obj-y` | `upmu_lbat_service_v2.o` |  |

### `drivers/misc/mediatek/pmic/extbuck`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_EXTBUCK_MT6311` | `mt6311-i2c.o` | MediaTek I2C Ext Buck MT6311 |
| `CONFIG_EXTBUCK_MT6311` | `mt6311-regulator.o` | MediaTek I2C Ext Buck MT6311 |
| `CONFIG_EXTBUCK_MT6311` | `mt6311_api.o` | MediaTek I2C Ext Buck MT6311 |
| `CONFIG_MTK_EXTBUCK` | `extbuck.o` | EXT BUCK Driver |
| `CONFIG_REGULATOR_ISL91302A` | `isl91302a-regulator.o` | Intersil Spi Ext Buck Isl91302a |
| `CONFIG_REGULATOR_ISL91302A` | `isl91302a-spi.o` | Intersil Spi Ext Buck Isl91302a |
| `CONFIG_REGULATOR_ISL91302A` | `vproc2_ipi.o` | Intersil Spi Ext Buck Isl91302a |
| `CONFIG_REGULATOR_MT6691` | `mt6691-regulator.o` | MediaTek I2C Ext Buck MT6691 |
| `CONFIG_REGULATOR_RT5734` | `rt5734-regulator.o` | Richtek Spi Ext Buck rt5734 |
| `CONFIG_REGULATOR_RT5734` | `rt5734-spi.o` | Richtek Spi Ext Buck rt5734 |
| `CONFIG_REGULATOR_RT5738` | `rt5738-regulator.o` | Richtek Ext Buck RT5738 |

### `drivers/misc/mediatek/pmic/mt6357/v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_PMIC_CHIP_MT6357` | `pmic.o` | MediaTek PMIC MT6357 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6357` | `pmic_chr_type_det.o` | MediaTek PMIC MT6357 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6357` | `pmic_chr_type_det_v2.o` | MediaTek PMIC MT6357 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6357` | `pmic_common.o` | MediaTek PMIC MT6357 Driver |
| `obj-y` | `mt6357_charger.o` |  |
| `obj-y` | `mt6357_gauge.o` |  |
| `obj-y` | `pmic_ipi.o` |  |

### `drivers/misc/mediatek/pmic/mt6358/v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_PMIC_CHIP_MT6358` | `pmic.o` | MediaTek PMIC MT6358 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6358` | `pmic_common.o` | MediaTek PMIC MT6358 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6358` | `pmic_dummy_chr_type_det.o` | MediaTek PMIC MT6358 Driver |
| `obj-y` | `mt6358_gauge.o` |  |
| `obj-y` | `pmic_ipi.o` |  |

### `drivers/misc/mediatek/pmic/mt6359/v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_PMIC_CHIP_MT6359` | `pmic.o` | MediaTek PMIC MT6359 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6359` | `pmic_common.o` | MediaTek PMIC MT6359 Driver |
| `obj-y` | `mt6359_gauge.o` |  |
| `obj-y` | `pmic_ipi.o` |  |

### `drivers/misc/mediatek/pmic/mt6359p/v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_PMIC_CHIP_MT6359P` | `pmic.o` | MediaTek PMIC MT6359P Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6359P` | `pmic_common.o` | MediaTek PMIC MT6359P Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6359P` | `pmic_oc_debug.o` | MediaTek PMIC MT6359P Driver |
| `obj-y` | `mt6359_gauge.o` |  |
| `obj-y` | `pmic_ipi.o` |  |

### `drivers/misc/mediatek/pmic/mt6360/ldo`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MT6360_LDO` | `mt6360_ldo_i2c.o` | Support for MT6360 LDO part |
| `CONFIG_MT6360_LDO` | `mt6360_ldo_regmap.o` | Support for MT6360 LDO part |

### `drivers/misc/mediatek/pmic/mt6360/pmic`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MT6360_PMIC` | `mt6360_pmic_i2c.o` | Support for MT6360 PMIC part |
| `CONFIG_MT6360_PMIC` | `mt6360_pmic_regmap.o` | Support for MT6360 PMIC part |

### `drivers/misc/mediatek/pmic/mt6360/pmu`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MFD_MT6360_PMU` | `mt6360_pmu_i2c.o` | Support for MT6360 PMU part |
| `CONFIG_MT6360_PMU_CHARGER` | `mt6360_pmu_adc.o` | MT6360 PMU Charger Part |
| `CONFIG_MT6360_PMU_CHARGER` | `mt6360_pmu_chg.o` | MT6360 PMU Charger Part |
| `CONFIG_MT6360_PMU_FLED` | `mt6360_pmu_fled.o` | MT6360 PMU FLED Part |
| `CONFIG_MT6360_PMU_RGBLED` | `mt6360_pmu_rgbled.o` | MT6360 PMU RGBLED Part |
| `CONFIG_MT6360_PMU_RGBLED` | `mt_led_trigger.o` | MT6360 PMU RGBLED Part |

### `drivers/misc/mediatek/pmic/mt6370`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MFD_MT6370_PMU` | `mt6370_pmu_i2c.o` | Support for MT6370 PMU part |
| `CONFIG_MT6370_PMU_BLED` | `mt6370_pmu_bled.o` | MT6370 PMU BLED Part |
| `CONFIG_MT6370_PMU_CHARGER` | `mt6370_pmu_charger.o` | MT6370 PMU Charger Part |
| `CONFIG_MT6370_PMU_CHARGER` | `mt6370_pmu_charger_gm20.o` | MT6370 PMU Charger Part |
| `CONFIG_MT6370_PMU_DSV` | `mt6370_pmu_dsv.o` | MT6370 PMU DSV Part |
| `CONFIG_MT6370_PMU_DSV` | `mt6370_pmu_dsv_debugfs.o` | MT6370 PMU DSV Part |
| `CONFIG_MT6370_PMU_FLED` | `mt6370_pmu_fled.o` | MT6370 PMU FLED Part |
| `CONFIG_MT6370_PMU_LDO` | `mt6370_pmu_ldo.o` | MT6370 PMU LDO Part |
| `CONFIG_MT6370_PMU_RGBLED` | `mt6370_pmu_rgbled.o` | MT6370 PMU RGBLED Part |

### `drivers/misc/mediatek/pmic/mt6390/v1`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_MTK_CHARGER` | `mt6390_charger.o` |  |
| `CONFIG_MTK_CHARGER` | `pmic_chr_type_det_v2.o` |  |
| `CONFIG_MTK_PMIC_CHIP_MT6390` | `pmic.o` | MediaTek PMIC MT6390 Driver |
| `CONFIG_MTK_PMIC_CHIP_MT6390` | `pmic_common.o` | MediaTek PMIC MT6390 Driver |
| `obj-y` | `mt6390_gauge.o` |  |
| `obj-y` | `pmic_ipi.o` |  |

### `drivers/power/supply/mediatek/battery`

| Kconfig 开关 | 目标模块 | 说明 |
|---|---|---|
| `CONFIG_FUEL_GAUGE_BQ27Z561` | `bq27z561_fg.o` | TI bq27z561 battery gauge driver |
| `obj-y` | `mtk_battery.o` |  |
| `obj-y` | `mtk_battery_core.o` |  |
| `obj-y` | `mtk_battery_recovery.o` |  |
| `obj-y` | `mtk_gauge_class.o` |  |
| `obj-y` | `mtk_gauge_coulomb_service.o` |  |
| `obj-y` | `mtk_power_misc.o` |  |

## 2. 链接期必须提供的平台导出符号（头声明 ∩ 驱动调用点）

下表是 `adapted/` 里真实调用、且声明来自 ares-r-oss 平台头的函数。它们是全量链接时 `undefined symbol` 的主要来源，必须由目标树「已修好的平台基线层」实现并 `EXPORT_SYMBOL_GPL`。

| 符号 | 调用次数 | 声明所在头 | 原型 |
|---|---|---|---|
| `mtk_emimpu_set_apc` | 33 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_set_apc(struct emimpu_region_t *rg_info, unsigned int d_num, unsigned int apc);` |
| `mtk_emimpu_init_region` | 15 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_init_region( struct emimpu_region_t *rg_info, unsigned int rg_num);` |
| `mtk_emimpu_set_addr` | 15 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_set_addr(struct emimpu_region_t *rg_info, unsigned long long start, unsigned long long end);` |
| `mtk_emimpu_set_protection` | 15 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_set_protection(struct emimpu_region_t *rg_info);` |
| `mtk_emimpu_free_region` | 15 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_free_region(struct emimpu_region_t *rg_info);` |
| `get_cur_vcore_opp` | 13 | `drivers/devfreq/helio-dvfsrc-opp.h` | `extern int get_cur_vcore_opp(void);` |
| `mtk_emimpu_lock_region` | 9 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_lock_region(struct emimpu_region_t *rg_info, bool lock);` |
| `topo_ctrl_get_nr_clusters` | 7 | `drivers/misc/mediatek/performance/include/topo_ctrl.h` | `extern int topo_ctrl_get_nr_clusters(void);` |
| `buck_set_mode` | 7 | `drivers/misc/mediatek/pmic/include/pmic_regulator.h` | `extern int buck_set_mode(enum BUCK_TYPE type, unsigned char pmode);` |
| `smi_reg_restore_sec` | 5 | `drivers/iommu/mtk_iommu_ext.h` | `int smi_reg_restore_sec(void);` |
| `smi_reg_backup_sec` | 5 | `drivers/iommu/mtk_iommu_ext.h` | `int smi_reg_backup_sec(void);` |
| `spm_resource_req` | 2 | `drivers/misc/mediatek/base/power/include/spm_v4/mtk_spm_resource_req.h` | `bool spm_resource_req(unsigned int user, unsigned int req_mask);` |
| `mt_ppm_sysboost_freq` | 2 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern void mt_ppm_sysboost_freq(enum ppm_sysboost_user user, unsigned int freq);` |
| `mt_ppm_sysboost_core` | 2 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern void mt_ppm_sysboost_core(enum ppm_sysboost_user user, unsigned int core_num);` |
| `mt_ppm_sysboost_set_core_limit` | 2 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern void mt_ppm_sysboost_set_core_limit( enum ppm_sysboost_user user, unsigned int cluster, int min_core, int max_core);` |
| `mt_ppm_sysboost_set_freq_limit` | 2 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern void mt_ppm_sysboost_set_freq_limit( enum ppm_sysboost_user user, unsigned int cluster, int min_freq, int max_freq);` |
| `freqhopping_config` | 2 | `drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.h` | `int freqhopping_config(unsigned int pll_id, unsigned long def_set_idx, unsigned int enable);` |
| `lbat_user_register` | 2 | `drivers/misc/mediatek/pmic/include/pmic_lbat_service.h` | `extern int lbat_user_register(struct lbat_user *user, const char *name, unsigned int hv_thd_volt, unsigned int lv1_thd_volt, unsigned int lv2_thd_volt` |
| `aee_rr_rec_clk` | 2 | `drivers/clk/mediatek/clk-mt6885-pg.h` | `extern void aee_rr_rec_clk(int id, u32 val);` |
| `clk_buf_ctrl` | 1 | `drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h` | `bool clk_buf_ctrl(enum clk_buf_id id, bool onoff);` |
| `clk_buf_show_status_info` | 1 | `drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h` | `void clk_buf_show_status_info(void);` |
| `clk_buf_get_xo_en_sta` | 1 | `drivers/misc/mediatek/base/power/include/clkbuf_v1/mtk_clkbuf_ctl.h` | `u8 clk_buf_get_xo_en_sta(enum xo_id id);` |
| `pmic_ldo_vcn18_lp` | 1 | `drivers/misc/mediatek/pmic/include/mt6357/mtk_pmic_api_buck.h` | `extern int pmic_ldo_vcn18_lp(enum BUCK_LDO_EN_USER user, unsigned char op_en, unsigned char op_cfg);` |
| `pmic_ldo_vfe28_lp` | 1 | `drivers/misc/mediatek/pmic/include/mt6357/mtk_pmic_api_buck.h` | `extern int pmic_ldo_vfe28_lp(enum BUCK_LDO_EN_USER user, unsigned char op_en, unsigned char op_cfg);` |
| `register_pg_callback` | 1 | `drivers/clk/mediatek/clk-mt6885-pg.h` | `extern struct pg_callbacks *register_pg_callback(struct pg_callbacks *pgcb);` |
| `mtk_emimpu_prehandle_register` | 1 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_prehandle_register(irqreturn_t (*bypass_func) (unsigned int emi_id, struct reg_info_t *dump, unsigned int leng));` |
| `mtk_emimpu_postclear_register` | 1 | `include/memory/mediatek/emi.h` | `int mtk_emimpu_postclear_register(void (*clear_func) (unsigned int emi_id));` |
| `mt_usb_is_device` | 1 | `drivers/power/supply/mediatek/battery/mtk_battery_internal.h` | `extern bool mt_usb_is_device(void);` |
| `mt_ppm_userlimit_freq_limit_by_others` | 1 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern unsigned int mt_ppm_userlimit_freq_limit_by_others( unsigned int cluster);` |
| `ppm_game_mode_change_cb` | 1 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern void ppm_game_mode_change_cb(int is_game_mode);` |
| `mt_ppm_userlimit_cpu_freq` | 1 | `drivers/misc/mediatek/base/power/include/ppm_v3/mtk_ppm_api.h` | `extern unsigned int mt_ppm_userlimit_cpu_freq(unsigned int cluster_num, struct ppm_limit_data *data);` |
| `force_get_tbat` | 1 | `drivers/power/supply/mediatek/battery/mtk_battery_internal.h` | `extern int force_get_tbat(bool update);` |
| `vcore_pmic_set_mode` | 1 | `drivers/misc/mediatek/pmic/include/mt6357/mtk_pmic_api_buck.h` | `int vcore_pmic_set_mode(unsigned char mode);` |

## 3. 接线建议

1. 把 `baseline/build-files/` 里的 Makefile/Kconfig 作为**路径与开关的参考**；不要直接覆盖目标树的同名文件（4.14 kbuild 语法与 7.3 有别）。
2. defconfig 中启用上表对应 `CONFIG_*`（原厂 `ares_defconfig` 已有，见 `config/wifi-vendor-defconfig.fragment`）。
3. 这些平台驱动同样是 4.14 源码，**本身也需要 4.14→7.3 的 API 适配**；本包只导出其头文件与构建清单，不代做平台实现的适配。
4. §2 的符号若目标树已有等价实现，用 `EXPORT_SYMBOL_GPL` 暴露即可；若没有，需按 `baseline/build-files/` 的定位把对应 `.c` 一并移植。
5. 编译期接线统一用 `integration/ccflags.mk`（含 `$(src)` 与 `$(BASELINE)` 两套 -I，已补祖先目录）。

> 机器可读模块表：`tools/baseline-modules.tsv`；逐文件 blob sha：`tools/baseline-manifest.tsv`。

---

## 勘误（stage-4）：§2 的 `buck_set_mode` 计数有误

`scan_baseline_modules.py`（本文件生成工具）原版 `count_refs` 未剥离 C 注释，
把 `msdc_cust.c` 里 7 行被注释掉的 `buck_set_mode(VCORE, enable)` 计成 7 次真实
调用，因而将 `buck_set_mode` 列入上表 §2。复核结论：

- **真实调用 0**，且该符号只在 `CONFIG_MTK_PMIC_CHIP_MT6335` 条件块声明，
  而 MT6893 是 `CONFIG_MTK_PMIC_CHIP_MT6359P=y` —— 属别款 PMIC 的废弃 API；
- 应从「链接期必须提供的 33 符号」**剔除**，有效符号为 **32**；
- `aee_rr_rec_clk` 的 2 次里 2 次是声明、1 处真实调用（在 stage-3 新增的
  `baseline/impl/clk-mt6873-pg.c`，受 `CONFIG_MTK_RAM_CONSOLE` 保护），
  且已找到真实开源实现并移植。

工具已修：`strip_c_comments()` + 剔除声明行。逐行证据与修正后计数见
`reports/SYMBOL-REFS-CORRECTED.md`；两个符号的完整解决见
`docs/UNRESOLVED-SYMBOLS.md` 与 `solution/platform-symbols.c`。
上表原文保留，作为审计对照，不作静默改写。
