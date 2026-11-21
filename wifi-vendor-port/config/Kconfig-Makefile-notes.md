# 原厂构建接线（Kconfig / Makefile）说明

> 来源：`drivers/misc/mediatek/connectivity/{Kconfig,Makefile}` @
> `seriaTvT/kernel_mt6893` `c651f31e`。以下为**原文要点**，不是主线可用的接线。
> 目的是让你移植时知道原厂靠哪些符号、哪些目录、哪些编译期宏把栈串起来。

---

## 1. 顶层 Kconfig 体系

`connectivity/Kconfig` 定义 `MTK_COMBO` 这个 tristate 总开关，下面用 `choice`
选芯片，MT6893 对应：

```
config MTK_COMBO_CHIP_CONSYS_6893
    bool "MTK MT6893 combo chip support"
...
config MTK_COMBO_CHIP
    string
    default "CONSYS_6893" if MTK_COMBO_CHIP_CONSYS_6893
```

WiFi 相关关键符号（全部是**原厂私有符号，主线 Kconfig 不存在**）：

| 符号 | 作用 |
|---|---|
| `MTK_COMBO` | 总开关 |
| `MTK_COMBO_CHIP_CONSYS_6893` | 选 MT6893 consys |
| `MTK_COMBO_WIFI` | WiFi 驱动（`select WIRELESS_EXT` / `WEXT_PRIV`） |
| `WLAN_DRV_BUILD_IN` | 「把 wlan 编进内核」而非模块 |
| `MTK_COMBO_BT` / `GPS` / `FMRADIO` | 同 combo 其他功能 |
| `MTK_CONNFEM` | 射频前端 |
| `MTK_BTIF` | BT/STP 传输 |
| `MTK_CONNSYS_DEDICATED_LOG_PATH` | connsys firmware log 走专用通道 |

> 注意 `MTK_COMBO_WIFI` 会 `select WEXT_PRIV` —— **WEXT（Wireless Extensions）
> 在现代主线里已被 cfg80211 取代甚至移除**，这是移植时必须处理的一处
> 硬依赖，不能照搬。

## 2. 顶层 Makefile：`WLAN_DRV_BUILD_IN=y` 导出的编译期环境

`connectivity/Makefile` 在 `CONFIG_WLAN_DRV_BUILD_IN=y` 时 `export` 一批变量，
子目录 Makefile（尤其 `wlan/core/gen4m/Makefile`）靠它们决定编哪些芯片代码：

```
export CONFIG_MTK_COMBO_WIFI_HIF=axi      # ← 关键：HIF 是 AXI，不是 SDIO
export MTK_ANDROID_WMT=y
export MTK_ANDROID_EMI=y
export WIFI_IP_SET=1
export MTK_WLAN_SERVICE=yes
export CONNAC_VER=2_0
export CFG_BUILD_CONNAC2=true
```

MT6893 分支：

```
ifeq ($(CONFIG_MACH_MT6893), y)
export MTK_COMBO_CHIP=CONNAC2X2_SOC3_0   # ← 芯片代号，对应 wlan/core/gen4m/coda/soc3_0
export BT_PLATFORM=6893
export WLAN_CHIP_ID=6893
endif
```

还定义 `MTK_WCN_REMOVE_KERNEL_MODULE` / `MTK_WCN_BUILT_IN_DRIVER`，把
模块形态改成内建。

> **重要修正**：MT6893 的 WiFi host interface 是 **AXI / conninfra**
> （`CONFIG_MTK_COMBO_WIFI_HIF=axi`，Connac2x2 SOC3_0），
> 不是 SDIO。SDIO (`msdc1`) 是同机其它用途 / 老 combo 的路子，本包仍导出
> `ComboA` msdc 与 `wmt-sdmmc.c` 作为参考，但**不要把它当成 MT6893 WiFi
> 的数据通路去接**。真机移植请以 `gen4m` + `conninfra` 的 AXI 路径为准。

## 3. 挂进编译的目录（`obj-$(CONFIG_*) +=`）

```
obj-$(CONFIG_MTK_COMBO)                      += conninfra/
obj-$(CONFIG_MTK_COMBO_WIFI)                 += wlan/adaptor/
obj-$(CONFIG_MTK_COMBO_CHIP_CONSYS_6893)     += wlan/core/gen4m/   # MT6893 走这条
obj-$(CONFIG_MTK_BTIF)                       += bt/mt66xx/btif/
obj-$(CONFIG_MTK_COMBO_GPS)                  += gps/
obj-$(CONFIG_MTK_FMRADIO)                    += fmradio/
obj-$(CONFIG_MTK_CONNFEM)                    += connfem/
```

`connadp.o`（`common/*_build_in_adapter.o` + `power_throttling/*.o`）是
built-in 适配层，把 wlan/conninfra 的初始化挂到内核启动流程。

## 4. 头文件搜索路径（`subdir-ccflags-y`）暴露的私有依赖

Makefile 里这一串 `-I` 直接说明了这套栈还依赖哪些**本包之外**的原厂目录：

```
drivers/misc/mediatek/base/power/include
drivers/misc/mediatek/clkbuf/src
drivers/misc/mediatek/include/mt-plat/$(MTK_PLATFORM)/include
drivers/misc/mediatek/pmic/include[/mt6359]       # PMIC_NEW_ARCH / MT6359
drivers/mmc/core
drivers/misc/mediatek/eccci/$(MTK_PLATFORM)       # ← 连 eccci 都引用
drivers/misc/mediatek/eccci/
drivers/clk/mediatek/
drivers/pinctrl/mediatek/
drivers/misc/mediatek/power_throttling/
```

也就是说，`connectivity` 即便只编 WiFi，也会拉进 `base/power`、
`clkbuf`、`mt-plat`、`pmic`、`eccci`、`clk`、`pinctrl` 这些原厂私有目录的
头文件。本包已导出 `include/`（mt-plat 头文件）与 `conn_md`/`emi*`，但
`base/power`、`clkbuf`、`pmic/include`、`eccci`、`clk/mediatek`、
`pinctrl/mediatek` **不在本包导出范围内**（它们是整棵树级的平台代码）。

> 移植时这是最大的隐性工作量：要么把这些平台目录也搬过来（等于搬大半个
> vendor kernel），要么为 `connectivity` 写一个「只暴露必需符号」的轻量
> shim 层，把这些 `-I` 和未定义符号收敛掉。建议后者。

## 5. 主线落点建议（对照 PORTING-GUIDE.md 第 2 节）

1. 在目标主线新建 `drivers/misc/mediatek/`，把本包 `drivers/misc/mediatek/`
   放进去。
2. `drivers/misc/Kconfig` 加 `source "drivers/misc/mediatek/connectivity/Kconfig"`，
   `drivers/misc/Makefile` 加 `obj-y += mediatek/`。
3. **先别抄 defconfig 片段**；先让搬进来的 Kconfig 能定义上述私有符号，
   再用 `make menuconfig` 打开 `MTK_COMBO` / `CONSYS_6893` / `MTK_COMBO_WIFI` /
   `WLAN_DRV_BUILD_IN`，确认 `.config` 里真的生效（会被 `olddefconfig` 丢弃
   未知符号的问题见 `PORTING-GUIDE.md` 2.2）。
4. 处理 WEXT、头文件 `-I`、平台私有符号三类硬依赖，逐个消编译错误。
5. 编译过了再进 `PORTING-GUIDE.md` 第 3 节的 DT/probe 阶段。
