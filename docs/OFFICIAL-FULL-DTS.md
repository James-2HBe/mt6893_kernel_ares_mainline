# 从原厂仓库全量移植 ares 设备树到主线 7.3.3

> 目标：从 **小米官方内核仓库** `MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss`
> 把 k40g（ares / MT6893）**编译链所需的全部设备树文件**移植到 **Linux 主线 7.3.3 内核**。

---

## 一句话结论

官方仓库里 ares 编译链共 **41 个文件**（26 个 `.dts/.dtsi`、1 个 `.dws`、12 个私有头、
`defconfig` + `Makefile`），**已全部下载、归档并安装进主线内核树**；
在此基础上生成的 `mt6893-xiaomi-ares-official.dts` 已接入内核 Makefile 并编译出 dtb，
且把之前遗留的 **6 个「最小桩」全部换成官方真值或按官方 dtsi 删除**。

---

## 关键发现（改变了移植结果）

在此之前只归档了 23 份「按需挑选」的官方文件，并因 `k6893v1_64/cust.dtsi`
（编译期生成、源码仓库 404）打了 6 个空桩。这次用 GitHub tree API 拿到
**70,264 条目的完整仓库清单**，发现：

| 发现 | 影响 |
|---|---|
| `drivers/misc/mediatek/dws/mt6885/k6893v1_64.dws`（2591 行）存在 | 这是 `cust.dtsi` 的**唯一上游来源**，227 个 GPIO + eint/i2c/pmic/clk 模块全在里面 |
| `bat_setting/mt6893_battery_{prop,table,prop_dim2_ext}.dtsi`（77/537/77 行） | `bat_gm30` 的**官方真实定义**（含 537 行充放电曲线表），此前完全漏掉 |
| `mt6893.dts` L7475 有 `irtx_pwm` 真实节点 | 另一个桩可直接换成官方真值 |
| 官方 `xiaomi-mt6893-common.dtsi` L60/L61、L893/L895 | `mmc0_pins_*`、`nfc_clk_req_*` 的引用**本来就被注释**，4 个 pinctrl 桩属死引用，直接删即可 |
| `CONFIG_MTK_GAUGE_VERSION=30` 在官方 defconfig 里为真 | 之前电池节点段因未定义该宏被 cpp 跳过 —— 这是漏掉电池数据的根因 |
| 官方 include 链实际用 `cust_mt6893_touch_1080x2300.dtsi` | GT9886 实机分辨率 **1080×2280**（不是另一批次的 2340），现有参数是对的 |

另外补齐了 8 个 4.14 私有 `dt-bindings/*` 头（`mt6885-clk.h`、
`mt6885-pinfunc.h` 等）与 `k6893v1_64_defconfig`，使归档自洽可追溯。

---

## 为什么不能「把官方 dts 原样放进主线编译」

官方 `mt6893.dts`（8276 行）是 **4.14 完整根 dts**，直接编译会：

- `#include <generated/autoconf.h>` + 33 处 `CONFIG_MTK_*` 条件编译，主线 autoconf.h 无这些符号；
- 依赖 4.14 私有 `dt-bindings/*` 头（主线只有 `mt6893-pinfunc.h`）；
- 自带 SoC 层（gic / pio / clocks / 各控制器），与主线/目标仓库已有的
  `mt6893.dtsi`（6235 行）**重复定义**；
- `mediatek,MT6893` 等私有 compatible 在主线无 binding，`dtbs_check` 会大量报错；
- 大量 MTK 私有框架节点（`mtkfb` / `odm` / `CONFIG_MTK_*` 分支）在 7.3.3 无对应驱动。

因此采用**分层移植**（既保证「文件全部到位」，又保证「能编出 dtb」）：

| 层 | 内容 | 位置 |
|---|---|---|
| 1 数据层 | 41 个官方原件**原样**归档，不改动 | 内核树 `arch/arm64/boot/dts/mediatek/ares-official/` |
| 2 组装层 | 从官方根 dts / PMIC / 电池文件提取 ares 真实节点，与板级 overlay 合并 | `tools/assemble_ares_dts_v3.py` |
| 3 契约层 | 补 15 个标准 `*-gpios`（`<&pio N 0>` → `devm_gpiod_get()` 契约） | `port/normalize_dt_gpios.py` |
| 4 接入层 | 主 dts + Makefile dtb 条目 + `-@` | `install_official.sh` |

---

## 6 个最小桩 → 全部消除

| 桩 | v3 处理 | 数据来源 |
|---|---|---|
| `irtx_pwm` | 换官方真实节点（5 行） | `mt6893.dts` L7475：`compatible = "mediatek,irtx-pwm"; pwm_ch = <3>;` |
| `bat_gm30` | 换官方真实电池定义（**666 行**） | `bat_setting/` 三文件按官方 include 顺序拼接 |
| `mmc0_pins_default` | 删除 | 官方 dtsi L60 `/* pinctrl-0 = <&mmc0_pins_default>; */` |
| `mmc0_pins_uhs` | 删除 | 官方 dtsi L61 同上 |
| `nfc_clk_req_active` | 删除 | 官方 dtsi L893 `// &nfc_clk_req_active>;` |
| `nfc_clk_req_suspend` | 删除 | 官方 dtsi L895 同上 |

