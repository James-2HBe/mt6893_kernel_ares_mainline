# ares 实机参数来源表（第 2 轮移植依据）

> 本轮所有驱动不再是占位/骨架，参数全部来自可追溯的 ares 官方源码与设备树。
> 「实机参数」= `MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`（小米官方开源，
> 代表量产机 BOM）+ 用户上传 lineage-23.2 板级 dts + 官方 `k6893v1_64_defconfig`。
> 本轮新抓取并归档到 `vendor-reference/official/headers/` 的 4 份私有头文件，
> 是"根据实机参数移植"的关键补齐。

## 1. 实机 BOM（官方 defconfig 决定性证据）

来自 `vendor-reference/official/k6893v1_64_defconfig`：

```text
CONFIG_TOUCHSCREEN_MTK_TOUCH="GT9886 GT9896S"     ← 触控 IC（悬案解决）
CONFIG_MTK_FINGERPRINT_SELECT="GF3658"            ← 指纹 IC
CONFIG_INPUT_AW8697_HAPTIC=y                       ← 触觉 IC
```

**结论**：量产机触控是 **Goodix GT9886 / GT9896S**，不是 lineage 板级 dts 里的
`focaltech,fts`（FT3658U）。后者属于另一硬件批次/社区适配。本轮触控驱动按
GT9886 官方协议实现；若真机确为 FT3658U 需关闭 `CONFIG_TOUCHSCREEN_GT9886`。

## 2. 本轮归档的官方私有头文件

| 文件 | 行数/字节 | 提供的实机参数 |
|---|---|---|
| `goodix_ts_core.h` | 767 行 / 20119 B | `GOODIX_TOUCH_EVENT 0x80`、`REQUEST 0x40`、PID `9886`/`9885`、`MAX_TOUCH 10`、`MAX_KEY 3`、`board_data` 结构 |
| `gf_spi_tee.h` | 267 行 / 6709 B | `GF_IOC_MAGIC 'g'`、18 个 `GF_IOC_*` 命令、`gf_ioc_transfer`/`gf_ioc_chip_info`/`gf_spi_cfg_t`、`gf_netlink_cmd` 枚举 |
| `aw8697.h` | 10 KB | `AW8697_CHIPID 0x97`、`VBAT` 阈值、`FF_EFFECT_COUNT_MAX 32`、trig 配置 |
| `aw8697_reg.h` | 25 KB | 全部 `0x00..0x7f` 寄存器 + `AW8697_BIT_*` 位定义 |

## 3. 触控 GT9886：真实寄存器与帧协议

来源 `GT9886_goodix_ts_i2c.c` + `goodix_ts_core.h`（均在 `drivers/src/`）：

| 项 | 实机值 | 来源 |
|---|---|---|
| I2C 地址 | `0x5d`，16 位大端寄存器地址 | 官方 dts `gt9886@5d`；`TS_ADDR_LENGTH=2` |
| 坐标寄存器 | `0x824E` | `TS_REG_COORDS_BASE` |
| 命令/请求/版本/配置 | `0x8040`/`0x8044`/`0x8240`/`0x8050` | `TS_REG_*` |
| PID / Doze | `0x4535` / `0x30F0`(stat `0x3100`) | `TS_REG_*` |
| 每点字节 | `8` | `BYTES_PER_COORD` |
| Doze 握手 | 写 `0xAA`→读 `0xBB`；使能 `0xCC` | 官方 `goodix_ts_enable` |
| 帧头 | `[0]=event(0x80 touch/0x40 request)` `[1]=coord_sta(bit3:0 num, bit4 key)` | `goodix_touch_handler` |
| 每点 | `[+0]=id(>=0x80 pen)` `[+1,+2]=x LE` `[+3,+4]=y LE` `[+5]=w` | 同上 2003-2053 行 |
| 帧尾 | `[2+8n]=key` `[3+8n]=checksum`（全帧 u8 和为 0） | 同上 1961-1980 行 |
| 板级 | `1080x2280`、10 点、`panel-max-w/p=256`、irq-flags=2(falling)、power-on/off=10000/5000us、key-map `<158 172 217>`、pen `<331 332>` | 官方 ares dts |

> 与主线 `goodix.c` 的差异：主线 coor `0x814E`、帧头 1 字节（status）、cfg
> `0x8050` contact_size=8。GT9886 多 1 字节 event 头、coor 不同 → **不能只加
> compatible 复用**，故按官方协议做独立驱动（真能上报触点）。

## 4. 指纹 GF3658：HAL 契约与 SPI 协议

| 项 | 实机值 | 来源 |
|---|---|---|
| DT | `spi1` `fingerprint@0`、8 MHz、`netlink-event=30`、`power-type=1`、`vfp-supply` | 官方 dts |
| ioctl | magic `'g'`，`GF_IOC_INIT..GF_IOC_SPI_INIT_CFG_CMD`（18 个） | `gf_spi_tee.h` |
| 事件 | `TEST=0 IRQ=1 SCREEN_OFF=2 SCREEN_ON=3` | 同头文件 `gf_netlink_cmd` |
| SPI 读 | 发 `0xF0 {addr_hi,addr_lo}`，再发 `0xF1` 收 `len+1` 字节（首字节 dummy） | `gf_spi_tee.c` `gf_spi_read_bytes_ree` |
| SPI 写 | 发 `0xF0 {addr_hi,addr_lo, data...}` | 同文件 `gf_spi_write_bytes_ree` |
| 真实寄存器 | `0x8140`(status) `0x4220`(version) `0x4010`(DSP) `0x4180`(SWRST) `0x40B0`(WDT) `0x404B`(cache) `0x8000`(FW sig) | 同文件 |
| 缓冲 | `25*1024` | 同文件 `bufsiz` |

