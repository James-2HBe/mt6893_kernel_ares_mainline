# USB 外接网络（WiFi / 数据网络）支持说明

> 回答两个问题：**「这个内核能用自带 WiFi 和 SIM 卡 4G 吗？」→ 不能。**
> **「能用任何方式联网吗？」→ 可以，走外接 USB 设备 + 主线标准驱动。**
> 本文说明边界、做法、支持清单与真机未验证的风险。

## 1. 边界：自带 WiFi / 蜂窝为什么做不到

| | 自带 WiFi（connsys） | 自带蜂窝数据（CCCI / mddriver） |
|---|---|---|
| 交付板级 dts 的设备节点 | **没有**。只有 `consys-reserve-memory`、`gps-reserve-memory`、`modem,testing-mode` 三个保留内存/调试节点 | **没有**。SoC dtsi 里只有 `mddriver` + 一串 `ap_ccif*`/`md_ccif*` 私有节点 |
| 原厂 4.14 实现 | `drivers/misc/mediatek/connectivity/`：wlan/core 487 文件、conninfra 134、gps 161、bt 70+，走 WMT/STP 私有栈 | `mediatek,mddriver` + `ccci,modem_info_v2` + CLDMA/DPMAIF + CCIF 共享内存中断 |
| 主线 7.3.3 | 无 `mediatek,consys` / conninfra / `CONFIG_MTK_COMBO_WIFI`；`mt76` 等是**外挂 PCIe/USB/SDIO WiFi 芯片**驱动，不是 SoC 内置 connsys | 无 `mediatek,mddriver` / ccci / ccif 驱动 |
| 额外依赖 | 私有固件（WIFI_RAM_CODE 等）+ Android HAL | `md1img` 固件 + Android RIL + SIM/射频用户态栈 |
| 产物证据 | `System.map` 中 connsys/ccci/wmt 符号 **0** | 同左 |

**辨伪**：干净主线里 `mtk_wmt` 命中 4 个文件全是 `btmtk*.c` 的
`btmtk_wmt_hdr`（外挂蓝牙芯片的 HCI 分包头）；`ccmni` 命中 6 个是
`drivers/net/wwan/t7xx/` 的 DPMAIF 代码，那是 **PCIe 外挂 T7xx 5G 模组**
（PCI ID `0x14c3:0x4d75`、HP/Dell M.2 卡，`depends on PCI`），与 MT6893
SoC 集成 modem 不是一回事。三个可用内核树（6.19.14 / 7.2.12 / 7.3.3）
都**没有** MT6893 的 clk / connsys / ccci 驱动，换基线也解决不了。

> 缺的是整套私有连通栈 + 闭源固件 + Android HAL，属于**架构缺口**，不是
> 补 30 个驱动的工作量。在不硬改（不改内核源码、不硬套 compatible、
> 不搬闭源栈）的前提下，这一条无解。完整证据链、三条看似可行但不通的
> 路径、以及放宽边界要付的代价，见 `docs/NATIVE-NETWORKING.md`。

## 2. 本轮做法：只走上游标准机制

**原则**：不改一行主线内核源码，不碰 4.14 私有栈，不硬套不兼容的
compatible（SoC 专属串通过**补 binding enum** 正规加入，与
`mali-mt6893.patch` 同一做法）。只做三件事：DT 合规化、binding 补丁、配置开关。

### 2.1 DT：把 USB 控制器主线化为 host 模式

上一轮其实已经把基础铺好了：板级 dts 里 `tphy` 是
`"mediatek,mt6893-tphy", "mediatek,generic-tphy-v2"` + `status="okay"`，
子节点 `u2port` 是主线格式（`#phy-cells = <1>`），另有 `clk26m`
（fixed-clock）、`vusb33` / `otg_vbus`（regulator-fixed）全部合规。
本轮只处理 SoC 层 `usb@11201000` 及其 xHCI 子节点。

