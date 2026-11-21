# ares B 类驱动移植（主线 7.3.3）— 第 1 轮成果与验证

> ⚠️ **历史文档（第 1 轮）**：本文件记录的是第一次「把私有源码重写成主线
> API」的成果。其中标注的「GT9886 骨架 / 寄存器占位 / pio 需补 GPIO /
> haptic 用 leds-class」等状态**已在第 2 轮全部改变**。当前有效状态请以
> **`docs/PORT-ROUND2.md`** 与 **`docs/REAL-HW-PARAMS.md`** 为准；第 1 轮
> 的骨架/占位版源码保留在 `port/archive/` 供对照。
>
> 承接上一阶段「只归档官方 4.14 私有源码」的工作，本次把主线缺失的驱动
> **真正做成可接入、可编译的内核驱动**。
>
> 原则（沿用你的要求）：**仓库中已有且真实可用的不重复移植**；
> 不把 MTK 4.14 私有框架拖进 7.3.3。

## 一、覆盖范围

上一版 `docs/DRIVER-PORTING.md` 把 ares 外设分成 A（主线已有，8 个）、
B（主线缺失，11 个）、C（厂商框架数据节点）三类。本次处理：

| 类别 | 数量 | 本次动作 |
|---|---|---|
| **A 类** | 8 | 不写新驱动；`ares_defconfig.fragment` 已开开关，另给 2 个主线驱动补 ares compatible 补丁 |
| **B 类** | 11 | 9 个写成可编译驱动（含 1 个骨架、1 个功能受限），2 个由 A 类补丁覆盖，1 个给出阻断结论 |
| **C 类** | 其余 | 数据节点，无独立驱动，不处理（与 chopin 现状一致） |

### B 类 11 项的最终归属

| # | 设备 | 归属 | 交付文件 |
|---|---|---|---|
| 1 | 触控 GT9886 / FT3658U | **框架骨架 + 阻断结论** | `port/touchscreen-gt9886.c` |
| 2 | 屏下指纹 GF3658 | **功能受限驱动**（SPI 透传，无 TEE） | `port/gf3658-spi.c` |
| 3 | 线性马达 AW8697 | 新驱动 | `port/haptic-aw8697.c` |
| 4 | 双电荷泵 bq2597x | **A 类补丁**（复用 bq25980） | `port/compat/bq25980-*.patch` |
| 5 | 电量计 bq27z561 | **A 类补丁**（复用 bq27xxx） | `port/compat/bq27xxx-*.patch` |
| 6 | 相机 LDO WL2866D ×2 | 新驱动（寄存器最完整） | `port/regulator-wl2866d.c` |
| 7 | 音频开关 ET7480 | 新驱动（pinctrl 实现，不猜寄存器） | `port/et7480-audioswitch.c` |
| 8 | LED AW2028 | 新驱动 | `port/leds-aw2028.c` |
| 9 | USB 充电识别 XMUSB350 | 新驱动 | `port/xmusb350-charger.c` |
| 10 | 弹出式肩键 gamekey | 新驱动 | `port/input-gamekey.c` |
| 11 | ST21NFC | 新驱动（保留 HAL 直通语义） | `port/st21nfc.c` |

## 二、编译验证（真实 arm64 交叉编译，非语法检查）

环境：`make LLVM=1 ARCH=arm64 M=<port> -C /workspace/linux-7.3.3 modules`
（clang 18 `--target=aarch64-linux-gnu`，内核 7.3.3 已 `modules_prepare` / `vmlinux`）。

| 项 | 结果 |
|---|---|
| CC 阶段（语法 / 类型 / 内核 API 检查） | **9 个驱动全部通过，0 error** |
| 链接 + **严格 modpost** | `make LLVM=1 ARCH=arm64 M=port modules`：**exit 0，0 error，0 warning** |
| 产物 | **9 个 `.ko`**，全部通过 modpost 符号解析 |
| 符号表 | 内核树已 `make vmlinux` + `make modules` 生成 `Module.symvers`（21,620 行），**未降级** |

