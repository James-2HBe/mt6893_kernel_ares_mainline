# 第 2 轮：让"移植失败/受限"的驱动适配主线 7.3.3

> 目标：对第 1 轮判定为「无法等价移植 / 只给骨架 / 寄存器占位 / pio 无法解析」
> 的驱动，**换一种方式**使其按 k40gamming（ares）**实机参数**适配主线 7.3.3。

## 一句话结论

第 1 轮走的是「把 4.14 私有驱动重写成主线 API」，遇到 3 类死结；第 2 轮换成
**「扩展/复用主线已有驱动 + 完整移植官方 HAL 契约 + 标准化 DT GPIO 契约」**，
并用刚抓取的 4 份官方私有头文件把占位全部换成实机值。**9 个驱动全部严格
交叉编译通过（0 error / 0 warning / 9 个 .ko），标准化 dts 编译通过。**

## 第 1 轮的三个死结 → 第 2 轮的解法

| 死结（第 1 轮） | 根因复核 | 第 2 轮的"其他方式" |
|---|---|---|
| GT9886 触控只给框架骨架，帧协议留空 | 官方私有协议 + 缺 `goodix_ts_core.h`，主线 goodix 是 GT9xx/GT9916 | 抓到官方头文件 + 官方 i2c 源码，**按真实 GTX8 帧协议实现可上报触点的主线驱动**；官方 defconfig 确认实机就是 GT9886 |
| GF3658 指纹只做 SPI 透传，netlink/TEE 未实现 | 主线无指纹子系统，采集在 TEE TA | **完整移植内核侧 HAL 契约**：`/dev/goodix_fp` + 18 个 GF_IOC_* + 官方 REE SPI 协议(0xF0/0xF1) + `poll/read/write/fasync` 取代 netlink + 标准 gpiod/regulator。TEE 采集本就不属内核 |
| pio 三元组无法解析（gamekey/NFC/AW8697 阻断） | 以为主线没有 MT6893 pinctrl | 主线**有** `pinctrl-mt6893.c`，`pio` 是 `#gpio-cells=2` → `<&pio N 0>` 本就是合法 GPIO spec。只需**补标准 `*-gpios` 属性名**（15 个，按实机 pin），驱动改 `gpiod_to_irq()` |
| AW8697 用 leds-class + 寄存器占位 | 怕 `CONFIG_INPUT_FF_MEMLESS=m` 缺符号 | 完整符号表已生成，`input_ff_create_memless` 可用 → **改标准 `input/ff-memless`** + 官方 `aw8697_reg.h` 全寄存器 + 实机 `vib_*` 校准值 |
| AW2028 寄存器"占位" | 以为要按 datasheet 猜 | 逐项核对后**与主线 `leds-aw2013.c` 完全一致**，补 chip-id 校验，占位说法作废 |

## 9 个驱动（第 2 轮状态）

| 驱动 | 主线框架 | 第 2 轮改进 | 实机依据 |
|---|---|---|---|
| `touchscreen-gt9886.c` | i2c + input-mt + gpiod + regulator | **骨架 → 可用**：真实 0x824E 帧、8B/点、checksum、pen/key、doze 握手 | 官方 i2c 源码 + core.h + ares dts |
| `gf3658-spi.c` | spi + miscdevice + cdev + poll/fasync | **透传 → 完整 HAL 契约**：18 ioctl + compat_ioctl + 官方 SPI 协议 | gf_spi_tee.h/.c + ares dts |
| `haptic-aw8697.c` | regmap + **input/ff-memless** | 占位 → 官方 25KB 寄存器表 + 17 项 vib_* 校准 | aw8697_reg.h/.c + ares dts |
| `leds-aw2028.c` | regmap + led_classdev | 补 0x33 chip-id + 主线 aw2013 全时序寄存器 | 主线 aw2013（已核对一致） |
| `input-gamekey.c` | platform + input + gpiod | **阻断解除**：实机 pio111/112 + pio4/5 | lineage dts `&gamekey` |
| `st21nfc.c` | i2c + miscdevice + poll | **阻断解除**：reset/irq = pio10/11，gpiod_to_irq | 官方 dts `gpio-*-std` |
| `xmusb350-charger.c` | power_supply + gpiod | **阻断解除**：pio28/205/204/29/110 + en_* 标志；寄存器 DT 可配 | 官方 ares-mt6893.dtsi |
| `et7480-audioswitch.c` | i2c + pinctrl + sysfs | 保持（无寄存器定义，按状态切换） | 官方 dts |
| `regulator-wl2866d.c` | regmap + regulator | 保持（第 1 轮已用官方真实寄存器） | 官方 wl2866d.c |

## "复用主线驱动"路径的诚实结论

