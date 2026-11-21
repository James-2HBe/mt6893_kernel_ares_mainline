# ares B 类驱动 — 厂商 DT 契约与改造清单

> 原则：**不改动已交付的 `mt6893-xiaomi-ares.dts` 主体**（它是官方 + lineage 交叉数据），
> 新驱动主动去解析厂商私有属性；只有主线框架确实无法解析的（MTK `pio` 数值三元组、
> 私有 `*-gpio` 命名）才建议补标准属性。
>
> 主线 `dtbs_check` 会因这些 vendor compatible / 私有属性大量报错，属预期：
> 本交付以「dtb 生成成功 + 驱动能 probe」为目标，不追求 upstream binding 合规。

## 总表

| # | 设备 | compatible | 总线 / 地址 | 驱动 | 是否需改 dts |
|---|---|---|---|---|---|
| 1 | 相机 LDO ×2 | `willsemi,wl2866d1` / `willsemi,wl2866d2` | i2c4 @0x28 | `regulator-wl2866d.c` | **否**，全部解析 |
| 2 | RGB LED | `awinic,aw2028_i2c`（已规范化） | i2c11 @0x65 | `leds-aw2028.c` | **否**，已修 |
| 3 | 线性马达 | `awinic,aw8697_haptic` | i2c7 @0x5A | `haptic-aw8697.c` | 复位脚需补 |
| 4 | 弹出肩键 | `xm,gamekey` | 根节点 platform | `input-gamekey.c` | **必须补 GPIO** |
| 5 | 音频开关 | `mediatek,et7480-audioswitch` | i2c9 @0x42 | `et7480-audioswitch.c` | **否** |
| 6 | NFC | `mediatek,nfc` | i2c6 @0x08 | `st21nfc.c` | 复位脚需补 |
| 7 | 充电识别 | `xm,xmusb350-charger` | i2c11 @0x35 | `xmusb350-charger.c` | 否（私有 gpio 忽略） |
| 8 | 屏下指纹 | `goodix,goodix-fp` | spi1 @0 | `gf3658-spi.c` | 否 |
| 9 | 触控（骨架） | `goodix,gt9886` / `goodix,gt9896s` | i2c @0x5d | `touchscreen-gt9886.c` | 真机型号待确认 |
| 10 | 67W 电荷泵 | `ti,bq2597x-master` / `-slave` | i2c11 | **A 类补丁** | 否 |
| 11 | 电量计 | `ti,bq27z561_i2c0` / `_i2c9` | i2c | **A 类补丁** | 否 |

## 逐项属性与解析方式

### 1. WL2866D（无需改 dts）
父节点 `index = <0>`；每个 LDO 子节点属性：
`offset`（u16，0..3）、`regulator-name`、`regulator-min-microvolt`、
`regulator-max-microvolt`、`min-dropout-voltage`、`iout_ua`、`init-voltage`。
驱动按 `offset` 决定 DVDD（0/1，600mV 起 6mV/档）或 AVDD（2/3，1200mV 起 12.5mV/档），
注册 4 个 regulator 并在 probe 时写入 `init-voltage`。

### 2. AW2028（已修）
原厂写 `compatible = "awinic, aw2028_i2c"`，**逗号后有空格**，不符合 DT
compatible 规范（`vendor,model` 必须紧连）。交付 dts 已由
`tools/fix_mainline_contract.py` 的「compatible 逗号空格规范化」步骤统一修为
`"awinic,aw2028_i2c"`（幂等，重建不会再带回）；驱动 `of_match` 只保留
`"awinic,aw2028"` 与 `"awinic,aw2028_i2c"` 两条规范串，不再兜底带空格的写法。
节点无子节点，驱动默认注册 `aw2028:red/green/blue` 三个 led_classdev。

