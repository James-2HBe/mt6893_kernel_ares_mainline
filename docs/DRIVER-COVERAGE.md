# ares 驱动覆盖审计（主线 7.3.3）

> 回答一个问题：**"7.3.3 内核驱动都移植全了吗？"**
> 结论取决于"全"指什么。按「本轮承诺移植的那批」= 全了；按「整机所有硬件
> 都能跑」= 差得远，且缺口大多**不可移植**而非漏做。本文给出口径与逐项判定。

## 1. 审计口径

- 比对对象：交付板级 dts `mt6893-xiaomi-ares-official.dts` 的全部 `compatible`。
- 比对基准：**干净主线 tar**（`/workspace/linux-7.3.3.tar.xz` 解压出的 `drivers/`），
  排除安装脚本注入的 9 个移植驱动，否则会把它们误算成"主线自带"。
- 另计：标准无逗号 binding（`fixed-clock`、`regulator-fixed`、`simple-bus`、
  `shared-dma-pool`、`operating-points-v2`、`syscon` 等）。

> 曾被反复误判的两个坑：① grep 里把 `\|` 当字面量导致统计失真；
> ② 内核树已被注入驱动，必须用干净 tar。以下数字是修正后的。

## 2. 总表

| 类别 | 数量 | 说明 |
|---|---|---|
| 干净主线 7.3.3 直接覆盖 | **8** | `arm,gic-v3`、`arm,mali-valhall-jm`、`mediatek,generic-tphy-v2`、`fixed-clock`、`regulator-fixed`、`simple-bus`、`shared-dma-pool`、`operating-points-v2` |
| 本移植包已补（9 个驱动 + 4 条 compat 别名） | **13** | 见第 3 节 |
| 本轮新增适配 | **2** | GPU 节点（第 4 节）、相机 EEPROM 复用 at24（第 5 节） |
| 仍无驱动 | **28** | 见第 6 节，逐项判定 |

**关键区分**：主线自带的是 **SoC 基础控制器**，不是整机外设。干净树里 MT6893
相关只有 9 个文件（`pinctrl-mt6893.c`、`mtk-pm-domains.c`、`mtk_iommu.c`、
`mtk-smi.c`、`mtk-dvfsrc.c`、`mtk-dvfsrc-regulator.c`、`spi-mt65xx.c` 等）；
`drivers/clk/mediatek` **没有任何 mt6893 时钟驱动**，
`gpu/drm/mediatek`、`media/`、`gpu/drm/panfrost` 里也搜不到 mt6893。

## 3. 本移植包已补（13 条 compatible）

| 驱动 | compatible |
|---|---|
| 触控 GT9886 | `goodix,gt9886` / `goodix,gt9896s` |
| 触觉 AW8697 | `awinic,aw8697_haptic` |
| RGB LED | `awinic,aw2028_i2c` |
| 供电 WL2866D | `willsemi,wl2866d1` / `willsemi,wl2866d2` |
| NFC | `mediatek,nfc` |
| 指纹 SPI | `goodix,goodix-fp` |
| 充电 IC | `xm,xmusb350-charger` |
| 电荷泵（补到主线 bq25980） | `ti,bq2597x-master` / `ti,bq2597x-slave` |
| 电量计（补到主线 bq27xxx） | `ti,bq27z561_i2c0` / `ti,bq27z561_i2c9` |

共 9 个移植驱动 + 4 条主线驱动别名 = 13 条 compatible，**9/9 全了**，
都真编进过 `Image` / 出过 `.ko`。游戏肩键驱动（`KEYBOARD_ARES_GAMEKEY`）
对应的 `xm,gamekey` 节点受 cpp 批次选择器条件化，故不在此次静态清点结果内。

## 4. 本轮新增：GPU 节点（详见 `GPU-MAINLINE.md`）

`mediatek,mt6893-mali` 原先既不匹配 schema 也不匹配驱动。已：

- `/delete-node/ &mali;` 删除 SoC 层 4.14 私有节点，新建符合
  `^gpu@[a-f0-9]+$` 的 `gpu@13000000`（GIC_SPI 364/363/362、
  `clocks = <&clk26m>`、`mali-supply`、2 档 OPP）；
- 给 `arm,mali-bifrost.yaml` 补 `mediatek,mt6893-mali`；
- 强制 `CONFIG_DRM_PANFROST=y` 及 8 个 select 依赖。

**验证**：`dt-validate` 中 GPU / OPP 相关诊断 **0 行**，panfrost 目录实编译通过。

## 5. 本轮新增：相机 EEPROM 主线化，复用 at24（不移植重复驱动）

5 个 `mediatek,camera_eeprom` 节点是标准 I2C EEPROM（`reg` 0x50/0x51/0x52、
`status = "okay"`）。原厂 4.14 走 per-sensor OTP 解析栈，与主线模型不兼容。
按「与主线内核重复的驱动不用移植」原则，**直接把节点对齐主线 binding**：

```dts
mtk_camera_eeprom0: eeprom@51 {
	/* 原厂 mediatek,camera_eeprom（相机 OTP）；容量可用 size 覆盖 */
	compatible = "atmel,24c16";
	reg = <0x51>;
	status = "okay";
};
```

主线 `drivers/nvmem/at24.c` 的 `of_match` 含 `atmel,24c16`，内核运行时由
主线 at24 驱动接管并暴露 nvmem 设备。**不改内核源码、不加 compat 补丁**；
原厂串保留在行内注释里可追溯（官方原件在 `vendor-reference/` 也未改动）。
容量以 24c16 的 2KB 为保守默认，可在节点加 `size = <...>` 覆盖（`at24.c`
第 619–665 行支持 `size` / `pagesize` / `address-width` / `num-addresses`）。