- ✅ **可行但未采用**：AW2028 与主线 `leds-aw2013.c` 寄存器一致。但主线驱动要求
  `vcc`/`vio` regulator + LED 子节点，ares 的 `aw2028@65` 是空节点且被注释
  （实机未贴装），加 compatible 仍无法 probe → 保留独立驱动做 fallback。
- ❌ **不可行（不糊弄）**：
  - GT9886 ≠ 主线 `goodix.c`：coor `0x824E` vs `0x814E`、帧多 1 字节 event 头，
    只加 compatible 会读到错位数据。
  - AW8697 ≠ 主线 `aw86927.c`：CHIPID `0x97` vs `0x9270`、play/wav/cont bank 不同，
    `aw86927_detect()` 会直接 `-ENODEV`。
  - GF3658：主线无任何指纹子系统，无"已有驱动"可复用。
  → 三者均采用「主线框架 + 官方真实参数」独立实现，而非伪造复用。

## DT 契约标准化（`port/normalize_dt_gpios.py`）

非破坏性、幂等、按 compatible 去重；生成
`mt6893-xiaomi-ares-mainline.dts` + `port/dt-bindings/gpio-normalize.patch`：

- 6 个设备节点补 15 个标准 `*-gpios`（值 = 同一 `<&pio N flags>`）；
- 保留厂商私有属性与 4-cell `interrupts` 原样（驱动已不依赖后者）；
- 缩进对齐，补丁可读；重复运行不变化。

## 编译验证

| 项 | 结果 |
|---|---|
| 驱动严格编译 `make LLVM=1 ARCH=arm64 M=port modules` + 完整 `Module.symvers`(21620 符号) | **9 个 .ko，0 error / 0 warning，MODPOST 全解析** |
| 标准化 dts `cpp`+`dtc -@` | exit 0，dtb 214165 字节，108 警告（均为厂商 dtsi 历史告警） |
| 内核 Makefile `make ARCH=arm64 mediatek/mt6893-xiaomi-ares-mainline.dtb` | 成功，`__symbols__` 存在 |
| 反编译回读 | 9065 行；`__symbols__`/`xiaomi,ares`/6 类 `*-gpios`/全部 compatible 命中；**chopin 残留 0** |

编译中修复的真实 7.3.3 API 漂移：`enable_irq/disable_irq` 变 `void`、
`FB_EVENT_BLANK` 被移除（改用 userspace write 注入屏状态）、
`copy_from_user` 目标尺寸编译期检查（`__bad_copy_to`）。

## 接入方式

```bash
./port/integration/apply.sh <kernel> --dry-run   # 预览（含 mainline dts）
./port/integration/apply.sh <kernel>             # 复制 9 个 .c + 标准化 dts + Kconfig/Makefile
cd <kernel> && git apply <交付>/port/compat/*.patch     # A 类 compatible
cat port/integration/defconfig.additions >> arch/arm64/configs/defconfig
make ARCH=arm64 defconfig && make ARCH=arm64 modules
make ARCH=arm64 mediatek/mt6893-xiaomi-ares-mainline.dtb
```

**起机用 `mt6893-xiaomi-ares-mainline.dtb`**（含标准 GPIO 契约）；
旧的 `mt6893-xiaomi-ares.dtb` 保留作对照，但不含 GPIO 契约，驱动只能降级 probe。

## 仍需真机 / 已知限制（如实声明）

1. 无真机：probe / 中断 / 电气 / I2C-SPI 应答全部未上电验证，只做到编译通过。
2. **XMUSB350 寄存器映射官方未开源**：`xm,status-reg` 默认 `0x00` 可配，
   IRQ GPIO 电平兜底；不虚构地址。
3. **触控 IC 需确认真机批次**：官方 defconfig = GT9886；lineage dts 写 FT3658U。
   若是后者，关闭 `CONFIG_TOUCHSCREEN_GT9886`。
4. GT9886 未含在线固件/config-bin 升级（不影响触点上报）；GF3658 的 TEE 采集
   在 TA 内，HAL 需把 netlink 监听改为 `poll/read(/dev/goodix_fp)`。
5. 4-cell `interrupts` 属性保留（主线内核 `of_irq_get` 会失败，但驱动用
   `gpiod_to_irq(irq-gpios)`，不受影响）；若追求 dtbs_check 干净可另行清理。
6. 6 个最小桩（`irtx_pwm`/`bat_gm30`/`mmc0_pins_*`/`nfc_clk_req_*`）仍源自
   编译期 DWS 生成的 `cust.dtsi`，需真机 `/proc/device-tree` 替换。
7. 官方完整内核快照因 GitHub 限速未全量下载；本轮 4 份私有头 + 全部参数均
   通过 raw 精准抓取并已归档，可离线复现。
