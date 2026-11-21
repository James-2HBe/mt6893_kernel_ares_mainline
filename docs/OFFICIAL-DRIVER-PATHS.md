# 官方 4.14 ares 私有驱动源码路径表

> 数据源：`MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`
> 用途：B 类（主线 7.3.3 缺失）驱动搬移时，在本地 clone 官方内核后按此表取源码。
> ✅ = 已成功抓取并归档到 `drivers/src/`；⚠️ = 未归档，需本地 clone 补齐。

## 归档实况（`drivers/src/`，11 个文件 / 384,504 字节）

为便于扁平化归档，文件名加了来源前缀，与官方原名对应关系如下：

| 归档文件 | 官方原名 / 路径 | 字节 |
|---|---|---|
| `GT9886_goodix_ts_i2c.c` | `GT9886/goodix_ts_i2c.c` | 60,379 |
| `GT9886_goodix_ts_core.c` | `GT9886/goodix_ts_core.c` | 55,685 |
| `GT9886_goodix_cfg_bin.c` | `GT9886/goodix_cfg_bin.c` | 20,282 |
| `GT9886_goodix_gtx8_update.c` | `GT9886/goodix_gtx8_update.c` | 38,289 |
| `GT9886_Makefile` | `GT9886/Makefile` | 355 |
| `gf3658_gf_spi_tee.c` | `fingerprint/goodix/gf_spi_tee.c` | 62,572 |
| `gf3658_gf_spi_factory.c` | `fingerprint/goodix/gf_spi_factory.c` | 762 |
| `aw8697.c` | `aw8697_haptic/aw8697.c` | 83,200 |
| `aw8697_ringbuffer.c` | `aw8697_haptic/ringbuffer.c` | 5,335 |
| `st21nfc.c` | `nfc/st21nfc/st21nfc.c` | 39,289 |
| `wl2866d.c` | `regulator/wl2866d.c` | 18,356 |

**未归档的 3 项**（非必需或抓取不全）：
- `goodix_ts_tui.c`：官方 Makefile 为 `obj-n`，未参与编译；曾抓到 1286 字节疑似不完整，已在清理时移除。
- `aw8697_haptic/Makefile`、`fingerprint/goodix/Makefile`：内容已用于反推 `obj-y` 的 `.o` 名单，本体可在本地 clone 后直接取得。

---

## 触控（`CONFIG_TOUCHSCREEN_MTK_TOUCH="GT9886 GT9896S"`）
实际是 **Goodix GT9886**（非 focaltech），路径 `drivers/input/touchscreen/GT9886/`：

| 文件 | 说明 | 状态 |
|---|---|---|
| `goodix_ts_i2c.c` | I2C 传输层 | ✅ 60KB |
| `goodix_ts_core.c` | 核心逻辑 | ✅ 55KB |
| `goodix_cfg_bin.c` | 配置解析 | ✅ 20KB |
| `goodix_gtx8_update.c` | 固件升级 | ✅ 38KB |
| `goodix_ts_tui.c` | TUI（`obj-n`，未编译） | ⚠️ 未归档（非必需，见上） |
| `Makefile` | 编译规则 | ✅（归档为 `GT9886_Makefile`） |

> 注意：ares 设备树 compatible 写作 `focaltech,fts`，与官方 GT9886 (Goodix) 不一致——
> 说明第三方 lineage-23.2 板级 dts 用了 FT3658U 触控，而官方 R 版用 GT9886。
> 两者是不同硬件版本，搬驱动前需先确认真机触控 IC 型号（拆机 / `dmesg`）。

## 屏下指纹（`CONFIG_MTK_FINGERPRINT_SELECT="GF3658"`）
路径 `drivers/input/fingerprint/goodix/`，Makefile: `gf_spi_tee.o gf_spi_factory.o`：

| 文件 | 说明 | 状态 |
|---|---|---|
| `gf_spi_tee.c` | 主驱动（SPI + TEE 通路） | ✅ 62KB |
| `gf_spi_factory.c` | 工厂测试 | ✅ 762B |
| `Makefile` | 编译规则 | ⚠️ 内容已用于反推 `.o` 名单，本体未归档 |

## 线性马达 AW8697（`CONFIG_INPUT_AW8697_HAPTIC=y`）
路径 `drivers/input/misc/aw8697_haptic/`，Makefile: `aw8697.o ringbuffer.o`：

| 文件 | 说明 | 状态 |
|---|---|---|
| `aw8697.c` | 主驱动 | ✅ 83KB |
| `ringbuffer.c` | 波形缓冲 | ✅ 5.3KB（归档为 `aw8697_ringbuffer.c`） |
| `Makefile` | 编译规则 | ⚠️ 内容已用于反推，本体未单独归档 |

## 相机 LDO（WL2866D ×2）
路径 `drivers/regulator/wl2866d.c` ✅ 18KB（同 SoC，regulator 驱动，最易移植）

## NFC（ST21NFC）
路径 `drivers/misc/mediatek/nfc/st21nfc/st21nfc.c` ✅ 39KB

## 未抓到 / 需本地补的（官方快照下载被 GitHub 限速中断）

| 设备 | defconfig 线索 | 建议搜索路径 |
|---|---|---|
| 67W 电荷泵 bq2597x | `CONFIG_MTK_CHARGER=y` | `drivers/power/supply/mtk_charger*` 或 `drivers/misc/mediatek/charger/` |
| 电量计 bq27z561 / bat_gm30 | `CONFIG_MTK_GAUGE_VERSION=30` | `drivers/power/supply/mtk_gauge*` / `drivers/misc/mediatek/gauge/` |
| 音频开关 ET7480 | `mediatek,et7480-audioswitch` | `sound/soc/codecs/` `drivers/misc/mediatek/audio/` |
| LED AW2028 | `awinic,aw2028_i2c` | `drivers/leds/` `drivers/misc/mediatek/leds/` |
| USB 充电识别 XMUSB350 | `xm,xmusb350-charger` | `drivers/power/supply/` `drivers/misc/mediatek/` |
| 弹出式肩键 gamekey | GPIO 111/112 + 霍尔 4/5 | `drivers/misc/mediatek/` `drivers/input/keyboard/` |

> 建议：`git clone --depth 1 -b ares-r-oss https://github.com/MiCode/Xiaomi_Kernel_OpenSource.git`
> 后 `grep -rIl "<compatible 字符串>" drivers/` 即可精确定位全部驱动。
> 但按 `DRIVER-PORTING.md` 的结论，bq2597x / bq27z561 / gamekey 更应走 **A 类**
> （复用主线驱动 + 补 compatible），不必搬 MTK 私有框架。

## 为什么不直接把这些源码编译进 7.3.3
它们是 4.14 MTK 私有驱动，依赖 `drivers/misc/mediatek/include/mt-plat/` 私有头、
`power_supply` / `regulator` / `input` / `i2c` 的 4.14 API、以及私有 DT binding。
详见 `DRIVER-PORTING.md` 第三节「共性障碍」。
