# 第 3 轮：把所有能修的全部修好 + kernel_mt6893 复用评估

> 目标内核：Linux **主线 7.3.3**；对象：`ares-7.3.3` 工程。
> 本轮两件事：①修掉前面几轮「如实声明的限制」里**技术上可修**的部分；
> ②评估 `James-2HBe/kernel_mt6893` 的设备树能否复用或辅助移植。

---

## 一句话结论

- **`kernel_mt6893` 不能整体复用**，但**能辅助移植**：它是同一套小米官方 ares 设备树的
  **另一分支版本**（4.14.357-openela、带 KVM、chopin/ares 真机测过），含此前归档缺失的
  ares 实机校准参数；已归档 29 个文件为第二数据源，并用于交叉验证本轮修复。
- **12 类主线契约硬伤全部修好**，且修复落在可复现脚本层（幂等），不是手改生成物：
  `status` 非法值、4-cell IRQ、三触控批次冲突、触觉双总线冲突、错的 tphy compatible、
  otg_vbus 电压、无 `reg` 的单元地址节点、irtx 通道真值、AW8697 重复块、chopin 注释。
- `dtbs_check` **首次跑通**，可修错误清零；剩余 4027 行中 611 处是「主线无 yaml binding 的
  厂商私有节点」、其余为 SoC 层 `mt6893.dtsi` 的历史偏差，均属**不可修**（见文末）。

---

## 一、`kernel_mt6893` 复用评估

### 仓库性质

| 项 | 值 |
|---|---|
| 默认分支 | `chopin_kvm` |
| 内核版本 | **4.14.357-openela**（`Makefile` VERSION/PATCHLEVEL/SUBLEVEL） |
| 描述 | Kernel for Xiaomi MT6893 (**tested on chopin and ares**) with KVM/virtualization |
| 文件树 | 70,345 条目（truncated），`mediatek/` 下 138 个 `.dts/.dtsi/.dws` |
| `mediatek/` 内容 | 完整小米官方 ares 设备树：`ares.dts`、`ares-mt6893.dtsi`、`k6893v1_64.dts`、`mt6893.dts`(8391 行)、`xiaomi-mt6893-common.dtsi`、`bat_setting/`、各 `cust_mt6893_*` |

### 与已归档 41 个官方原件的关系（字节级比对）

26 个同名文件：**14 个完全相同，12 个有实质差异**。

| 文件 | 本地 ares-r-oss | kernel_mt6893 | 差异性质 |
|---|---:|---:|---|
| `xiaomi-mt6893-common.dtsi` | 20873 | 24281 | **+144 行 AW8697 完整波形校准**、i2c9 触觉块 |
| `ares-mt6893.dtsi` | 17190 | 19338 | **+327 行**：FT3658U SPI 触控全参数、WL2866D 相机 LDO、`thermal-message`、`&spi6` |
| `mt6893.dts` | 223292 | 225707 | +158 行：fhctl-new、cqdma 主线 binding、reserve-memory-ssmr、cqdma/dpmaif 参数 |
| `k6893v1_64.dts` | 19101 | 19185 | `tee-only`、相机 sensor 名、`irq_nfc` 修正、触控 include 换为 `cust_mt6893_touch.dtsi` |
| `cust_mt6893_touch.dtsi` | — | 改写 | GT9886 分辨率 **2280→2300**、固件 **6893v01→6893v02** |
| `mtk_pe50.dtsi` / `mt6360_pd.dtsi` / `tcpc_config.dtsi` | — | 改写 | 充电曲线、PD VDO 3→6 cell、`usbr20_not_used` 调整 |
| `mt6315_s6.dtsi` | 971 | 1151 | 补 `6_vbuck4` regulator |
| `cust_mt6885_msdc.dtsi` | 4403 | 4211 | `no-mmc`、`#ifndef CONFIG_FPGA_EARLY_PORTING`、删 hs200 |
| `mt6360.dtsi` | 5884 | 5894 | `en_otg_wdt`、VMC `regulator-always-on` |

新仓库独有 8 个 6893/ares 相关文件（`agate/amber/saga-mt6893.dtsi`、
`cust_mt6893_msdc.dtsi`、`k6893v1_64_k510.dts` 等），其中
`cust_mt6893_msdc.dtsi` 与 `cust_mt6885_msdc.dtsi` **内容完全相同**（同一文件两名）。

### 结论：不能直接复用，只能辅助

**不能整体复用进 7.3.3 的原因**（与前面几轮一致，本轮再次核实）：

1. 是 **4.14 厂商根 dts**（`mt6893.dts` 8391 行），自带 SoC 层，与主线/目标仓库
   已有的 `mt6893.dtsi`(6235 行) **重复定义**；
