# 第二数据源：James-2HBe/kernel_mt6893 @ chopin_kvm

> 用途：**辅助移植与参数交叉验证**，不是可直接编译进主线 7.3.3 的设备树。
> 归档时间：2026-11-10；来源：GitHub raw + tree API（分支 `chopin_kvm`）。

## 仓库性质

| 项 | 值 |
|---|---|
| 内核版本 | Linux **4.14.357-openela**（`Makefile`: VERSION=4 PATCHLEVEL=14 SUBLEVEL=357） |
| 描述 | Kernel for Xiaomi MT6893 (**tested on chopin and ares**) with KVM/virtualization |
| 默认分支 | `chopin_kvm`（另有 `chopin_kvm_oc`、`feat/susfs-integration` 等） |
| `mediatek/` 设备树 | 138 个 `.dts/.dtsi/.dws` |
| 本目录归档 | **29 个文件**（28 个 dts/dtsi + `ares_defconfig`），约 516 KB |

## 与 `vendor-reference/official/`（小米官方 ares-r-oss）的关系

26 个同名文件中 **14 个完全相同、12 个有实质差异**。这是同一套小米官方 ares
设备树的**另一分支版本**（去掉 `Copyright (C) 2021 XiaoMi` 行，并带 chopin/ares
真机修正与 KVM 适配），不是新 SoC 数据。

### 主要差异（本轮核实）

| 文件 | 差异 |
|---|---|
| `xiaomi-mt6893-common.dtsi` | +144 行：AW8697 完整波形校准、`&i2c9` 触觉块；注释掉 `&i2c11 aw2028` 与 testing-mode |
| `ares-mt6893.dtsi` | +327 行：FT3658U SPI 触控全参数（`display-coords = <0 0 17280 38400>`）、WL2866D 相机 LDO、`thermal-message` |
| `mt6893.dts` | +158 行：`fhctl-new`、`cqdma` 主线 binding 化、reserve-memory-ssmr、`dpmaif_capability` 6→14、`&msdc1 disabled` |
| `k6893v1_64.dts` | `tee-only`（指纹 SPI）、相机 sensor 名、`irq_nfc` 修正、触控 include 改为 `cust_mt6893_touch.dtsi` |
| `cust_mt6893_touch.dtsi` | GT9886 分辨率 2280→**2300**、固件 `6893v01`→**`6893v02`** |
| `mtk_pe50.dtsi` / `mt6360_pd.dtsi` / `tcpc_config.dtsi` | 充电曲线、PD VDO 3→6 cell、`usbr20_not_used` 调整 |
| `mt6315_s6.dtsi` | 补 `6_vbuck4` regulator |
| `cust_mt6885_msdc.dtsi` | `no-mmc`、`#ifndef CONFIG_FPGA_EARLY_PORTING`、删 hs200 状态 |
| `mt6360.dtsi` | `en_otg_wdt`、VMC `regulator-always-on` |

`cust_mt6893_msdc.dtsi` 与该仓库的 `cust_mt6885_msdc.dtsi` **字节相同**（同一文件两名）。

## 为什么不能整体复用

1. 是 **4.14 厂商根 dts**（`mt6893.dts` 8391 行），自带 SoC 层，与主线/目标仓库
   已有 `mt6893.dtsi`(6235 行) **重复定义**；
2. `#include <generated/autoconf.h>` + `CONFIG_MTK_*` 条件编译，主线 autoconf 无这些符号；
3. 依赖 4.14 私有 `dt-bindings/*` 头；`mediatek,MT6893` 等私有 compatible 主线无 binding；
4. `mtkfb`/`odm`/`fhctl` 等 MTK 私有框架节点在 7.3.3 无驱动；
5. 含 **chopin 专属改动**（`mt6893.dts` 末尾 `&msdc1 { status="disabled"; }` 注释明写
   "chopin has no microSD slot … as ares does"），直接套用会把 chopin 的板级假设带进 ares。

## 本轮实际用到的 4 类辅助价值

1. **确认硬件批次真实并存**：`ares_defconfig` 同时有
   `CONFIG_TOUCHSCREEN_FOCALTECH=y` 与 `CONFIG_TOUCHSCREEN_MTK_TOUCH="GT9886 GT9896S"`，
   另有 `CONFIG_MTK_FINGERPRINT_SELECT="GF3658"`、`CONFIG_INPUT_AW8697_HAPTIC=y`、
   `CONFIG_MTK_GAUGE_VERSION=30`、`CONFIG_BQ2597X_CHARGE_PUMP=y`、
   `CONFIG_REGULATOR_WL2866D=y`、`CONFIG_NFC=y`。
   → 证明三触控批次与已移植的 9 个驱动都是 ares 实机器件，不是拼凑。
2. **校准 irtx 真值**：两仓库官方根 dts 的 `pwm_ch` 都是 `<3>`，
   而 lineage 板级 overlay 写 `<0>` → 主 dts 改回官方真值 3。
3. **确认触控互斥做法**：既然三批次在官方也并存（靠 defconfig 二选一），
   主 dts 用 cpp 宏 `ARES_TOUCH_VARIANT` 编译期互斥而非删节点，是正确解法。
4. **参数出处核对**：官方 `mt6359p.dtsi` 的 `ldo_vusb` = 3.0V；
   AW8697 `vib_effect_max` 存在 170(官方 ares-r-oss) / 197(本分支) 两个批次值；
   GT9886 固件版本 6893v01 / 6893v02。

> `vusb33` 保持 3.3V 未按 `ldo_vusb`(3.0V) 修改：它是主线 SoC 层
> `vusb33-supply` 的 3.3V 电源域，与 PMIC `ldo_vusb` 不是同一轨，且无 ares 板级
> 实测依据，不擅自改动。

## 归档清单（29 个）

`ares.dts`, `ares-mt6893.dtsi`, `ares_defconfig`, `k6893v1_64.dts`,
`k6893v1_64_k510.dts`, `xiaomi-mt6893-common.dtsi`, `mt6893.dts`,
`cust_mt6893_msdc.dtsi`, `cust_mt6885_msdc.dtsi`, `cust_mt6893_camera.dtsi`,
`cust_mt6893_touch.dtsi`, `cust_mt6893_touch_1080x2300.dtsi`,
`cust_mt6893_touch_120hz.dtsi`, `cust_mt6893_touch_boost.dtsi`,
`cust_mt6893_touch_gt9886_1080_2340.dtsi`,
`mt6359p.dtsi`, `mt6360.dtsi`, `mt6360_pd.dtsi`, `mt6315_s3.dtsi`,
`mt6315_s6.dtsi`, `mt6315_s7.dtsi`, `mtk_pe50.dtsi`, `tcpc_config.dtsi`,
`trusty.dtsi`, `wl2866d1.dtsi`, `wl2866d2.dtsi`,
`agate-mt6893.dtsi`, `amber-mt6893.dtsi`, `saga-mt6893.dtsi`。

> 后 3 个是同 SoC 其他板型（agate/amber/saga），仅作横向参考，非 ares 数据。
> 原仓库路径均为 `arch/arm64/boot/dts/mediatek/<文件名>`（`ares_defconfig` 在
> `arch/arm64/configs/`）。这些文件**不参与主线 Kbuild 编译**。
