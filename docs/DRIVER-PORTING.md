# ares 驱动移植清单（Linux 7.3.3 主线 / James-2HBe 仓库）

> 核对基线：
> - 目标树 `James-2HBe/mt6893_kernel_ares_mainline @ 7.3.3-mt6893-xiaomi-chopin`（= Linux 7.3.3 主线 + MT6893 SoC 层）
> - 官方源 `MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`（小米官方 4.14 内核，Android R）
> - 主线干净树 `/workspace/linux-7.3.3`（逐文件 grep 验证）
>
> 原则（按你的要求）：**仓库中已有且真实可用的，一律不重复移植**。
>
> **更新（第 2 轮）**：B 类驱动代码级移植见 `docs/PORTED-DRIVERS.md`；
> 「失败/受限驱动换方式按实机参数适配」见 **`docs/PORT-ROUND2.md`**，
> 实机参数来源见 **`docs/REAL-HW-PARAMS.md`**（本轮补齐 4 份官方私有头、
> 15 个标准 `*-gpios`、9 个驱动全部严格编译通过）。
> 本文档保留当时的甄别结论、数据源与共性障碍分析，作为背景材料；
> 其中「代码改写未执行」「需补 GPIO」「寄存器占位」等表述为**早期状态**，
> 已不适用于当前交付。

---

## 一、结论速览

ares 板级设备树里出现的外设 compatible 共 34 个，按驱动来源分三类：

| 类别 | 数量 | 处理方式 |
|---|---|---|
| **A 主线 7.3.3 已有驱动** | 8 | ✅ **不重复移植**，只需在 `ares_defconfig` 打开 Kconfig 开关 |
| **B 官方 4.14 私有驱动（主线缺失）** | 11 | ⚠️ 需从 4.14 搬移/重写（本次归档源码 + 改造要点） |
| **C 厂商框架节点（无独立驱动）** | 其余 | 仅数据节点，运行时不 probe，与 chopin 现状一致 |

---

## 二、A 类：主线已有，**不重复移植**（只需开 config）

这些驱动在 Linux 7.3.3 干净树里已存在且是标准 binding。ares 的 compatible
与主线驱动可能不完全一致，需要补 DT compatible 或确认已在驱动的 `of_match` 里。

| ares 设备 | 设备树 compatible | 主线 7.3.3 驱动 | Kconfig 开关 | 备注 |
|---|---|---|---|---|
| 67W 电荷泵（主/从） | `ti,bq2597x-master` / `ti,bq2597x-slave` | `drivers/power/supply/bq25980_charger.c` | `CONFIG_CHARGER_BQ25980` | **compatible 不同**：主线是 `ti,bq25980`/`ti,bq25975`/`ti,bq25960`；bq2597x 是同族，需在驱动 `of_match` 补两条 |
| 电量计 | `ti,bq27z561_i2c0` / `ti,bq27z561_i2c9` | `drivers/power/supply/bq27xxx_battery_i2c.c` | `CONFIG_BATTERY_BQ27XXX` + `_I2C` | 主线已支持 `bq27z561`；ares 用了自定义后缀名，需补 compatible |
| 立体声功放 ×2 | `cirrus,cs35l41` | `sound/soc/codecs/cs35l41-i2c.c` | `CONFIG_SND_SOC_CS35L41_I2C` | 完全匹配，直接可用 |
| 子 PMIC ×2 | `mediatek,mt6315-regulator` | `drivers/regulator/mt6315-regulator.c` | `CONFIG_REGULATOR_MT6315` | 通用 compatible，直接可用 |
| 主 PMIC | `mediatek,mt6359-regulator` | `drivers/regulator/mt6359-regulator.c` | `CONFIG_REGULATOR_MT6359` | 直接可用 |
| MT6360 PMIC/充电/LED/PD | `mediatek,mt6360` 系列 | `drivers/mfd/mt6360-core.c` + 子驱动 | `CONFIG_MFD_MT6360` 及各子项 | MFD 核心已有；charger/FLED/TCPC 子驱动需逐项确认 |
| NFC | `mediatek,nfc` / st21nfc | `drivers/nfc/`（st21nfca / st-nci 框架） | `CONFIG_NFC_ST21NFCA` 等 | ares 用 ST21NFC（非 NFCA），见 B 类说明 |
| USB Type-C PD | `mt6360_typec` | `drivers/usb/typec/` 主框架 | `CONFIG_TYPEC` + TCPM | 主线有通用框架，MTK 私有 tcpc 层需适配 |