| 属性 | 4.14 原值 | 主线化后 | 依据 |
|---|---|---|---|
| `compatible` | `"mediatek,mtu3"`（单串） | `"mediatek,mt6893-mtu3", "mediatek,mtu3"` | 两个 yaml 都要求「SoC 专属串 + 通用串」；驱动 `of_match` 认通用串 |
| `clocks` | `<&topckgen 0x7b &infracfg_ao 0x25 &infracfg_ao 0x3b>`（私有 2-cell） | `<&clk26m>` + `clock-names = "sys_ck"` | `mtu3.yaml` / `mtk-xhci.yaml` 的 `clocks` minItems=1、首项须 `sys_ck`；驱动用 `devm_clk_bulk_get_optional()`，其余时钟可缺 |
| `phys` | `<&u2port &mt6360_pmu>`（第二项指向私有 PMIC 中断控制器） | `<&u2port 0>` | `mtu3.yaml` 的 `phys` minItems=1；`u2port` 已主线化，`#phy-cells=<1>` |
| `usb-role-switch` / `role-switch-default-mode` | `peripheral`（设备模式） | 删除 | 简化为纯 host；`role-switch-default-mode` 依赖 `usb-role-switch` |
| `dr_mode` | 无 | `"host"` | `usb-drd.yaml` 允许 `host` |
| xHCI 子节点 `clocks` | 同私有 3 项 | `<&clk26m>` + `"sys_ck"` | 同上 |
| xHCI 子节点 supplies | 无 | `vbus-supply = <&otg_vbus>`、`vusb33-supply = <&vusb33>` | `xhci_mtk_vregs_get()` 用非 optional 的 `devm_regulator_bulk_get()`，7.3.3 已无 `CONFIG_REGULATOR_DUMMY` |

由 `tools/fix_mainline_contract.py` **第 14 步**注入到成品 dts 末尾，幂等；
用 `/delete-property/` 删除 SoC 层私有属性后重新赋值，**不改 SoC dtsi**。
反编译验证（phandle 反查）：`clocks → clk26m`、`phys → usb-phy@0`、
`vusb33-supply → regulator-vusb33`、`vbus-supply → usb-otg-vbus-regulator`，
私有 `topckgen` / `mt6360_pmu` / role-switch 全部消失。

### 2.2 binding 补丁（2 个）

| 补丁 | 作用 |
|---|---|
| `port/dt-bindings/usb-mtu3-mt6893.patch` | `mediatek,mtu3.yaml` 的 compatible enum 增加 `mediatek,mt6893-mtu3` |
| `port/dt-bindings/usb-xhci-mt6893.patch` | `mediatek,mtk-xhci.yaml` 增加 `mediatek,mt6893-xhci` |

均已应用到内核树，并接入 `auto_install.sh` 的 `apply_compat` 幂等机制
（`git apply --check` + needle 检测）。**驱动源码无需改动**——mtu3 与
xhci-mtk 的 `of_match` 本来就含通用 fallback 串。

### 2.3 配置（`ares_defconfig.fragment`）

USB host：`CONFIG_USB_XHCI_HCD` / `USB_XHCI_MTK` / `USB_MTU3` / `USB_ROLE_SWITCH`。
USB 网络：`USB_USBNET`、`CDCETHER`、`CDC_NCM`、`RNDIS_HOST`、`CDC_MBIM`、
`QMI_WWAN`、`AX88179_178A`、`RTL8152`、`USB_WDM`、`USB_SERIAL_OPTION`、
`USB_SERIAL_WWAN`、`WWAN`。
USB WiFi 网卡：`CFG80211` / `MAC80211` / `MT76_CORE` / `MT76_USB` / `MT7921U` /
`RTL8XXXU` / `ATH9K_HTC`。

全部符号已在 7.3.3 Kconfig 中逐条确认存在，`force_defconfig.py` +
`olddefconfig` 后 17 项全部生效。

## 3. 支持哪些外接设备

| 想做什么 | 外接设备 | 主线驱动 | 固件 / 用户态 |
|---|---|---|---|
| WiFi | USB WiFi 网卡（MT7921U、RTL8xxxU、ATH9K_HTC、MT76 系 USB） | `mt7921u` / `rtl8xxxu` / `ath9k_htc` / `mt76_usb` | 固件放 `/lib/firmware`（如 `mt7921u.bin`）；`wpa_supplicant` + `dhcpcd` |
| 蜂窝数据（独立上网卡） | USB 4G/5G 上网卡（QMI 或 MBIM） | `qmi_wwan` / `cdc_mbim` + `usb_wwan` / `option` | `ModemManager`，或 `qmicli` / `mbimcli`；SIM 卡在上网卡上 |
| 蜂窝数据（借手机网络） | 另一台手机开「USB 共享网络」 | `rndis_host` + `cdc_ether` | `dhcpcd usb0` 即可，无需固件 |
| 有线网络 | USB 以太网适配器（AX88179 / RTL8152 / CDC ECM） | `ax88179_178a` / `rtl8152` / `cdc_ether` | `dhcpcd eth0` |

