# 板载原生 WiFi / 蜂窝数据：可行性评估

> 回答：**「能不能不外接任何东西，让这台手机用自己的 WiFi 和 SIM 卡上网？」**
> 结论：**在「不硬改」的前提下做不到。** 这不是漏移植了若干驱动，
> 而是主线内核与 MTK 私有连通栈之间的**架构缺口**。本文给出可复核的证据链、
> 三条看起来能走实际走不通的路、以及放宽边界要付的代价。

---

## 1. 结论速览

| 目标 | 不硬改能否做到 | 约束内替代方案 |
|---|---|---|
| 板载 WiFi（MT6635 combo） | **不能** | 外接 USB WiFi 网卡（`mt7921u` / `rtl8xxxu` / `ath9k_htc`） |
| 板载蜂窝数据（MT6893 集成 modem） | **不能** | 外接 USB 4G/5G 上网卡（QMI / MBIM），或另一台手机 USB 共享网络 |
| 不买任何外接硬件也能联网 | **能，但不是本机自上网** | USB 线连 PC，用 ConfigFS 起 RNDIS/ECM/NCM gadget 借 PC 网络 |
| 改内核、搬私有栈换「真原生」 | 超出「不硬改」边界，见第 5 节 | 需用户明确放宽 |

「不硬改」= 不改主线内核源码迁就厂商、不硬套不兼容的 `compatible`、
不搬 4.14 私有栈与闭源固件。这一边界是上一轮明确约定的。

---

## 2. 硬件与固件事实（原厂 4.14）

原厂 `ares_defconfig` 里，无线与基带全部由 MTK 私有 Kconfig 控制：

```
CONFIG_MTK_COMBO=y
CONFIG_MTK_COMBO_CHIP_CONSYS_6893=y
CONFIG_MTK_COMBO_BT=y
CONFIG_MTK_COMBO_GPS=y
CONFIG_MTK_COMBO_WIFI=y
CONFIG_MTK_FM_CHIP="MT6635_FM"
CONFIG_MTK_CONNSYS_DEDICATED_LOG_PATH=y
CONFIG_WLAN_DRV_BUILD_IN=y
CONFIG_MTK_ECCCI_C2K=y
CONFIG_MTK_BTIF=y
```

要点：

- WiFi / BT / FM / GPS 是 **MT6635 combo 芯片**，挂在 connsys 上，
  `CONFIG_MTK_COMBO_CHIP_CONSYS_6893` 就是 MT6893 专属连通子系统。
- 蜂窝是 **CCCI / mddriver** 体系（`CONFIG_MTK_ECCCI_C2K` + BTIF），
  数据面走 CLDMA / DPMAIF + CCIF 共享内存中断。
- **原厂 dtsi 里没有 WiFi 或 modem 的设备节点**。只有
  `wifi-reserve-memory` / `gps-reserve-memory` 这类保留内存、
  `modem,testing-mode` 调试节点，以及 `&msdc1`（给 combo 用的 SDIO 控制器）。

最后一条是决定性的：**这些驱动不走 DT probe**，靠平台设备硬编码 +
WMT 固件下载 + Android HAL 拉起。所以连「写一段合规 DT 描述板载硬件」
这个最常见的前提都不成立——没有可描述的 binding，内核里也没有
会去匹配它的驱动。

---

## 3. 主线 7.3.3 缺什么（干净内核树逐条核查）

核查对象是从官方 tar 解出的 `/tmp/clean-kmods/linux-7.3.3`，
**不是**已被本项目注入驱动的内核树，避免误判。

| 核查项 | 干净主线 7.3.3 结果 |
|---|---|
| `drivers/misc/mediatek/connectivity/`（wlan / conninfra / gps / bt） | **不存在**。原厂该栈约 wlan 487 + conninfra 134 + gps 161 + bt 70+ 个文件 |
| `CONFIG_MTK_COMBO_*` / `CONFIG_MTK_ECCCI_*` Kconfig | **不存在** |
| CCCI / mddriver / CCIF 驱动 | **不存在** |
| `Documentation/devicetree/bindings/` 中 mt6631 / mt6635 / connsys / wmt 条目 | **0 个** |
| `drivers/net/wireless/mediatek/` | 只有 `mt76/`、`mt7601u/` |
| `drivers/net/wwan/` | `iosm`、`t7xx`、`mhi_wwan_*`、`rpmsg_wwan_ctrl`、`qcom_bam_dmux`、`wwan_core` —— 均非 MT6893 集成 modem |
| 交付 `System.map` 中 connsys / ccci / wmt 符号 | **0** |

另外三个可用内核基线（6.19.14 / 7.2.12 / 7.3.3）都**没有** MT6893 的
clk / connsys / ccci 驱动，换基线也解决不了。

---

## 4. 三条看似可行、实际不通的路（辨伪）

### 4.1 「主线 mt76 支持 SDIO，MT6635 也是 SDIO，能直接用吗？」

不能。主线 `MT76_SDIO` 只被两个芯片选中：

- `MT7663S`（`mt7615/Kconfig`，802.11ac 2x2 SDIO **独立模块**）
- `MT7921S`（`mt7921/Kconfig`，802.11ax SDIO **独立模块**）

MT6635 是 connsys combo，固件格式、寄存器、上电流程完全不同：它上电后
**默认不枚举 SDIO function**，必须先由 WMT 通过 UART 下载
`WIFI_RAM_CODE` 与 patch、配置 pinmux 与时钟，才让 SDIO 侧可用。
通用 SDIO 枚举拿不到设备，`mt76` 的 chip ID 也匹配不上。
硬写 `compatible = "mediatek,mt7663s"` 属于「硬套不兼容 compatible」，
正是被禁止的做法，而且驱动 probe 后依然起不来。