> 注：官方代码里 `GF_NETLINK_ROUTE 29`，DT 写 `netlink-event 30`，二者不一致；
> 本轮用 `poll/read/write` 取代 netlink，此差异不再影响功能。

## 5. 触觉 AW8697：全寄存器与 vib_* 校准值

寄存器表来自 `aw8697_reg.h`（`ID 0x00`、`SYSST 0x01`、`SYSINT 0x02`、
`SYSCTRL 0x04`、`GO 0x05`、`CONT_CTRL 0x48`、`DRV_LVL 0x7b`、
`RAMADDRH 0x40`/`RAMDATA 0x42` 等）。初始化/GO 序列来自官方 `aw8697.c`。

DT `aw8697_haptic@5a` 的实机校准值（`vib_*`）：

```text
vib_f0_pre=1700  vib_f0_cali_percen=7  vib_f0_coeff=260
vib_cont_drv_lev=106  vib_cont_drv_lvl_ov=106  vib_cont_td=0xef
vib_cont_zc_thr=0x08f8  vib_cont_num_brk=3  vib_tset=0x1a
vib_r_spare=0x68  vib_sw_brake=0x2c  vib_bstdbg=<0x30 0xeb 0xd4 0 0 0>
vib_f0_trace_parameter=<9 3 1 31>  vib_bemf_config=<0 0x08 0x03 0xf8>
vib_effect_id_boundary=10  vib_effect_max=170
vib_bst_vol_default=0x10  vib_bst_vol_ram=0x11  vib_bst_vol_rtp=0x10
```

> 与主线 `aw86927.c`：CHIPID `0x97` vs `0x9270`(reg `0x57/0x58`)、play/wav/cont
> 寄存器 bank 不同 → **不同芯片，加 compatible 会 detect 失败**。本轮采用其
> 主线架构（`input/ff-memless`）搭配 AW8697 官方寄存器表。

## 6. AW2028 LED：与主线 aw2013 逐项一致

主线 `drivers/leds/leds-aw2013.c` 寄存器 = AW2028 实机寄存器：

```text
RSTR 0x00 (reset 0x55, chipid 0x33)   GCR 0x01 (enable BIT0)
LCTR 0x30   LCFG(x) 0x31+x   PWM(x) 0x34+x   LEDT0/1/2 0x37+3x   MAX 0x77
```

> 复用限制：主线 aw2013 需要 `vcc`/`vio` regulator + LED 子节点，而 ares dts
> 的 `aw2028@65` 是空节点且被注释（实机未贴装）→ 保留独立驱动做 fallback，
> 不谎称"加 compatible 即可"。

## 7. GPIO / 中断实机映射（DT 标准化依据）

主线 `pinctrl-mt6893.c` 存在，`pio` 节点 `#gpio-cells = <2>` → 厂商
`<&pio N flags>` 本就是合法 2-cell GPIO spec，缺的只是标准属性名。

| 设备 | 私有属性 | 标准属性（本轮补） | 实机 pin |
|---|---|---|---|
| GT9886 | `goodix,reset/irq-gpio` | `reset-gpios`/`irq-gpios` | 20 / 21 |
| GF3658 | `&goodix_fp` pinctrl + `fpc,gpio_irq` | `reset-gpios`/`irq-gpios` | 14 / 15 |
| AW8697 | `reset-gpio`/`irq-gpio` | `reset-gpios`/`irq-gpios` | 129 / 22 |
| ST21NFC | `gpio-rst-std`/`gpio-irq-std` | `reset-gpios`/`irq-gpios` | 10 / 11 |
| gamekey | `key_left/right` `hall_left/right` | `key-gpios`/`hall-gpios` | 111/112, 4/5 |
| XMUSB350 | `xm,xmusb350_{rst,sda,scl}_gpio` `mi,connect_therm` | `reset/irq/sda/scl/therm-gpios` | 28/110/205/204/29 |

> MTK 4-cell `interrupts = <pin type pin 0>` 与主线 `#interrupt-cells=2` 不兼容，
> 本轮所有驱动改用 `gpiod_to_irq(irq-gpios)`，不再依赖该属性（属性保留原样，
> 因注释会破坏 dtc 的 phandle 解析）。共补 15 个标准属性，见
> `port/dt-bindings/gpio-normalize.patch`。

## 8. 未能取得官方源码的部分（如实声明）

- **XMUSB350 寄存器映射**：小米未开源该驱动，仅 `xm,status-reg` 以 DT 属性
  可配（默认 `0x00`），并支持 IRQ GPIO 电平兜底检测，**不虚构寄存器地址**。
- **ET7480**：官方 dts 只有 pinctrl 状态，无寄存器定义，故按状态切换实现。
- **GT9886 固件/config bin**：官方 `goodix_cfg_bin.c` / `gtx8_update.c` 属固件
  升级路径，本轮驱动不含在线升级（不影响触控上报）。
- 无真机，地址/中断/电气/驱动 probe 全部未上电验证。