> 这些都是**外接 USB 设备**，需要自行准备硬件。固件不在内核树里，
> 需从 `linux-firmware` 取对应文件放到 `/lib/firmware`；`make_initramfs.py`
> 支持 `--include /path/fw.bin:/lib/firmware/fw.bin` 一并打进 initramfs。

## 4. 真机使用步骤（模板，未实机验证）

```bash
# 1) 确认 USB host 真的起来了（这是全部方案的前提）
dmesg | grep -iE 'mtu3|xhci|usb.*host'
lsusb

# 2) 插入设备，确认驱动绑定与网络接口出现
dmesg | tail -30
ip link

# 3a) USB WiFi 网卡
modprobe mt7921u                       # 或 rtl8xxxu / ath9k_htc
wpa_supplicant -i wlan0 -c /etc/wpa_supplicant.conf -B
dhcpcd wlan0

# 3b) USB 4G 上网卡（MBIM 例）
modprobe cdc_mbim
mbimcli -d /dev/cdc-wdm0 --query-device-caps
# 或交给 ModemManager 自动建 wwan0

# 3c) 手机 USB 共享网络（RNDIS，最省事）
modprobe rndis_host
dhcpcd usb0

# 3d) USB 以太网
dhcpcd eth0
```

用户态工具（`wpa_supplicant` / `dhcpcd` / `ModemManager` / `usbutils`）不在
本交付包内，需放进 rootfs。

## 5. 零额外硬件的替代：USB gadget 共享网络

如果「不外接」指的是**不想另买 USB 网卡 / 上网卡**，还有一条纯主线路径：
用一根普通 USB 线把手机连到 PC，让手机以 **USB 网卡**身份从 PC 借网络。

主线 7.3.3 的 `mtu3` 本来就是 Dual Role 控制器
（`drivers/usb/mtu3/Kconfig` 的 choice 默认 `USB_MTU3_DUAL_ROLE`）。
本交付的 `.config` 实测已带出 `USB_GADGET=y` / `USB_MTU3_DUAL_ROLE=y` /
`EXTCON=y` / `USB_CONFIGFS=m` / `USB_F_{RNDIS,ECM,NCM}=m`，
`ares_defconfig.fragment` 又把这些显式固化。即 **host 与 gadget 两侧驱动
都已在内核里**，只差 DT 的角色声明。

切换步骤（编译期二选一，不改内核源码）：

1. 把 `mt6893-xiaomi-ares-official.dts` 末尾 USB 块里的
   `#define ARES_USB_ROLE  ARES_USB_ROLE_HOST` 改成 `ARES_USB_ROLE_PERIPHERAL`；
2. 重新编译 dtb（`cpp` + `dtc`，flags 见 `verify.sh` 第 4 节）；
3. 用户态用 configfs 组一个 RNDIS（或 ECM / NCM）gadget（示例模板，未实机验证）：

```bash
modprobe libcomposite usb_f_rndis u_ether
mount -t configfs none /sys/kernel/config
cd /sys/kernel/config/usb_gadget && mkdir g1 && cd g1
echo 0x1d6b > idVendor; echo 0x0104 > idProduct   # 示例 VID/PID
mkdir -p configs/c.1 functions/rndis.0 strings/0x409
ln -s functions/rndis.0 configs/c.1/
echo "$(ls /sys/class/udc)" > UDC
ip link set usb0 up && dhcpcd usb0                # 或从 PC 侧共享网络
```

> 这**不是**手机自己用 WiFi / SIM 卡上网：网络来自 PC，手机是「被共享」的一端。
> 优点是零额外硬件、全主线机制、不碰任何私有栈；代价是必须连一台 PC，
> 且此时 USB host 功能（外接网卡、U 盘）不可用，ADB / MTP / fastboot
> 也不在本配置内。两种角色不能同时用，改宏重编 dtb 即可来回切。

