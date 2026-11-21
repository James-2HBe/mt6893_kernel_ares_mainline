# 原厂 WiFi 私有栈 → 主线内核 移植指南

> 面向：把本包 `drivers/` 里的 4.14 私有 WiFi 栈移植到你自己的主线内核。
> 本包只负责「把料备齐」，具体改代码由你完成。以下按实际工作量排序，
> 给出改动面、判断依据和验证口径，避免你逐文件盲改。

---

## 0. 先建立正确预期

这不是「拷目录 + 加两行 Makefile」能完成的事。4.14 vendor kernel 与
7.x 主线之间，MTK 私有栈依赖的内部 API 大面积变更，且这套栈**强耦合
非 DT 的平台数据/全局符号**。请按「先让它编过 → 再让它 probe → 最后调通
固件与数据面」三阶段推进，不要指望一次到位。

一个现实建议：**先只移植 `wlan` + 最小 `conninfra`/`common` 子集**，
目标是 `insmod` 后能加载、能看到 `wlan0`，再逐步补 BT/GPS/FM。
本包把这些模块都导出了，但不代表要一次性全上。

## 1. 依赖图（谁依赖谁）

```
        wlan0 (net_device, cfg80211)
          │  glRegisterBus / AXI(Connac2x2 SOC3_0) 平台 probe
          ▼
   connectivity/wlan/core/gen4m        ← WiFi 驱动主体（501 文件）
          │  wmt 上电/下固件/STP 收包
          ▼
   connectivity/common/common_main     ← WMT/STP 核心（117 文件）
   connectivity/common/common_detect   ← wmt_detect：找芯片、读 GPIO/保留内存
          │  conninfra 提供寄存器访问/时钟/电源/固件下载
          ▼
   connectivity/conninfra (203)        ← connsys 基础设施
   connectivity/connfem (19)           ← 射频前端
   connectivity/power_throttling (4)
          │
          ├── drivers/misc/mediatek/btif        STP 传输（UART）
          ├── drivers/misc/mediatek/conn_md     connsys↔modem 通道
          ├── drivers/misc/mediatek/include/mt-plat  私有头文件（mt-plat/*）
          ├── drivers/misc/mediatek/emi* / devmpu / mddp   保留内存 / EMI MPU
          └── drivers/mmc/host/mediatek/ComboA  SDIO host（msdc1，仅参考导出，非 MT6893 WiFi 通路）+ wmt-sdmmc.c
          │
          ▼
   DT: consys@18000000 / wifi@18000000 / 保留内存 / regulator (vcn33_*_wifi)
   固件: WIFI_RAM_CODE_* / WMT.cfg / combo patch / 射频校准（vendor 分区）
```

关键点：**`wlan` 不直接读 `wifi@18000000`**。它由 `wmt_detect` 探测芯片、
`conninfra` 上电并下固件，之后才注册网络设备。所以 DT 节点与驱动不是
主线常见的 `of_match → probe` 一对一关系，移植时不能只补 binding。

## 2. 阶段一：让它编过（Kconfig / Makefile 接线）

### 2.1 主线落点

主线 7.3.3 里**没有** `drivers/misc/mediatek/`（已确认）。需要新建并挂接线：

| 主线文件 | 需要加的 | 参考原厂 |
|---|---|---|
| `drivers/misc/Kconfig` | `source "drivers/misc/mediatek/connectivity/Kconfig"` | `config/wifi-vendor-defconfig.fragment` 的 `MTK_COMBO` 体系 |
| `drivers/misc/Makefile` | `obj-y += mediatek/` | 原厂 `mediatek-Makefile`（见 `dts/original/` 同级 vendor-reference） |
| `arch/arm64/configs/defconfig` | 打开 `MTK_COMBO*`（**先别抄主线没有的符号**，见 2.2） | `config/wifi-vendor-defconfig.fragment` |
| `drivers/net/wireless/Kconfig/Makefile` | 若把 wlan 挂到标准无线目录 | - |

### 2.2 配置符号的处理（最容易踩的坑）