### 3. AW8697（复位脚需补）
`vib_mode`、`vib_f0_pre`、`vib_cont_drv_lev`、`vib_*` 等由驱动解析保存。
厂商写 `reset-gpio = <&pio 129 0>`、`irq-gpio = <&pio 22 0>` 是 MTK pio 三元组，
**主线 `gpiod` 无法解析**（`devm_gpiod_get_optional` 找的是 `reset-gpios`）。
建议在节点补：
```dts
reset-gpios = <&pio 129 GPIO_ACTIVE_HIGH>;
```

### 4. gamekey（必须补 GPIO）
厂商节点是**空的**：`gamekey { compatible = "xm,gamekey"; };`，没有任何 GPIO 属性。
驱动会注册 input 设备但打印警告、不产生按键事件。必须补（pin 号按真机原理图）：
```dts
&gamekey {
    key-gpios = <&pio 111 GPIO_ACTIVE_LOW>,
                <&pio 112 GPIO_ACTIVE_LOW>;
};
```
驱动用 `devm_gpiod_get_array_optional(dev, "key", GPIOD_IN)` + `gpiod_to_irq()`
双边沿中断上报 `KEY_F1..KEY_F4`（键码按 userspace keylayout 调整）。

### 5. ET7480（无需改 dts）
`pinctrl-names = "uart_enable", "uart_disable"` 两个状态由驱动
`pinctrl_lookup_state()` 取出，通过 sysfs `et7480_switch` 切换，**不猜寄存器**。

### 6. ST21NFC（复位脚需补）
厂商用数值 `gpio-rst = <10>` / `gpio-irq = <11>` 与
`gpio-rst-std = <&pio 10 0x0>`，主线无对应 gpio_chip，无法转成 `gpio_desc`。
驱动读取数值并记录日志，中断用 `client->irq`（由 `interrupts` 填充）。
要让复位生效需补 `reset-gpios = <&pio 10 GPIO_ACTIVE_HIGH>;`。

### 7. XMUSB350（无需改 dts）
`charger_name`、`alias_name`、`en_bc12`、`en_hvdcp` 由驱动解析；
`interrupts` 走 `client->irq` + `devm_request_irq`；私有
`xm,xmusb350_{rst,sda,scl}_gpio` 三元组被忽略（pinctrl 状态由 pinctrl 子系统处理）。

### 8. GF3658（无需改 dts）
`vfp-supply` 走 `devm_regulator_get_optional(dev, "vfp")`；
`netlink-event = <30>` / `power-type = <1>` 解析保存。
**TEE 采集、netlink 事件、厂商 HAL ioctl 语义未实现**（见 `docs/PORTED-DRIVERS.md`）。

### 9. GT9886（真机型号待确认）
`goodix,panel-max-x/y/w`、`goodix,irq-flags`、`vtouch-supply` 由驱动解析。
`goodix,reset-gpio = <&pio 20 0x0>` 是私有写法，标准 `reset-gpios` 缺失时跳过复位。

⚠️ **硬件版本冲突**：官方 R 版 dts 写 `goodix,gt9886`（GTX8 协议），
lineage 板级 dts 写 `focaltech,fts`（FT3658U）。同一份交付 dts 里两者都出现。
搬触控驱动前必须确认真机 IC（拆机或 `dmesg`），否则会选错驱动。

### 10/11. A 类补丁（0 新驱动）
主线 `bq25980_charger.c` / `bq27xxx_battery_i2c.c` 只需在 `of_match`
补 ares 的 compatible 别名，见 `compat/*.patch`。补丁已通过 `git apply --check`。
注意 bq2597x 与 bq25980 寄存器差异仍需真机验证。

## 不改 dts 的收益与代价

- **收益**：已验证 `mt6893-xiaomi-ares.dtb`（213,718 字节，含 `__symbols__`）
  保持不动，LK 叠加 dtbo 的链路不受影响。
- **代价**：私有 `*-gpio` 命名仍会产生 `dtbs_check` 警告（已由
  `gpio-normalize*.patch` 补标准 `*-gpios` 缓解）；gamekey / NFC 复位 /
  AW8697 复位在无补充属性时功能受限。AW2028 的 compatible 空格已修复，
  不再属于遗留代价。
