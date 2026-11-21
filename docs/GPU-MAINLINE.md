# ares 主线格式 GPU 节点（Panfrost / Mali-G77）

> 目标：让 MT6893 的 GPU 节点满足主线 `arm,mali-bifrost.yaml` 契约，
> 能被 `CONFIG_DRM_PANFROST` 匹配，并通过 `dtbs_check`。
> 本文只描述**已落地并验证**的内容；未解决的真机限制单列在「已知限制」。

## 1. 为什么需要改

SoC 层 `mt6893.dtsi` 里的 `mali@13000000` 是 4.14 ALPS 私有格式：

| 项 | 原厂写法 | 主线要求 |
|---|---|---|
| compatible | `"mediatek,mt6893-mali"`（单串，schema 无此条） | SoC 串 + `"arm,mali-valhall-jm"` |
| 节点名 | `mali@13000000` | `^gpu@[a-f0-9]+$` |
| clocks | `<&topckgen 0x69 ...>` 共 11 项 2-cell 私有时钟 | `clocks` 1–3 项、`clock-names` |
| interrupts | `<0 0x16c 4 0 0x16b 4 0 0x16a 4>` 裸数值 | GIC 宏 + `interrupt-names` |

`&label { ... }` 可以覆盖属性，但**改不了节点名**，而节点名是 schema 的硬性要求。
因此采用：先 `/delete-node/ &mali;` 删除 SoC 层私有节点，再整体新建一个
`gpu@13000000` 节点。删除是安全的——`phandle = <0x35>` 在整机 dts 里没有任何
其它节点作为 phandle 引用（其余 `0x35` 出现都是普通数据值）。

## 2. 落地内容

| 文件 | 改动 |
|---|---|
| `mt6893-xiaomi-ares-official.dts` | 末尾注入 GPU 块（`/delete-node/` + `gpu@13000000` + `gpu_opp_table`），两份工程同步 |
| `tools/fix_mainline_contract.py` | 新增 `MAINLINE_GPU_BLOCK` 常量与 `fix()` 第 12 步注入；以 `gpu_opp_table:` 为哨兵保证幂等 |
| `port/dt-bindings/mali-mt6893.patch` | 在 `arm,mali-bifrost.yaml` 的 Valhall enum 加 `mediatek,mt6893-mali` |
| `auto_install.sh` | 第 3 步增加 `apply_compat` 调用，干净内核树会自动应用该 binding 补丁 |
| `ares_defconfig.fragment` | 强制 `CONFIG_DRM_PANFROST=y` 及其 8 个 select 依赖 |
| `mt6893-xiaomi-ares-official.dtb` | 由 cpp + dtc 重新生成（222541 字节） |

> 组装链已带注入逻辑，**重建 dts 不会丢失 GPU 节点**；`fix_mainline_contract.py`
> 的幂等自检已验证通过。

## 3. 数据来源（不编造）

**中断换算**：厂商 `<0 0x16c 4  0 0x16b 4  0 0x16a 4>` 中 `0` 即 `GIC_SPI`、
`4` 即 `IRQ_TYPE_LEVEL_HIGH`，所以 364/363/362 **本身就是 GIC 编号，不需要减 32**
（早期评估说"减 32"是错的，已纠正）。主线 `mt8192.dtsi` 用 365/364/363，
与本 SoC 一一对应。交付 dts 的 GIC 是 `#interrupt-cells = <3>`，故写 3-cell；
`mt8192` 的 4-cell 写法不能照抄。

**reg**：`0x13000000 + 0x4000`，与 SoC dtsi 一致。

**频率/电压**：全部取自官方 `ares-r-oss` 的
`drivers/misc/mediatek/base/power/include/gpufreq_v1/mt6893/mtk_gpufreq_internal_plat.h`：