> 由 `tools/fix_mainline_contract.py` 第 13 步完成，幂等；重建 dts 不会丢。
> `CONFIG_NVMEM=y` / `CONFIG_EEPROM_AT24=y` 已写入强制配置片段。
>
> **验证**：`dt-validate` 中 `camera_eeprom` / `at24` 相关诊断 **0 行**——
> 节点已完全满足 `eeprom/at24.yaml`（`$nodename: ^eeprom@..$` +
> 厂商白名单）。曾试过"只追加 `atmel,24c16` fallback"，反而因节点名与首串
> 不匹配白名单多出 218 行报错，故采用完全主线化。剩余 3 行含 `eeprom@`
> 字样的诊断来自 i2c 控制器节点 `mediatek,mt8192-i2c` 的既有
> `unevaluatedProperties` 告警（把子节点名列出），与 EEPROM 节点本身无关。

**关于 5 → 4**：源文件里是 5 个 `camera_eeprom` 节点，其中 `i2c4` 总线下
`camera_eeprom1@51` 与 `camera_eeprom4@51` **同为 I2C 地址 0x51**。主线化后
两者都叫 `eeprom@51`，dtc 按 DT 规则在同一父节点下合并同名节点，因此交付
dtb 里是 **4 个 EEPROM 设备**（i2c2@51、i2c4@51、i2c8@52、i2c8@50）。
物理上同一 I2C 地址不可能挂两个设备，原厂是靠不同节点名绕过 DT 检查，
合并反而修正了这处数据冗余，功能等价。若真机确认是两颗不同芯片，需要
先核实实际地址再拆分，不能只改节点名。

## 6. 仍无驱动的 28 项：逐类判定

| 分类 | 项 | 判定 |
|---|---|---|
| **保留内存 / CMA（本就不是驱动）** | `mediatek,reserve-memory-{mcupm,sspm,adsp,scp}_share`、`mediatek,consys-reserve-memory`、`mediatek,gps-reserve-memory`、`mediatek,ion-carveout-heap`、`mediatek,zone_movable_cma`、`modem,testing-mode` | 纯 reserved-memory / 数据节点，不需要驱动 |
| **显示 / 背光 / 面板** | `mediatek,lcd-backlight`、`button-backlight`、`keyboard-backlight`、`jogball-backlight`、`k10_42_02_0a_dsc_cmd,lcm`、`mediatek,phy_tuning` | 依赖 `mtkfb` + DSI + 闭源面板栈；主线 `mediatek-drm` 只到 mt8195，无 MT6893 mmsys |
| **MT6360 PMIC 私有 MFD** | `mediatek,mt6360_pmu_{core,chg,fled,rgbled}` | 原厂是**纯 DT 容器**（无 `compatible`、无 I2C `reg`）+ 子节点；主线 `mt6360-core.c` 是 I2C 驱动 + `MFD_CELL_OF` 动态建子设备。结构不兼容，补别名无效 |
| **LED 子节点** | `mediatek,red` / `green` / `blue` | 私有 `leds-mtk-*` 栈子节点；本包 `aw2028` 已覆盖 RGB LED 功能 |
| **相机 ISP** | `mediatek,imgsensor` | 43/44 处 imgsensor 节点，闭源 sensor 栈 + Android HAL |
| **需 PWM 控制器** | `mediatek,irtx-pwm`（红外）、`mediatek,vibrator` | 主线有 `pwm-ir-tx` / `pwm-vibra`，但需要 `pwms` phandle；SoC dtsi 的 `mediatek,pwm` 控制器**主线不认**（`pwm-mediatek.c` 只有 `mediatek,mtXXXX-pwm`），链路断在上游。振动功能本包 AW8697 已覆盖 |
| **其它** | `focaltech,fts`（备选触控批次）、`microtrust,shared_mem`（TEE）、`mediatek,usb_c_pinctrl` | 备选批次已有 GT9886 覆盖；TEE / 私有 pinctrl 闭源 |

**没有一类是"漏做"**：要么不是驱动，要么依赖闭源栈，要么上游控制器缺失，
要么已被本包等价驱动覆盖。

## 7. 结论

> **"11 阶段全绿、Image 编出来了"证明的是构建链路通，不是驱动齐。**
> 交付 dts 里 61 个 `status = "okay"` 节点中，多数 `compatible` 没有驱动，
> 内核会静默跳过——不崩，但功能就是没有。这也是 `dtbs_check` 大量
> "no binding" 告警的来源。

原厂 4.14 的 GPU / 相机 / 显示走 Mali DDK、GED、`mtkfb`、imgsensor 闭源栈，
依赖 Android HAL 和私有 sysfs，属于**不能移植**而非漏做。

## 8. 复现

```bash
# 覆盖审计（干净主线 tar 解压到 /tmp/clean-kmods 后）
python3 - <<'PY'
# 见本文件第 1 节口径；提取 dts 全部 compatible 与干净树 drivers/ 的 of_match 做差集
PY

# 重建 dts（会自动带回 GPU 节点与 camera_eeprom fallback，幂等）
python3 tools/fix_mainline_contract.py <输入.dts> <输出.dts>

# 校验
dt-validate -s <processed-schema.json> -l arm,mali-bifrost <board>.dtb
```