### 4.2 「主线有 DPMAIF / CLDMA，蜂窝有戏？」

代码确实存在，但在 `drivers/net/wwan/t7xx/` 里，属于
`MTK_T7XX`：`depends on PCI`，是 **MediaTek T7xx 系列 PCIe 外挂 5G
模组**（M.2 数据卡，常见于笔记本 WWAN 槽）。它与 MT6893 SoC 内部
集成的 modem 不是同一套硬件，也没有 CCCI 通道、没有 `md1img` 加载路径。
把 `mediatek,mddriver` 硬套到 `mtk_t7xx` 上不会工作。

### 4.3 「补一段 DT 节点 + 打开配置总行吧？」

不行。第 2 节已说明原厂根本没有这些设备节点；主线也没有对应
binding 和驱动。补节点只会产生一个 `status = "okay"` 但无驱动匹配的
空壳，外加 `dtbs_check` 报「compatible 未文档化」。这不是缺配置，是缺
整套内核侧实现。

---

## 5. 要真原生，需要放宽到什么程度

如果愿意解除「不硬改」，理论上唯一的路是把原厂 4.14 的连通栈整体
搬过来。需要的东西（**全部**缺一不可）：

| 类别 | 内容 | 问题 |
|---|---|---|
| 内核私有栈 | connsys / WMT / STP / conninfra / wlan core / gps / bt；CCCI / CCIF / CLDMA / DPMAIF / mddriver | 数千文件，且深度绑定 4.14 的内部 API；7.3.3 的 `regmap`、`pinctrl`、`clk`、`power_domain`、`net_device`、固件加载等接口已大改，不是拷目录就能编 |
| 闭源固件 | `WIFI_RAM_CODE_*`、combo patch、`md1img`（modem 镜像）、射频校准数据 | 不在内核树，不在 `linux-firmware`，只在 Android vendor 分区，且受签名/版本校验 |
| 用户态栈 | Android WiFi HAL、`libmtk-ril` / RIL daemon、`ccci_mdinit`、SIM 与射频控制、netd 配置 | 纯主线 rootfs 没有，需要整套 Android vendor 环境 |
| DT / 时钟 / 电源 | connsys 专属时钟、`mt6360_pmu` 中断控制器、pinmux、regulator | 主线无 clk-mt6893，正是本包一直在用 `clk26m` 规避的缺口 |

**成功率评估：极低。** 即便放宽边界，也要同时满足「4.14 私有栈在 7.3.3
上编得过」「拿得到匹配版本的闭源固件」「跑得起 Android vendor 用户态」
三个条件；其中后两个不在内核移植能解决的范围内。这也是本项目从一开始
就把边界划在「少量外设 + 主线标准机制」的原因。

---

## 6. 约束内全部可行路径

| 路径 | 需要额外硬件 | 是否动私有栈 | 网络来自 | 文档 |
|---|---|---|---|---|
| USB WiFi 网卡（host） | USB 网卡 | 否 | 外接网卡连 WiFi | `USB-NETWORKING.md` §3 |
| USB 4G/5G 上网卡（host） | USB 上网卡 + SIM | 否 | 上网卡自己的蜂窝 | 同上 |
| 手机 USB 共享网络（host, RNDIS） | 另一台手机 + 线 | 否 | 另一台手机 | 同上 |
| USB 以太网（host） | USB 网卡 | 否 | 有线网络 | 同上 |
| **USB gadget 共享 PC 网络（peripheral）** | **仅一根 USB 线** | **否** | **PC** | `USB-NETWORKING.md` §5 |

最后一行是「不外接硬件」这一诉求在约束内能拿到的最接近解：
纯主线 ConfigFS gadget（RNDIS / ECM / NCM），内核的 mtu3 已编成
Dual Role，只需把 dts 末尾的 `ARES_USB_ROLE` 改成 peripheral 重编 dtb。
但要如实说明：**它借的是 PC 的网络，不是这台手机自己的 WiFi / SIM 卡**，
且切过去后 USB host 功能（外接网卡、U 盘）不可用。

---

## 7. 决策建议

1. **维持现状**（推荐）：保留 host 默认角色，用外接 USB 设备联网；
   想零硬件时按 §5 切换角色连 PC。已编译、已过 `dtbs_check` 零回归验证。
2. **放宽「不硬改」**：接受搬 4.14 私有栈 + 闭源固件 + Android vendor
   用户态，尝试真原生。工作量与风险远大于本包此前全部工作，且成功与否
   取决于拿不到内核侧控制的固件与用户态，建议先确认这些资源是否可得。

无论选哪条，**板载 WiFi 与 SIM 卡数据在纯主线 7.3.3 上不可用**这一事实
不变；任何声称「补几个驱动就能用」的做法都会落到 §4 的三类辨伪里。

---

## 8. 相关文件

- `docs/USB-NETWORKING.md`（外接与 gadget 路径、验证结果）
- `docs/DRIVER-COVERAGE.md`（整体驱动覆盖口径）
- `mt6893-xiaomi-ares-official.dts`（末尾 USB 块的 `ARES_USB_ROLE`）
- `ares_defconfig.fragment`（host 网络设备 + gadget ConfigFS 配置）
- 参考内核：`vendor-reference/kernel_mt6893/ares_defconfig`（原厂私有开关）