> **这批是本次"不重复移植"的主体。** 直接复用主线代码，比搬 4.14 私有驱动
> 更可靠（主线驱动经过 review、API 与 7.3.3 一致、有 dt-bindings）。

---

## 三、B 类：官方 4.14 私有驱动（主线缺失，需搬移/重写）

以下 compatible 在 7.3.3 主线**全树 grep 无对应驱动**，只能从官方 `ares-r-oss`
搬源码。归档于 `drivers/src/`（已抓到的）。

| # | ares 设备 | compatible | 官方 4.14 源码（defconfig 线索） | 主线替代/难度 |
|---|---|---|---|---|
| 1 | 触控 FT3658U | `focaltech,fts` | `drivers/input/touchscreen/GT9886/` `GT9896S/` `FT3518/`（CONFIG_TOUCHSCREEN_MTK_TOUCH="GT9886 GT9896S"） | 高：MTK 私有 `tpd` 框架 + `spi` 私有队列，需重写为标准 `input` + `spi_driver` |
| 2 | 屏下指纹 GF3658 | `goodix,goodix-fp` | `CONFIG_MTK_FINGERPRINT_SELECT="GF3658"`（drivers/input/fingerprint/ 或 misc/mediatek/fingerprint/） | 高：依赖 MTK `fp` HAL 私有 ioctl/`netlink` 通路；主线无对应子系统 |
| 3 | 线性马达 AW8697 | `awinic,aw8697_haptic` | `CONFIG_INPUT_AW8697_HAPTIC=y` | 中：可改为主线 `input/ff-memless` 或新写 `leds-class` haptic；主线已有 `CONFIG_INPUT_AW8697`?（需确认版本） |
| 4 | 双电荷泵 bq2597x | `ti,bq2597x-*` | MTK `mtk_charger` 私有框架（`CONFIG_MTK_CHARGER=y`） | **建议走 A 类**：主线 bq25980 驱动补 compatible 即可，无需搬 MTK 私有 charger 框架 |
| 5 | 电量计 bq27z561 | `ti,bq27z561_i2c*` | `CONFIG_MTK_GAUGE_VERSION=30` / `bat_gm30` | **建议走 A 类**：主线 bq27xxx 已支持 |
| 6 | 相机 LDO ×2 | `willsemi,wl2866d1/2` | `drivers/regulator/wl2866d.c`（已抓 18KB） | 低：regulator 驱动，改 regulator_desc/API 后可直接用 |
| 7 | 音频开关 ET7480 | `mediatek,et7480-audioswitch` | sound/soc/codecs 或 misc | 中：简单 switch，可重写为 `snd_soc_component` 或小 misc 驱动 |
| 8 | LED AW2028 | `awinic,aw2028_i2c` | drivers/leds（MTK `mtk_leds` 私有） | 低：可改用主线 `leds-class` + `leds-aw20xx`（若已有）或新写 |
| 9 | USB 充电识别 XMUSB350 | `xm,xmusb350-charger` | drivers/power/supply 或 misc | 中 |
| 10 | 弹出式肩键 gamekey | `gamekey`（GPIO 111/112 + 霍尔 4/5） | `CONFIG_MTK_GAMEKEY` 类（misc/mediatek） | 低：本质是 `gpio-keys` + 霍尔输入，可用主线 `gpio-keys` 重写 |
| 11 | ST21NFC | `mediatek,nfc` | `drivers/misc/mediatek/nfc/st21nfc/st21nfc.c`（已抓 39KB） | 中：主线有 `st-nci`/`st21nfca`，ST21NFC 是另一颗，需端口到 `nci` 框架 |