2. `#include <generated/autoconf.h>` + `CONFIG_MTK_*` 条件编译，主线 autoconf 无这些符号；
3. 依赖 4.14 私有 `dt-bindings/*` 头；`mediatek,MT6893` 等私有 compatible 主线无 binding；
4. 大量 MTK 私有框架节点（`mtkfb`/`odm`/`fhctl`）在 7.3.3 无驱动；
5. 该分支含 **chopin 专属改动**（如 `mt6893.dts` 末尾 `&msdc1 { status="disabled"; }`
   注释明写 "chopin has no microSD slot… as ares does"），直接套用会把 chopin 的板级
   假设带进 ares。

**能辅助移植的 4 类价值**（本轮已落地）：

| 价值 | 落地 |
|---|---|
| ① ares 实机参数交叉验证 | 归档 29 个文件到 `vendor-reference/kernel_mt6893/`，含官方 `ares_defconfig` |
| ② 确认硬件批次真实并存 | 两仓库 defconfig 都写 `"GT9886 GT9896S"`，新仓库另有 `CONFIG_TOUCHSCREEN_FOCALTECH=y`，`CONFIG_MTK_FINGERPRINT_SELECT="GF3658"`，`CONFIG_MTK_GAUGE_VERSION=30` → 证明三触控批次/GF3658/电池 gauge 30 都是实机配置，不是拼凑 |
| ③ 校准 irtx 真值 | 两仓库官方根 dts 的 `pwm_ch` 都是 **`<3>`**，lineage overlay 的 `<0>` 是偏离 → 本轮改回官方真值 |
| ④ 确认电压/校准参数出处 | 官方 `mt6359p.dtsi` 的 `ldo_vusb` = 3.0V；AW8697 `vib_effect_max` 存在 170(官方) / 197(另一分支) 两个批次值 |

> 注：`vusb33` 保持 3.3V 未改。它是主线 SoC 层 `usb@vbus33-supply` 的 3.3V 域，
> 与 PMIC `ldo_vusb`(3.0V) 是不同电源轨；无 ares 板级实测依据，不擅自改动。

---

## 二、本轮修好的 12 类问题（全部可复现、幂等）

新增 `tools/fix_mainline_contract.py`，接入 `tools/build_official_dts.sh` 第 3 步：

```
v3 组装(官方真值,0桩) → normalize_dt_gpios(15 个 *-gpios) → fix_mainline_contract → 主 dts
```

| # | 问题 | 依据 | 修复 | 处数 |
|---|---|---|---|---:|
| 0 | 同一 `&i2c7` 下 AW8697 定义两次，dtc 静默合并、属性互相覆盖（170 vs 197） | 官方块带主线 `*-gpios` 契约 | 冗余块用 cpp `#if 0` 包裹（数据保留源码，可恢复） | 1 |
| 1 | `status = "ok"` 是主线非法值，`of_device_is_available()` 不认 | `dtbs_check` 点名 focaltech/gt9886/aw8697 | → `"okay"` | 14 |
| 2 | **3 个触控节点同时 `okay`**，共用 GPIO20/21 + IRQ21，真机必然冲突 | 两仓库 defconfig 证实三批次并存，靠 4.14 defconfig 二选一 | cpp 宏 `ARES_TOUCH_VARIANT` 编译期互斥（默认 GT9886，另两个 `disabled`） | 4 |
| 3 | AW8697 同时挂 `i2c7` 与 `i2c9`，共用 GPIO129/22 | 原厂 ares-r-oss 用 i2c7；另一分支用 i2c9 | cpp 宏 `ARES_HAPTIC_BUS` 互斥（默认 i2c7） | 3 |
| 4 | 13 处 **4-cell `interrupts`** `<N IRQ_TYPE N 0>`，主线 `pio` 是 `#interrupt-cells=<2>` | 主线 `mt6893.dtsi` pio 定义；9 个驱动全用 `gpiod_to_irq` 不依赖该属性 | → 2-cell `<N IRQ_TYPE>` | 13 |
| 5 | `tphy` 用了 `mediatek,mt6785-tphy`，主线 `mediatek,tphy.yaml` 只认 `mt6893-tphy` | 主线 binding enum | → `mediatek,mt6893-tphy` | 1 |
| 6 | `otg_vbus` min 4425000 / max 5820000 不等，违反 `fixed-regulator.yaml` | 主线 binding 要求 min=max | → 5V / 5V | 1 |
| 7 | `fixedregulator@0/1` 带单元地址却无 `reg`，`root-node.yaml` 报缺 reg | `dtbs_check` | → `regulator-0/1`（去掉 `@N`） | 2 |
| 8 | `irtx_pwm` 的 `pwm_ch` 被 lineage overlay 覆盖为 `<0>` | **两个官方仓库根 dts 均为 `<3>`** | → 官方真值 3 | 1 |
| 9 | 2 处 chopin 字样注释（易误导为 chopin 板级值） | 数据源标注 | → 中性表述 | 2 |
| 10 | 主线契约宏头（批次选择器 + 判别方法） | — | 插入 `#define` 组，改一行即切批次 | 1 |
| 11 | 生成 `port/dt-bindings/mainline-contract.patch` | — | 可审计补丁 | — |

