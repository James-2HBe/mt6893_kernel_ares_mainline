# ares 官方设备树全量归档

来源：**`MiCode/Xiaomi_Kernel_OpenSource` @ `ares-r-oss`**（小米开源的原厂 4.14 内核）。
本目录是该仓库中 **ares / k6893v1_64 / MT6893 编译链所需的全部设备树原件**（41 个），
按原仓库相对结构摆放，供 Linux 主线 7.3.3 移植使用。

原始路径前缀：

| 本目录路径 | 原仓库路径 |
|---|---|
| `*.dts` / `*.dtsi` / `Makefile` | `arch/arm64/boot/dts/mediatek/` |
| `bat_setting/*.dtsi` | `arch/arm64/boot/dts/mediatek/bat_setting/` |
| `dws/k6893v1_64.dws` | `drivers/misc/mediatek/dws/mt6885/` |
| `headers/mt*.h` | `include/dt-bindings/{clock,gce,iio,memory,mfd,mmc,pinctrl}/` |
| `headers/goodix_ts_core.h` / `gf_spi_tee.h` / `aw8697*.h` | `drivers/input/...`（驱动私有头） |
| `k6893v1_64_defconfig` | `arch/arm64/configs/` |

## 文件清单（41）

### 根 dts 与板级（6）

| 文件 | 行 | 作用 |
|---|---:|---|
| `ares.dts` | 21 | ares 入口，`#include "mediatek/ares-mt6893.dtsi"` |
| `ares-mt6893.dtsi` | 733 | ares 归属配置，include 板级共用 |
| `xiaomi-mt6893-common.dtsi` | 907 | 板级共用外设（触控/指纹/充电/肩键/NFC/…） |
| `k6893v1_64.dts` | 835 | 项目底层配置（chosen videolfb / odm LED / mmc0 / pio / PMIC 电压） |
| `mt6893.dts` | 8276 | **4.14 官方完整根 dts**（SoC + 板级基础节点） |
| `cust_mt6893_camera.dtsi` | 623 | 相机传感器/供电引脚 |

### 触控配置（5，不同 IC / 不同批次）

| 文件 | 行 | 说明 |
|---|---:|---|
| `cust_mt6893_touch_1080x2300.dtsi` | 184 | **官方 include 链实际使用的那份**；GT9886 = 1080×2280 |
| `cust_mt6893_touch.dtsi` | 184 | GT9886 / GT9896S |
| `cust_mt6893_touch_120hz.dtsi` | 571 | Novatek NVT SPI 120Hz 触控（另一硬件批次） |
| `cust_mt6893_touch_boost.dtsi` | 100 | `goodix_thp,gt9896s` 2300 分辨率 |
| `cust_mt6893_touch_gt9886_1080_2340.dtsi` | 101 | GT9886 + 2340 分辨率（另一硬件批次） |

### 电池 / gauge（3）

| 文件 | 行 | 说明 |
|---|---:|---|
| `bat_setting/mt6893_battery_prop.dtsi` | 77 | `bat_gm30: battery{...}` 真实定义（`CONFIG_MTK_GAUGE_VERSION=30`） |
| `bat_setting/mt6893_battery_table.dtsi` | 537 | 电池充放电曲线表（内嵌 include） |
| `bat_setting/mt6893_battery_prop_dim2_ext.dtsi` | 77 | dim2 扩展校准参数（内嵌 include） |

### PMIC / 电源 / 外设（12）

`mt6315_s3/s6/s7.dtsi`(37/31/37)、`mt6359p.dtsi`(577)、`mt6360.dtsi`(199)、
`mt6360_pd.dtsi`(132)、`cust_mt6885_msdc.dtsi`(208)、`mtk_pe50.dtsi`(64)、
`trusty.dtsi`(66)、`tcpc_config.dtsi`(159)、`wl2866d1.dtsi`(43)、`wl2866d2.dtsi`(43)。

### DWS 源文件（1）

| 文件 | 行 | 说明 |
|---|---:|---|
| `dws/k6893v1_64.dws` | 2591 | **编译期生成 `k6893v1_64/cust.dtsi` 的 DWS 源**（XML，227 个 GPIO + eint/i2c/pmic/clk 模块）。原仓库不含生成后的 `cust.dtsi`，此文件是其唯一上游来源。 |

### 私有头文件（12）

| 文件 | 行 | 说明 |
|---|---:|---|
| `headers/goodix_ts_core.h` | 767 | GT9886 帧协议常量 / board_data |
| `headers/gf_spi_tee.h` | 267 | GF3658 `GF_IOC_*` ioctl + netlink 契约 |
| `headers/aw8697.h` | 465 | AW8697 驱动定义（CHIPID `0x97`） |
| `headers/aw8697_reg.h` | 539 | AW8697 全寄存器表 |
| `headers/mt6885-pinfunc.h` | 1357 | 4.14 私有 pinmux 宏（主线对应 `mt6893-pinfunc.h`） |
| `headers/mt6885-clk.h` | 1024 | 4.14 私有时钟 ID |
| `headers/mt6885-gce.h` | 554 | GCE 事件 ID |
| `headers/mt6885-larb-port.h` | 361 | IOMMU larb port |
| `headers/mt6359-irq.h` / `mt6315-irq.h` | 104 / 26 | PMIC IRQ 号 |
| `headers/mt635x-auxadc.h` / `mt6885-msdc.h` | 34 / 42 | AUXADC / MSDC 常量 |

### 配置（2）

`k6893v1_64_defconfig`(469 行，实机 BOM 权威来源)、`Makefile`(16 行，4.14 dts 目录)。

## 主线 7.3.3 的适配方式

官方 4.14 根 dts **不能原样编译进主线**，原因：
使用 `#include <generated/autoconf.h>`、4.14 私有 `dt-bindings/*` 头、
`CONFIG_MTK_*` 条件编译、`mediatek,MT6893` 等私有 compatible，且自带 SoC 层会与
主线/目标仓库已有的 `mt6893.dtsi` 重复定义。

因此采用**分层移植**：

1. **本目录 41 个原件原样安装**到主线内核树
   `arch/arm64/boot/dts/mediatek/ares-official/`（可追溯数据源，不参与 Kbuild 编译）；
2. `tools/assemble_ares_dts_v3.py` 从 `mt6893.dts` / `mt6360*.dtsi` / `bat_setting/*`
   提取 ares 真实节点，与板级 overlay 合并，生成
   `mt6893-xiaomi-ares-official.dts`；
3. `port/normalize_dt_gpios.py` 补 15 个标准 `*-gpios`（主线 `devm_gpiod_get()` 契约）；
4. `install_official.sh` 把上述成果接入主线内核树 Makefile，
   `make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb` 编译出 dtb。

## v3 相对 v2 的关键改进

v2 有 6 个「最小桩」（空节点），v3 全部消除：

| 桩 | v3 处理 |
|---|---|
| `irtx_pwm` | 换成 `mt6893.dts` L7475 官方真实节点（`mediatek,irtx-pwm` / `pwm_ch=<3>`） |
| `bat_gm30` | 换成 `bat_setting/` 三份文件拼接的 666 行官方真实电池定义 |
| `mmc0_pins_default` / `_uhs` | 核对 `xiaomi-mt6893-common.dtsi` L60/L61，引用本来被注释 → 删除 |
| `nfc_clk_req_active` / `_suspend` | 核对 L893/L895，引用本来被注释 → 删除 |

详见 `docs/OFFICIAL-FULL-DTS.md`。