### 官方私有驱动的共性障碍（4.14 → 7.3.3 是重写，不是拷贝）

1. **依赖 MTK 私有平台层**：`drivers/misc/mediatek/include/mt-plat/` 下的
   `mtk_charger.h`、`mt_boot.h`、`mt_gpio.h` 等，主线完全没有。搬驱动前必须先
   决定：是连私有框架一起搬（会污染主线树），还是改写为通用 API。
2. **API 漂移（9 年跨度）**：
   - `power_supply` → `power_supply_desc` / `psy_cfg`
   - `regulator_desc` 的 `.ops` 签名、`of_parse_cb`
   - `i2c` / `spi` 的 `probe` 从 `probe(client, id)` 到单参数
   - `input` 设备的 `devm_input_allocate_device`
   - `pinctrl` 从 `pinctrl_select_state` 到 `pinctrl_pm_select_*`
   - GPIO 从数值编号到 `descriptor`（`gpio_desc`/`gpiod_*`）
3. **DT binding 全部是私有字符串**，主线 `dtbs_check` 会报缺 binding；
   需为每个驱动新写 `Documentation/devicetree/bindings/*.yaml`。
4. **无真机无法验证**：probe 是否成功、中断/电气参数是否正确，必须在
   ares 真机上串口调试。本环境只做源码归档 + 静态改造要点，不声称可运行。

---

## 四、C 类：厂商框架数据节点（无独立驱动，与 chopin 现状一致）

`mediatek,red`/`green`/`blue`/`lcd-backlight`/`vibrator`（odm LED 节点）、
`mediatek,camera_eeprom`、`mediatek,phy_tuning`、`k10_42_02_0a_dsc_cmd,lcm`（面板）、
`modem,testing-mode`、`xiaomi,simtray-status`、`xiaomi,cp-qc30`/`usbpd-pm`、
`mediatek,usb_c_pinctrl` 等。这些依赖 MTK Android 私有 HAL/框架，主线没有对应
驱动，节点只是数据。搬移它们的驱动需连同整套 MTK 用户态框架，超出内核移植范畴。

---

## 五、建议的落地顺序

1. **先起串口**：官方 `chosen` 已带 `console=ttyS0,921600n1`，配合主线
   `8250_mtk`（补 MT6893 compatible）——这是唯一可靠调试通道。
2. **A 类 8 个驱动**：在 `ares_defconfig` 打开主线 Kconfig，补 2~3 条
   compatible（bq2597x / bq27z561），优先让 PMIC / 充电 / 电量计工作。
3. **B 类按难度挑**：先从 `wl2866d`（regulator）、gamekey（gpio-keys）这类
   低难度、可用主线框架重写的入手；触控/指纹最后做。
4. **不要连 MTK 私有平台层一起搬**，否则会把 7.3.3 树拖回 4.14 的私有化状态。

---

## 六、本次未验证项（如实声明）

- 无 ares 真机，所有驱动 probe 结果、中断、电气参数**均未上电验证**。
- 官方 4.14 私有驱动源码只归档了已成功抓取的部分（见 `drivers/src/`）；
  GitHub 对部分路径返回 404/超时，未抓到的在表中标注了 defconfig 线索供你本地补。
- 4.14 → 7.3.3 的具体代码改写已在后续阶段完成（9 个新驱动 +
  2 个 A 类补丁，arm64 严格编译 0 error / 0 warning，见
  `docs/PORTED-DRIVERS.md`）。但**仍无真机验证**：寄存器占位值、
  DT 的 `*-gpios` 改造、GT9886/GF3658 的协议补齐，仍需按该文档逐项推进。