`config/wifi-vendor-defconfig.fragment` 里的 `CONFIG_MTK_COMBO_*` /
`CONFIG_WLAN_DRV_BUILD_IN` 等是**原厂私有 Kconfig 符号**，主线 Kconfig 里
根本不存在。两种做法：

- **A（推荐起步）**：把原厂 `connectivity/Kconfig`、`Makefile`、`Kbuild`
  一起搬进主线，让这些私有符号由搬进来的 Kconfig 定义，defconfig 片段
  才有意义。
- **B**：先在搬入的 Kconfig 里把开关收敛成 `bool`/固定 `y`，不依赖
  defconfig，编译通了再挂出来。

> 无论 A/B，都不要把 fragment 直接喂给主线 `make defconfig`：会被
> `olddefconfig` 当作未知符号丢弃，看起来「加了但没生效」。

### 2.3 头文件 / 全局符号

`connectivity` 大量 `#include <mt-plat/xxx.h>`、`#include "conninfra.h"`
以及引用原厂导出的全局函数（`emi_mpu_set_region_protection`、
`clk_buf_ctrl`、`upmu_set_reg_value`、`KAL_*`/`OSAL` 抽象层等）。
本包已把 `include/` 与 `emi*/devmpu/mddp` 一并导出，但**这些依赖本身
也绑定 4.14 的平台实现**，主线没有对应 provider。这是阶段一最大的
改动面，需要为每个缺失符号二选一：实现 shim，或改用主线等价接口。

## 3. 阶段二：让它 probe（DT 与平台数据）

### 3.1 私有 DT 节点

`dts/wifi-private-nodes.dtsi` 已按原文抽出 14 个节点，分两类：

| 类别 | 节点 | 主线处理 |
|---|---|---|
| 保留内存 | `consys-reserve-memory` (`0x450000`)、`wifi_mem` (`shared-dma-pool`, `0xF20000`)、`gps-reserve-memory` | 主线 `reserved-memory` 框架可直接表达，但 `wifi_mem` 的 phandle 引用要与驱动取 `memory-region` 的方式对齐 |
| 私有设备 | `consys: consys@18000000` (`mediatek,mt6893-consys`)、`wifi: wifi@18000000` (`mediatek,wifi`)、`bt@18000000`、`msdc1` (`mediatek,msdc`)、`btif`、`msdc1_top`、`emi*` | 主线无这些 binding。要么写主线风格 binding + 驱动补 `of_match_table`，要么继续走原厂平台数据（不推荐，等于放弃 DT 化） |

### 3.2 必须先解决的前置缺口（否则节点写了也没用）

这几项正是本交付一直用 `clk26m` 规避、主线尚未补齐的部分，移植时绕不开：

- **时钟**：原厂 `clocks = <&scpsys ...>` / `infracfg_ao` 的 connsys 专属
  clock id，主线没有 `clk-mt6893`。需要 clk 驱动或用 fixed-clock 顶上。
- **电源**：`vcn13/vcn18/vcn33_1_wifi/vcn33_2_wifi` 来自 `mt6360/mt6359p`
  PMIC，主线 regulator binding 不同，需要映射。
- **中断控制器 / pinctrl**：consys 的中断、`msdc1` 的 pinmux 用原厂
  `PINMUX_GPIO*` 宏，主线 pinctrl 是另一套。
- **phandle 是反编译值**：`dts/original/mt6893.dtsi` 是从原厂 dtb 反编译的，
  里面的 `phandle = <0x198>`、`memory-region = <0xed>` 是数值引用，
  **不能直接编译**，必须还原成 label 引用（`<&wifi_mem>` 等）。
  `dts/wifi-private-nodes.dtsi` 里已保留部分 label 形式，`consys` 段仍需手工还原。

### 3.3 probe 顺序

`wmt_detect`（找芯片）→ `conninfra`（上电/时钟/下固件）→ `wlan`（注册
`wlan0`）。任何一步失败都不会有网络设备。建议用 `initcall_debug` +
在三个入口加打印，先确认调用链是否走到。

## 4. 阶段三：固件与数据面（你负责的部分）