现在 dts 里 `status = "disabled"` 只剩 4 处，且均为官方真实节点
（`mmc_fixed_3v3` / `mmc_fixed_1v8_io` / `&msdc0` / `&msdc1`），没有任何占位桩。

---

## 编译验证（Linux 7.3.3 内核树真编译）

| 项 | 结果 |
|---|---|
| `tools/assemble_ares_dts_v3.py` | 生成 4968 行（纯官方数据版）；官方基础节点 20/20、PMIC 5/5、pinctrl 4/4、TypeC 1/1 全部提取成功 |
| GPIO 标准化后 | **4983 行**，补丁新增 15 个标准 `*-gpios` |
| `cpp` + `dtc -@` | exit 0，222000 字节，108 警告（均为厂商 dtsi 历史告警），**0 错误** |
| 内核 Makefile `make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb` | **exit 0**，dtb **222447 字节**，`__symbols__` 存在 |
| 反编译回读 | 9104 行；`xiaomi,ares` / `mediatek,irtx-pwm` / `mediatek,bat_gm30` / `R_FG_VALUE` / `goodix,gt9886` / 标准 `*-gpios` / 全部关键外设 compatible 命中；**chopin 残留 0** |

> 用 `LLVM=1` 是必要的：不带它单独编译 dts 会把内核 `.config` 刷成 gcc，
> 导致后续外部模块编译失败（`port/build.sh` 已内置自动恢复）。

---

## 接入方式

```bash
# 1) 安装到主线 7.3.3 内核树（幂等，不覆盖已有文件）
./install_official.sh /path/to/linux-7.3.3

# 2) 编译
cd /path/to/linux-7.3.3
make LLVM=1 ARCH=arm64 mediatek/mt6893-xiaomi-ares-official.dtb

# 3) 从干净状态复现主 dts
./tools/build_official_dts.sh
```

`install_official.sh` 做三件事：复制 41 个官方原件到
`arch/arm64/boot/dts/mediatek/ares-official/`、复制主 dts、在 Makefile 追加
`dtb-$(CONFIG_ARCH_MEDIATEK) += mt6893-xiaomi-ares-official.dtb` 与
`DTC_FLAGS_... += -@`（`-@` 保留 `__symbols__`，LK 叠加 dtbo 必需）。

**起机用 `mt6893-xiaomi-ares-official.dtb`**（含标准 GPIO 契约）；
`mt6893-xiaomi-ares.dtb` / `-mainline.dtb` 保留作前两轮对照。

---

## 如实声明的限制

1. **41 个官方原件不参与 Kbuild 编译**：它们是 4.14 数据源，放在
   `ares-official/` 子目录用于追溯与人工比对；真正编译的是组装后的
   `mt6893-xiaomi-ares-official.dts`。这不等于「官方 dts 全部节点都进了 dtb」——
   官方根 dts 388 个节点里，与主线 SoC 层重复的、以及主线无驱动的 MTK 私有框架节点
   未纳入。
2. **`dtbs_check` 未做**：厂商私有 compatible 缺 yaml binding，只保证 dtb 生成成功；
   13 处 4-cell `interrupts` 保留原样（驱动已改 `gpiod_to_irq(irq-gpios)`，不受影响）。
3. **4 个同 SoC 通用电源/PHY 节点**（`ufs_fixed_vcc` / `vusb33` / `otg_vbus` / `tphy`）
   按主线 binding 重建，电气值参考同 SoC chopin，非 ares 板级实测。
4. **`irtx_pwm` 的 `pwm_ch` 被板级 overlay 覆盖**：官方根 dts 是 `<3>`，
   lineage 板级 `&irtx_pwm` 写成 `<0>`（两数据源交叉的正常优先级结果），未擅自改动。
5. **触控 IC 批次**：官方 defconfig 为 GT9886/GT9896S，lineage dts 写 `focaltech,fts`
   （FT3658U）；官方 5 份触控配置对应不同硬件批次。真机批次需 `dmesg` 确认。
6. **无真机**：所有地址 / 中断 / 电气 / 节点 probe 从未上电验证；编译通过 ≠ 能启动。
7. 干净主线 7.3.3 **没有** SoC 层 `mt6893.dtsi` 与 `clk-mt6893`，
   需先合入目标仓库 `James-2HBe/mt6893_kernel_ares_mainline` 的 SoC 层；
   `install_official.sh` 会检测并提示。按你的要求，这部分「已有且真实可用」未重复移植。