`ARES_USB_ROLE=1` 的 dtb 已单独编译 + `dtbs_check` 验证：报错条数与 host
模式完全相同（1661 条），报错集合差异为空，绑定零回归。

## 6. 验证结果

| 验证项 | 结果 |
|---|---|
| `dt-validate`（用含 GPU/USB binding 补丁的 `processed-schema-usb.json`） | exit 0；交付 dtb 共 1661 条报错，**全部**为原厂板级 dts 既存私有节点的 dtschema 噪声（`bytes` 类型 1519 条、`vendor-prefixes` 40 条、spi/i2c 私有属性等），与本轮改动无关 |
| 零回归对比（去掉 USB 覆盖块、同 cpp flags 重编） | 注入前 **1670** 条 → 注入后 **1661** 条；消失的 9 条全是 USB binding 错误（compatible 单串不在 enum、`clock-names` 缺 `mcu_ck`、xHCI 子节点同样两项）；**新增 0 条实质错误** |
| 唯一文本差异 | `accdet` 的 `et7480-i2c-handle` phandle `0xcd → 0xcc`，注入块改变 phandle 编号分配所致，属同一既存噪声 |
| 注入节点自身 | `usb@11201000` / `usb@0` 在校验输出中 **0 条**报错 |
| dtb 反编译 | `compatible` 双串、`clocks → clk26m`、`phys → u2port 0`、`vbus-supply → otg_vbus`、`vusb33-supply → vusb33`、`dr_mode = "host"` 全部正确；`usb-role-switch` / `role-switch-default-mode` 已清除 |
| peripheral 角色 | 改宏重编 dtb 通过，dtbs_check 报错集合与 host **完全一致，差异为空** |
| 默认 host dtb | 加角色宏前后**字节完全一致**（222232 字节），零回归 |
| 配置生效 | fragment 中 USB/网络/GPU/EEPROM 共 17 项经 `force_defconfig.py` + `olddefconfig` 全部生效；新增 gadget 侧 12 项已确认 Kconfig 类型并显式固化 |
| `./verify.sh` | 全绿 |

> `dtbs_check` 的绝对条数不能当合规判据：1661 条噪声来自原厂 dts 里大量
> 无主线 binding 的私有节点（accdet / adsp / 电池参数表等），这些本就不在
> 移植范围内。判断本轮改动是否合规，用「去掉注入块重编、对比报错集合」的
> 零回归口径。

## 7. 已知限制与风险（务必先读）

1. **真机未验证**。沙箱只能验证编译、DT 覆盖语义与 schema；USB 是否真能
   枚举设备必须上机看 `dmesg`。最大不确定点是 `clocks = <&clk26m>`：
   主线驱动只做 `clk_bulk_prepare_enable()`（不设频率），真实 USB 时钟由
   bootloader/硬件默认维持——这个假设未在 ares 上验证过。
2. **自带 WiFi / SIM 卡 4G 仍然不可用**。本方案不提供 connsys 或 CCCI。
3. **host 与 peripheral 二选一**：默认 `dr_mode = "host"`，此时 OTG / 设备
   模式、ADB、MTP、fastboot 不可用。按第 5 节改 `ARES_USB_ROLE` 宏重编 dtb
   可切到 peripheral 共享 PC 网络，但外接网卡就不能用了。
4. **`otg_vbus` 是无 GPIO 的 `regulator-fixed`**，enable 是 no-op；VBUS
   实际由硬件/bootloader 维持，未验证。
5. **外接设备固件与用户态工具需自备**，不在内核树也不在 initramfs 里。
6. 若实机发现 `xhci_mtk` probe 失败，优先查 `dmesg` 里 regulator / phy
   的 `-EPROBE_DEFER`，再考虑给 `tphy` 补主线时钟，属于后续迭代。

## 8. 相关文件

- `tools/fix_mainline_contract.py`（第 14 步注入）
- `mt6893-xiaomi-ares-official.dts` / `.dtb`
- `port/dt-bindings/usb-mtu3-mt6893.patch`、`usb-xhci-mt6893.patch`
- `ares_defconfig.fragment`、`auto_install.sh`
- `docs/DRIVER-COVERAGE.md`（整体驱动覆盖口径）
- `docs/NATIVE-NETWORKING.md`（板载原生 WiFi / 蜂窝可行性评估）