**为什么用 cpp 宏而不是删节点**：ares 确实存在 FT3658U / GT9886 / GT9896S 三种触控
批次（两个官方 defconfig 都写 `"GT9886 GT9896S"`，新仓库再加 `TOUCHSCREEN_FOCALTECH=y`），
删任何一个都会丢真机数据。宏让**全部节点保留在源码**，只在编译期选一个占用
IRQ/GPIO，改一行即可适配另一批次 —— 这比 4.14 靠 defconfig 重编更轻。

### 切换方法（`mt6893-xiaomi-ares-official.dts` 顶部）

```c
#define ARES_TOUCH_VARIANT  ARES_TOUCH_GT9886   /* 可改 FT3658U / GT9896S */
#define ARES_HAPTIC_BUS     ARES_HAPTIC_I2C7    /* 可改 ARES_HAPTIC_I2C9 */
```

真机判别：`dmesg` 看触控 IC 自报 ID；或 i2c 探测 `0x5d`(GT9886) / `0x5a`(AW8697)
落在哪条总线。

---

## 三、验证结果

| 项 | 结果 |
|---|---|
| `./tools/build_official_dts.sh` | 复现成功，主 dts **5039 行**，幂等自检通过 |
| cpp 宏实际生效（反编译 dtb 回读） | GT9886 `okay`；FT3658U / GT9896S `disabled`；AW8697 恰好一个 `okay` 一个 `disabled` |
| 4-cell IRQ 残留 | **0** |
| 内核树 `make LLVM=1 ARCH=arm64 ...dtb` | **exit 0**，dtb **222403 字节**，`__symbols__` 存在 |
| chopin 残留 | **0** |
| `dtbs_check`（首次跑通，`dt-validate 2026.9`） | 4087 → **4027 行**；status 条件失败 **9→0**、错 tphy **7→0**、fixedregulator/reg **2→0**、interrupts 超限 **13→10** |
| 9 个驱动模块回归编译 | 见 `port/build.log`，0 error / 0 warning / 9 个 `.ko` |

### `dtbs_check` 剩余错误为何不可修

| 类别 | 行数量级 | 原因 |
|---|---|---|
| `failed to match any schema with compatible` | 611 | 4.14 厂商私有节点（mtkfb/odm/charger/camera…）在主线 7.3.3 **没有驱动也没有 yaml binding**。要清零只能删节点或编造 binding，都不是移植 |
| `b'X' is not of type ...`（wf_*/mediatek,smi-id 等） | ~1565 | 同上：私有属性被通用 schema 当未知属性报；私有节点无 binding |
| `$nodename does not match`（toprgu/ufs_mphy/spi0/i2c0…） | ~38 | 来自 **SoC 层 `mt6893.dtsi`**，属用户明确「已有且真实可用、不重复移植」的部分 |
| `too long` 剩余 10 处 | 10 | 全部是 SoC 层 `mt6893.dtsi` 的 wdt/dvfsrc/ufshci/ufsphy/spmi 节点（已核实组装 dts 中 0 处） |
| `simple-bus` 缺 `ranges`/`reg`（odm 下 led@N 等） | ~36 | 厂商 `odm` 私有容器写法，无主线 binding |

> 换句话说：**能修的（板级 dts 自身契约错误）已全部修完**；剩下的是「主线内核缺驱动」
> 和「SoC 层历史偏差」两个外部前提，不属于本工程可修范围。

---

## 四、如实保留的限制（与前面几轮一致）

1. **无真机**：所有地址 / 中断 / 电气 / 节点 probe 从未上电验证；编译通过 ≠ 能启动。
2. **41 个官方原件 + 29 个新仓库原件不参与 Kbuild**：是 4.14 数据源，放在
   `ares-official/` 与 `vendor-reference/` 供追溯；真正编译的是组装后的主 dts。
3. 干净主线 7.3.3 **没有** SoC 层 `mt6893.dtsi` 与 `clk-mt6893`，需先合入目标仓库
   `James-2HBe/mt6893_kernel_ares_mainline` 的 SoC 层；按你的要求这部分「已有且真实可用」
   未重复移植（本工作区的 7.3.3 内核树已含该 `mt6893.dtsi`）。
4. **GF3658 指纹**仍需 TEE TA 与 Android HAL 配合；主线驱动只提供 REE 侧 SPI/ioctl 通路。
5. 4 个同 SoC 通用电源/PHY 节点（`ufs_fixed_vcc`/`vusb33`/`otg_vbus`/`tphy`）按主线
   binding 重建，电气值参考同 SoC 官方 PMIC，非 ares 板级实测。
6. `&msdc1` 保持 `disabled`（ares 无 microSD，与官方根 dts 注释一致）。