- 固件清单与 vendor 提取路径见 `firmware/FIRMWARE-LIST.md`。
- `init` 需在 `wlan` 驱动加载前把固件放到 `/lib/firmware`（或改驱动里的
  `FW_NAME` / `request_firmware` 路径常量），否则 probe 会卡在固件下载。
- 数据面通了还需要用户态：`wpa_supplicant` / `netd` 等；Android WiFi HAL
  与主线 rootfs 的差异不在本包范围。

## 5. 主线适配点清单（按接口分类，逐类排查）

| 接口面 | 4.14 原厂用法 | 7.x 主线变化 | 处理 |
|---|---|---|---|
| 网络设备 | 私有 `glue`/`prGlueInfo` + `net_device` 直接操作 | `net_device` 字段/锁、`ndo` 回调签名变更 | 逐函数比对 |
| 无线栈 | 自带 cfg80211 对接层 | `cfg80211_ops`、`wiphy`、`regulatory` 变更 | 用主线 `cfg80211` 重写对接 |
| 固件加载 | `request_firmware` + 私有 `kalFirmwareLoad` | 接口在，但路径/异步要求变 | 核对 `FW_NAME` 与 `/lib/firmware` |
| Host interface | 原厂 `CONFIG_MTK_COMBO_WIFI_HIF=axi`（Connac2x2 SOC3_0），走 conninfra AXI，非 SDIO | 主线无 conninfra AXI 平台驱动 | 这是核心改动面：需自己实现 AXI 寄存器访问/中断/固件下载的 provider |
| SDIO/MMC | `ComboA` 私有 msdc 驱动（本包仅参考导出） | 主线 `mtk-sd` + `mmc` 框架 | 非 MT6893 WiFi 通路，别在此投入 |
| 时钟 | `scpsys` / `infracfg_ao` 私有 id | 主线 clk 框架 + `clk_bulk` | 补 clk-mt6893 或 fixed-clock |
| 电源/regulator | `vcn33_*_wifi` 来自 mt6360 | 主线 regulator binding | 映射 + DT `*-supply` |
| pinctrl | `PINMUX_GPIO*` | 主线 pinctrl-mt6893 | 重写 pinmux |
| 保留内存 | `mediatek,consys-reserve-memory` 私有解析 | `reserved-memory` + `dma` API | 用主线 `of_reserved_mem` |
| 内核协作 | `KAL`/`OSAL`、`proc`/`sysfs`、wakelock、`time`/`timer` | 大量废弃（`init_timer` 等） | 建 shim 层统一替换 |
| 调试 | `pr_no_info`/`DBGLOG` 宏、`conninfra` log | 主线 tracepoint/dynamic debug | 保留但裁剪宏 |

> 上表是「要查哪些面」，不是「改这几处就够」。真实改动量取决于你选
> 「全搬 shim」还是「逐接口主线化」，前者快但离主线远，后者慢但干净。

## 6. 验证口径（建议）

1. **编译**：`make LLVM=1 ARCH=arm64 <defconfig> -j` 全过，无 `implicit
   declaration`、无未定义符号（用 `nm`/`modpost` 查）。
2. **加载**：`insmod`/builtin 起来，`dmesg` 里 probe 链三段都有打印。
3. **设备**：`ip link` 出现 `wlan0`；`iw dev` 能扫描（扫描通说明固件+数据面基本通）。
4. **DT**：`dtbs_check` 用你自己写的 binding 校验，确保私有节点不引入
   新的 schema 报错。
5. **回归**：本包不影响 `../` 现有 7.3.3 交付，改主线前先确认那套仍 `verify.sh` 全绿。

---

## 附：交叉引用

- `../docs/NATIVE-NETWORKING.md` —— 为什么纯主线做不到原生 WiFi、三条辨伪、
  放宽边界的代价（本包的立项背景）
- `../docs/USB-NETWORKING.md` —— 不搬私有栈时的两条替代联网路径
- `../README.md` —— 现有 7.3.3 交付工程总览
- `dts/wifi-private-nodes.dtsi` —— 私有节点原文（带出处行号）
- `config/wifi-vendor-defconfig.fragment` —— 原厂开关
- `firmware/FIRMWARE-LIST.md` —— 固件清单