`port/build.sh` 在符号表缺失时会自动把 modpost 降级为警告（CC 阶段始终是完整检查），
符号表齐备后自动转为严格模式 —— **本次最终验证走的是严格模式**，
既无编译错误也无未定义符号。

## 三、逐个驱动的重写策略与可信度

| 驱动 | 官方 4.14 源码 | 剥离的私有依赖 | 主线框架 | 硬件协议可信度 |
|---|---|---|---|---|
| `regulator-wl2866d` | ✅ 完整 691 行 | cdev/class/sysfs 调试口、4.14 双参 probe、面板特判、`simple_strtoul`、父电源投票 | regmap + `devm_regulator_register` | **高**：寄存器/电压换算直接来自官方 |
| `et7480-audioswitch` | ❌ 无独立驱动 | MTK accdet 框架 | i2c + pinctrl 状态 + sysfs | **中高**：不猜寄存器，靠 DT pinctrl 表达 |
| `leds-aw2028` | ❌ 未归档 | 厂商 HAL | regmap + 3× `led_classdev` | 中：寄存器参考同族 `leds-aw2013.c`，**需按 datasheet 校准** |
| `haptic-aw8697` | ✅ 83KB（缺 4 个私有头） | `aw8697*.h`、`ringbuffer.h`、pm_qos、of_gpio、4.14 power_supply | `led_classdev`（brightness = 强度） | 中：DT 参数解析完整，寄存器占位 |
| `xmusb350-charger` | ❌ 未归档 | MTK charger 框架 | `power_supply` + 中断 + work | 中：psy 框架完整，寄存器占位 |
| `input-gamekey` | ❌ 未归档 | MTK gamekey 框架 + 霍尔 HAL | platform + input + gpiod | 中：框架完整，需补 `key-gpios` |
| `st21nfc` | ✅ 39KB（缺 `st21nfc.h`） | `of_gpio.h`、数值 GPIO、LEGACY 分支、私有 ioctl | i2c + `miscdevice` + poll + gpiod | 中：保留 `/dev/st21nfc` 直通语义 |
| `gf3658-spi` | ✅ 62KB | `teei_fp.h`/`tee_client_api.h`/`cpu_ctrl.h`/`mtk_spi_hal.h`、TEE TA、netlink(30) | spi + `miscdevice` + regulator | **低**：仅 SPI 透传，采集/TEE 事件未实现 |
| `touchscreen-gt9886` | ✅ 4 个 .c（缺 4 个私有头） | `goodix_ts_core.h`/`goodix_cfg_bin.h`/`goodix_default_fw.h`/`mtk_panel_ext.h`、GTX8 固件协议 | i2c + input-mt + regulator | **骨架**：帧解析留空并打印警告，默认关闭 |

### 为什么 GT9886 / GF3658 无法等价移植

- **GT9886**：主线 `drivers/input/touchscreen/goodix.c` 是 GT9xx 协议、
  `goodix_berlin_*` 是 GT9916 协议，都与 GTX8 不同；官方 4.14 驱动依赖
  4 个未开源头文件，且触点帧 / 固件升级是私有协议。本交付只给框架骨架，
  **不谎称触控可用**。
- **GF3658**：官方驱动把指纹采集放在 **TEE TA** 里，内核侧只是 SPI 透传 +
  netlink 事件 + HAL ioctl。主线没有该 TA、没有 GF netlink 协议，搬内核代码
  无法工作；必须连 Android TEE TA 与厂商 HAL 一起搬，超出内核移植范畴。
- **真机型号未定**：官方 R 版 dts 写 `goodix,gt9886`，lineage 板级 dts 写
  `focaltech,fts`（FT3658U），两者是不同触控 IC。搬驱动前需拆机 / `dmesg` 确认。

## 四、A 类补丁（0 新驱动）

`port/compat/` 下两个补丁已通过 `git apply --check`：

