# 固件清单（由 init 从原厂 vendor 提取）

> 按你的分工，固件二进制**不在本包内**，由 `init` 从原厂 vendor 分区提取。
> 本文件给出「要提哪些文件、叫什么名、放哪、驱动在哪找」，避免你逐个翻源码。
>
> 全部文件名/路径由本包导出的**原厂源码常量反查**得到，来源标注了具体文件与行，
> 不是凭 MTK 惯例猜的。**但具体版本后缀（eco version / flavor / IP set）由真机
> 硬件和 vendor 里的实际文件决定**，见第 3 节。

---

## 1. 芯片与固件族

MT6893 = **Connac2x2 SOC3_0**（`CONFIG_MTK_COMBO_CHIP=CONNAC2X2_SOC3_0`，
`WLAN_CHIP_ID=6893`），固件文件统一带 **`soc3_0`** 前缀。

combo 芯片 MT6635 同时含 WiFi/BT/GPS/FM，WMT 负责统一下固件；
WiFi 侧 `wlan/core/gen4m/chips/soc3_0/` 是 SOC3_0 专属代码。

## 2. WiFi 固件（wlan 驱动 `kalFirmwareImageMapping` 加载）

### 2.1 RAM code（主固件）

基名定义（`wlan/core/gen4m/include/config.h:149`、
`chips/soc3_0/soc3_0.c:170`）：

```c
#define CFG_FW_FILENAME  "WIFI_RAM_CODE"
uint8_t *apucsoc3_0FwName[] = { (uint8_t *) CFG_FW_FILENAME "_soc3_0", NULL };
```

即基名 = **`WIFI_RAM_CODE_soc3_0`**。驱动在 `soc3_0.c:2700-2755`
会按 4 种候选名依次尝试（`%u` = `CFG_WIFI_IP_SET`=1，`%s` = flavor，
通常为空，`1` 为固定尾号）：

| 候选顺序 | 文件名模板 | 例（IP set=1, 无 flavor） |
|---|---|---|
| Type 1 | `WIFI_RAM_CODE_soc3_0_%u%s_%u.bin` | `WIFI_RAM_CODE_soc3_0_1_1.bin` |
| Type 2 | `WIFI_RAM_CODE_soc3_0_%u%s_%u` | `WIFI_RAM_CODE_soc3_0_1_1` |
| Type 3 | `WIFI_RAM_CODE_soc3_0` | `WIFI_RAM_CODE_soc3_0` |
| Type 4 | `WIFI_RAM_CODE_soc3_0.bin` | `WIFI_RAM_CODE_soc3_0.bin` |

另有 `CFG_CR4_FW_FILENAME = "WIFI_RAM_CODE2"`（config.h:150，CR4/第二核固件，
部分配置会用到）。

### 2.2 Patch / ROM EMI（三个 `_hdr.bin`，`soc3_0.c:830-845, 2825-2855`）

| 用途 | 文件名模板 | 例（eco ver=1） |
|---|---|---|
| WMMCU patch | `soc3_0_patch_wmmcu_%u%s_%x_hdr.bin` | `soc3_0_patch_wmmcu_1_1_hdr.bin` |
| WMMCU ROM EMI | `soc3_0_ram_wmmcu_%u%s_%x_hdr.bin` | `soc3_0_ram_wmmcu_1_1_hdr.bin` |
| WiFi ROM EMI | `soc3_0_ram_wifi_%u%s_%x_hdr.bin` | `soc3_0_ram_wifi_1_1_hdr.bin` |

其中 `%x` = `wlanGetEcoVersion()`（芯片 ECO 版本，运行时从寄存器读），
`%s` = `kalGetFwFlavor()`（flavor，对应 DT 里 `flavor_bin = "a"`），
`%u` = `CFG_WIFI_IP_SET`（=1）。**这些值决定最终文件名，务必按真机 vendor
里实际存在的文件名匹配，或把这几种组合都放齐。**

## 3. WMT 侧配置与 patch（conninfra/WMT 上电下载）

来源 `common/common_main/core/include/wmt_conf.h`、`wmt_core.h`：

| 文件 | 定义处 | 说明 |
|---|---|---|
| `WMT.cfg` | `wmt_conf.h:33` `CUST_CFG_WMT` | WMT 主配置（GPIO/时钟/波特率等板级参数） |
| `WMT_SOC.cfg` | `wmt_conf.h:34` `CUST_CFG_WMT_SOC` | SOC 相关配置 |
| `mt66xx_patch_hdr.bin` | `wmt_core.h:59` `defaultPatchName` | WMT 默认 combo patch（运行时可被 `WMT.cfg`/探测覆盖，实际名多为 `soc3_0_patch_*` 或 vendor 定制名） |
| `WIFI_RAM_CODE*` 等 | 见第 2 节 | combo 各功能镜像 |

> 同 combo 的 BT 侧还有 `BT_RAM_CODE_MT%04x_*_hdr.bin`、`btPowerTable.dat`、
> `bt.cfg` 等（本包 `bt/` 源码可见），若只做 WiFi 可不提，但 WMT 上电流程
> 可能仍会找 combo patch，建议**整包 vendor firmware 目录一起拷**，别手工挑。

## 4. NVRAM / 射频校准数据

`wlan_lib.c` 的 `WIFI_CFG_PARAM_STRUCT` / `pre_cal.c` 表明驱动会读 NVRAM
里的射频校准与功率表（不是独立 firmware 文件，通常来自 vendor 的
`nvram` 分区或 `WIFI` NVRAM 数据文件，经 `wmt_cdev_wifi` 的 `NVRAM` 命令注入）。
这部分**必须来自该台机器的原厂 vendor/NVRAM 分区**，不能用其它机器的，
否则射频参数不匹配。

## 5. 放置路径与 init 要点

1. **目标目录**：主线 `request_firmware()` 默认找 `/lib/firmware/`（原厂是
   `/etc/firmware` 或 `/vendor/firmware`，见 `LD_usbbt.c` 注释及 wmt 路径常量）。
   移植到主线时二选一：
   - 把固件放进 `/lib/firmware/`，或
   - 改驱动里的固件路径常量 / 加 `firmware_class.path` 内核参数指向 vendor 挂载点。
2. **时机**：`init` 必须在 `wlan`/`conninfra` 驱动加载**之前**把固件就位；
   否则 probe 阶段 `kalFirmwareImageMapping` 找不到文件直接失败。
3. **版本匹配**：固件与驱动、芯片 ECO 版本强绑定。请用**与这台机器原厂 ROM
   同一版本**的 vendor 分区提取，混版本大概率下载失败或起来后异常。
4. **签名/校验**：部分 vendor 固件带 header（`_hdr.bin` 即带 header 格式），
   不要自行裁剪/重命名，保持原文件名。
5. **权限**：`/lib/firmware` 下文件需可读（一般 `0644`）。

## 6. 本包不含固件的原因

- 固件是闭源二进制，不在内核源码树，也不在 `linux-firmware`；
- 只存在于该设备的 Android `vendor` / `vendor/firmware` 分区，且受版本/签名约束；
- 由你用 `init` 从原厂 vendor 提取是本方案既定分工（见 `../README.md` 第 6 节）。

> 真机未验证。上面所有文件名来自原厂源码常量；最终以 vendor 分区里
> 实际文件名 + 真机 `dmesg` 中 `kalFirmwareImageMapping` / WMT 打印的
> 「trying file name」为准，缺哪个补哪个。