| OPP 档 | 频率来源 | 电压来源 |
|---|---|---|
| 485 MHz | `MT_GPUFREQ_BATT_OC_LIMIT_FREQ` / `MT_GPUFREQ_LOW_BATT_VOLT_LIMIT_FREQ` = 485000 KHz | `PTPOD_DISABLE_VOLT` = 75000 (mV×100) = 750 mV |
| 836 MHz | `MT_GPUFREQ_CUST_INIT_OPP` = 836000 KHz | `GPU_ACT_REF_VOLT` = 80000 (mV×100) = 800 mV |

VGPU 硬件范围 `VGPU_MIN_VOLT 30000` ~ `VGPU_MAX_VOLT 119375`（mV×100），
`PMIC_STEP 625`。**这两档是「绑定合规 + 可 probe」的最小集，不是原厂完整
DVFS 档位表**，频率和电压也不声称是原厂精确配对；完整档位依赖下文第 1 项。

## 4. 验证结果

```
dt-mk-schema -j -o processed-schema-gpu.json Documentation/devicetree/bindings
dt-validate -s processed-schema-gpu.json -l arm,mali-bifrost  ares.dtb   # GPU 节点零诊断
dt-validate -s processed-schema-gpu.json                     ares.dtb   # GPU/OPP 相关 0 行
dt-validate -s processed-schema-gpu.json -l opp-v2           ares.dtb   # OPP 表零诊断
make LLVM=1 ARCH=arm64 drivers/gpu/drm/panfrost/                        # rc=0，10 个 .o
```

- `arm,mali-bifrost.yaml` 补丁在干净内核树上 `git apply --check` / `git apply` 均通过，
  应用结果与手工版逐字节一致。
- `force_defconfig.py` + `make defconfig` + `make olddefconfig` 后，最终 `.config`
  中 `CONFIG_DRM_PANFROST=y` 与 8 个依赖符号全部为 `y`。
- 反编译 dtb 确认：旧 `mali@13000000`、`mtcmos_mfg_async` 等私有属性已消失，
  新节点 `compatible`、`interrupts = <0 0x16c 4 0 0x16b 4 0 0x16a 4>`、
  `mali-supply = <0x4b>`、`operating-points-v2` 均正确。

> 完整 `dt-validate` 仍会输出约 3400 行既有告警（pinctrl 的
> `drive-strength` / `offset` 属性类型等），属既有技术债，与 GPU 无关。

## 5. 已知限制（主线缺件，不是本轮漏做）

1. **时钟**：`clocks = <&clk26m>`（26 MHz `fixed-clock`）只用于满足 schema 的
   `required` 并让 panfrost 的 `devm_clk_get(dev, NULL)` 取到 clock。真实 MFG PLL
   与 DVFS 需要 `clk-mt6893`，主线 `drivers/clk/mediatek` 没有该驱动。
2. **供电**：`mali-supply = <&mt_pmic_vgpu11_buck_reg>` 指向厂商 PMIC 的 vgpu11
   buck（phandle `0x4b`），该节点在私有 PMIC 树下，主线无 MT6359 regulator 驱动，
   真机 probe 需 regulator 框架兜底。
3. **电源域 / IOMMU**：未接 `power-domains`，GPU 至多完成 probe，**不保证渲染可用**。
4. **OPP**：最小集，见第 3 节。

原厂 4.14 的 GPU 走 Mali DDK + GED + `mtkfb` 闭源栈，依赖 Android HAL 与私有
sysfs，属于**不可移植**而非漏做。本文的目标是「主线格式 + 绑定通过 + 驱动可 probe」，
不是「GPU 完整可用」。

## 6. 复现

```bash
# 组装链会自动带回 GPU 节点
python3 tools/fix_mainline_contract.py <输入.dts> <输出.dts>

# binding 补丁（auto_install.sh 已自动处理）
git apply port/dt-bindings/mali-mt6893.patch

# 重建 schema 后校验
cd <内核树>
dt-mk-schema -j -o /tmp/schema.json Documentation/devicetree/bindings
dt-validate -s /tmp/schema.json -l arm,mali-bifrost <board>.dtb
```