| 补丁 | 主线文件 | 新增 compatible |
|---|---|---|
| `bq25980-add-ares-compatible.patch` | `drivers/power/supply/bq25980_charger.c` | `ti,bq2597x-master` / `ti,bq2597x-slave` |
| `bq27xxx-add-ares-compatible.patch` | `drivers/power/supply/bq27xxx_battery_i2c.c` | `ti,bq27z561_i2c0` / `ti,bq27z561_i2c9` |

主线已内置 `ti,bq27z561` / `BQ27Z561` 芯片定义，补别名即可复用；
bq2597x 与 bq25980 属同族但寄存器差异需真机验证。
补丁由 `port/make_compat_patches.py` 生成，**不修改内核树**。

## 五、接入内核树

```bash
# 1) 复制驱动 + 追加 Makefile / Kconfig（幂等，可 --dry-run 预览）
./port/integration/apply.sh /path/to/mt6893_kernel_ares_mainline --dry-run
./port/integration/apply.sh /path/to/mt6893_kernel_ares_mainline

# 2) 追加配置开关
cat port/integration/defconfig.additions >> <kernel>/arch/arm64/configs/defconfig
#    （A 类开关见上层交付的 ares_defconfig.fragment）

# 3) 应用 A 类 compatible 补丁
cd <kernel> && git apply /workspace/ares-7.3.3/port/compat/*.patch

# 4) 重新生成配置并编译
make ARCH=arm64 defconfig
make ARCH=arm64 modules
```

配套文件：
- `port/integration/Kconfig.fragment` — 9 个 Kconfig 条目
- `port/integration/Makefile.rules` — 各子系统 Makefile 追加规则
- `port/integration/defconfig.additions` — defconfig 开关（GT9886 骨架默认关）
- `port/dt-bindings/VENDOR-CONTRACT.md` — 11 个设备的 DT 契约、哪些 dts 必须改

## 六、已知限制（务必读）

1. **无真机**：所有驱动只做到「真编译通过」，probe 是否成功、中断 /
   电气参数、I2C 应答均**未上电验证**。
2. **寄存器占位**：AW2028 / AW8697 / XMUSB350 / GT9886 的 `*_REG_*`
   宏是占位值，集中在各文件头部，必须按 datasheet 校准。WL2866D 例外
   （寄存器来自官方源码）。
3. **MTK `pio` 三元组无法解析**：厂商 dts 大量使用 `<&pio 129 0>` 私有
   写法，主线 `gpiod` / `gpiod_to_irq` 需要标准 `*-gpios`。
   gamekey、NFC reset、AW8697 reset 因此在现有 dts 上功能受限，
   具体补充片段见 `dt-bindings/VENDOR-CONTRACT.md`。
4. **GT9886 骨架默认关闭**，GF3658 不含 TEE 采集；两者都不能让对应硬件工作。
5. **不追求 dt-bindings 合规**：厂商私有 compatible（含
   `awinic, aw2028_i2c` 的空格）会让 `dtbs_check` 大量报错，本交付以
   dtb 生成成功 + 驱动可 probe 为目标。
6. **haptic 用 leds-class 而非 input/ff-memless**：当前基线
   `CONFIG_INPUT_FF_MEMLESS=m`，外部模块严格 modpost 会缺符号；
   若需标准 force-feedback 接口，打开 `=y` 后改用
   `input_ff_create_memless()` 即可。

## 七、建议的落地顺序

1. **先起串口**（主线 `8250_mtk` 补 MT6893 compatible），这是唯一可靠调试通道。
2. **WL2866D + ET7480**：寄存器/行为最确定，先验证 I2C 总线与 pinctrl 通。
3. **A 类补丁 + `ares_defconfig.fragment`**：充电 / 电量计 / PMIC 先工作。
4. **补 3 处 DT GPIO 标准属性**（gamekey / NFC reset / AW8697 reset），
   再验证这三个驱动。
5. **最后处理触控与指纹**：确认真机 IC 型号 → 补齐官方头文件或 datasheet
   → 重写帧协议；TEE TA 需单独评估是否值得搬。
