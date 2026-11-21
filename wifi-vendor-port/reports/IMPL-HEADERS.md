# 平台实现依赖头补齐报告

由 `tools/complete_impl_headers.py` 生成。来源 `xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f`，jsdelivr CDN + 逐文件 blob sha 校验。

| 指标 | 值 |
|---|---|
| 补导符号实现 `.c` | 3 |
| BFS 轮次 | 3 |
| 导出的 MTK 平台内部头 | **25** |
| 未能定位的 include | 4 |

## 1. 补导的符号实现

- `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c`（75912 字节，sha `cf90affd2dc5`）
- `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u_v2.c`（90167 字节，sha `4d1ab4c09792`）
- `drivers/usb/mtu3/mtu3_dr.c`（16285 字节，sha `ab3d5807d78c`）

## 2. 导出的依赖头（按 include → 源路径）

| #include | lineage-23.2 路径 |
|---|---|
| `charger_class.h` | `drivers/misc/mediatek/include/mt-plat/charger_class.h` |
| `linux/ioctl.h` | `include/uapi/linux/ioctl.h` |
| `linux/param.h` | `include/uapi/linux/param.h` |
| `mach/mt_iommu.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/mt_iommu.h` |
| `mach/mtk_pmic.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/mtk_pmic.h` |
| `mach/pseudo_m4u.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/pseudo_m4u.h` |
| `mach/pseudo_m4u_plat.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/pseudo_m4u_plat.h` |
| `mach/upmu_hw.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/upmu_hw.h` |
| `mach/upmu_sw.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/upmu_sw.h` |
| `mt-plat/aee.h` | `drivers/misc/mediatek/include/mt-plat/aee.h` |
| `mt-plat/charger_type.h` | `drivers/misc/mediatek/include/mt-plat/charger_type.h` |
| `mt-plat/mtk_auxadc_intf.h` | `drivers/misc/mediatek/include/mt-plat/mtk_auxadc_intf.h` |
| `mt-plat/mtk_battery.h` | `drivers/misc/mediatek/include/mt-plat/mtk_battery.h` |
| `mt-plat/mtk_boot.h` | `drivers/misc/mediatek/include/mt-plat/mtk_boot.h` |
| `mt-plat/mtk_charger.h` | `drivers/misc/mediatek/include/mt-plat/mtk_charger.h` |
| `mt-plat/upmu_common.h` | `drivers/misc/mediatek/include/mt-plat/upmu_common.h` |
| `mtk_boot_common.h` | `drivers/misc/mediatek/include/mt-plat/mtk_boot_common.h` |
| `mtk_boot_reason.h` | `drivers/misc/mediatek/include/mt-plat/mtk_boot_reason.h` |
| `mtk_chip.h` | `drivers/misc/mediatek/include/mt-plat/mtk_chip.h` |
| `mtk_devinfo.h` | `drivers/misc/mediatek/include/mt-plat/mtk_devinfo.h` |
| `pseudo_m4u.h` | `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/pseudo_m4u.h` |
| `sync_write.h` | `drivers/misc/mediatek/include/mt-plat/sync_write.h` |
| `trustzone/kree/system.h` | `drivers/misc/mediatek/include/mt-plat/trustzone/kree/system.h` |
| `tz_cross/ta_m4u.h` | `drivers/misc/mediatek/include/mt-plat/trustzone/tz_cross/ta_m4u.h` |
| `tz_cross/trustzone.h` | `drivers/misc/mediatek/include/mt-plat/trustzone/tz_cross/trustzone.h` |

## 3. 未能在源树定位的 include

- `custome_external_battery.h`（1 次）
- `simulator_kernel.h`（1 次）
- `sys/types.h`（1 次）
- `sys/stat.h`（1 次）

这些属于条件编译分支 / 生成头 / 工具链头，不计入平台基线，由目标树按需提供。

## 4. 两个仍无 `.c` 实现的符号

- `buck_set_mode`：源树 `.c` 与已导出头中均未定位到定义，需由目标树「已修基线层」提供或补 `EXPORT_SYMBOL_GPL` 桩。
- `aee_rr_rec_clk`：源树 `.c` 与已导出头中均未定位到定义，需由目标树「已修基线层」提供或补 `EXPORT_SYMBOL_GPL` 桩。

## 5. 完整性

全部条目追加到 `tools/platform-impl-manifest.tsv`；导出位置 `baseline/platform/<原路径>`，裸 include 另有 `baseline/platform/flat/` 副本。

## 6. 定点补齐（finish_impl_headers.py）

BFS 因多 variant 歧义未落地的头，按固定路径保真补齐：

| 源树路径 | 说明 | 状态 |
|---|---|---|
| `drivers/misc/mediatek/include/mt-plat/mt6873/include/mach/mtk_cpufreq_api.h` | MT6893 用 mt6873 平台分支；按原路径导出后 -I 命中 mach/... | 已导出 |
| `include/linux/usb/class-dual-role.h` | MTK 私有 USB dual-role 头，7.3 主线不含 | 已导出 |

`asm/dma-iommu.h` 故意不补：源树仅有 `arch/arm/include/asm/dma-iommu.h`（4.14 arm32），aarch64 目标下属条件编译死分支，导入反而污染。
