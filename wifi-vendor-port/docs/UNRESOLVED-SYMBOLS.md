# 两个「未开源符号」的复核与解决（stage-4）

> 结论先行：stage-3 把 `buck_set_mode` / `aee_rr_rec_clk` 判为
> **「全树只有 extern 声明、无 `.c` 实现，需空桩」**。本轮复核**推翻**了这个结论：
>
> | 符号 | stage-3 判断 | 本轮真实结论 |
> |---|---|---|
> | `aee_rr_rec_clk` | 无实现，需桩 | **有真实开源实现**：`ram_console/mtk_ram_console.c:1129`，已保真导出 + 函数级移植 |
> | `buck_set_mode` | 7 次调用、无实现、需桩 | **废弃死 API**：7 次「调用」全是注释；MT6335 专用而 MT6893 用 MT6359P；真实调用 **0** |
>
> 两个符号都不再需要空桩。链接期 `undefined symbol` 已由 `solution/platform-symbols.c`
> 以 `__weak + EXPORT_SYMBOL_GPL` 消除，且用 7.3.3 头文件做了**实际编译 + nm 符号核验**。

---

## 1. 为什么 stage-3 会判错

两个独立的工具/检索缺陷叠加：

1. **关键词漏搜**。stage-3 用 `aee/`、`pmic/`、`clk-mt68` 等关键词圈定候选 `.c`，
   而 `aee_rr_rec_clk` 的真实实现在 `drivers/misc/mediatek/ram_console/mtk_ram_console.c`
   ——路径里没有 `aee`，被整个排除在候选集外。本轮改为**按子系统目录全量下载**
   （aee / ram_console / pmic / regulator / power-supply，共 504+88 个 `.c` 逐一 grep），才定位到。
2. **调用计数没剥离注释**。`scan_baseline_modules.py::count_refs` 直接对源码文本跑
   `\b(name)\s*\(`，把 `msdc_cust.c` 里 7 行 `buck_set_mode(VCORE, enable)` 的**注释**
   当成 7 次真实调用，进而把该符号列入「链接期必须提供的 33 符号」。

本轮修法：`tools/scan_baseline_modules.py` 增加 `strip_c_comments()`；
`tools/audit_symbol_refs.py` 以「剥离注释 + 剔除声明行」口径重算，产出
`reports/SYMBOL-REFS-CORRECTED.md`（旧报告保留作审计对照）。

---

## 2. `aee_rr_rec_clk` —— 找到并移植真实实现

### 2.1 真实实现

来源：`xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2`，
固定 commit `aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f`。

`drivers/misc/mediatek/ram_console/mtk_ram_console.c:1129`：

```c
void aee_rr_rec_clk(int id, u32 val)
{
	if (!ram_console_init_done || !ram_console_buffer)
		return;
	if (id >= 0 && id < ARRAY_LEN_8)                 /* ARRAY_LEN_8 = 8 */
		LAST_RR_SET_WITH_ID(clk_data, id, val);        /* RR_LINUX->clk_data[id] = val */
	else
		pr_notice("%s invalid id= %d\n", __func__, id);
}
```

保真副本：`baseline/reference/drivers/misc/mediatek/ram_console/`（7 个文件，
逐文件 `git blob sha` 校验 7/7 一致，见 `tools/reference-manifest.tsv`）。
整个 `aee_rr_rec_*` 族有 3598 行、上百个函数，但原厂**只 EXPORT_SYMBOL 了 1 个**
（`aee_rr_curr_vcore_dvfs_opp`）——说明原树这些驱动全 built-in、靠直接链接解析，
并非「未开源」。

语义：把 clk 调试值写入 last-reboot-reason 的 `clk_data[8]`，供异常重启后诊断。
**对 WiFi / 连接功能本身没有任何影响**，是纯调试记录。

### 2.2 为什么做「函数级移植」而非整文件搬 3598 行

`mtk_ram_console.c` 依赖 `ram_console_buffer` / `RR_LINUX` / `pstore` /
`of_reserved_mem` / FIQ / aee 等整套 4.14 私有运行时与保留内存布局。整体并入 7.3
会引入大量框架级 API 冲突，而调用点 `clk-mt6873-pg.c:1100` **只用到这一个函数**。

因此 `solution/platform-symbols.c` 做函数级移植：
- 保留 init guard、`id` 边界检查、越界 `pr_notice`；
- 仅把存储后端从 reserved-memory 的 `RR_LINUX->clk_data[]` 换成本 TU 静态数组
  （`AEE_RR_CLK_STORE` 宏是切换点，目标树若有真实 ram_console 改回一行即可）；
- 补 `EXPORT_SYMBOL_GPL`，并新增配对读回 `aee_rr_curr_clk()`（对齐原厂 `aee_rr_curr_*` 模式）。

### 2.3 与头文件两种声明形态的自洽

`adapted/misc/mediatek/include/mt-plat/mtk_ram_console.h`：
- `CONFIG_MTK_RAM_CONSOLE=y`（`ares_defconfig` 实际取值）：`extern void aee_rr_rec_clk(int,u32);`
  → 正需要外部实现，由 `solution/` 提供，`extern` 声明与 `weak` 定义签名一致；
- 未开启：头里是 `static inline` 空实现，调用点也在同名 `#ifdef` 块内被裁掉
  → 不需要外部符号。

为避免「头文件 inline 兜底」与「本实现」在同一编译单元重定义，
`platform-symbols.c` 的 `aee_rr_rec_clk` 段用
`#if defined(CONFIG_MTK_RAM_CONSOLE) || defined(CONFIG_MTK_PLATFORM_SYMBOLS_FORCE)` 守卫。
两种配置均实测 0 error（见 `solution/verify.sh` 步骤 3b）。

---

## 3. `buck_set_mode` —— 废弃死 API，从链接期清单剔除

### 3.1 三条硬证据

1. **调用全为注释**。7 处「引用」分布在 `msdc_cust.c` 的 7 个平台变体，全部是
   `msdc_pmic_force_vcore_pwm()` 里被注释掉的一行
   （原注释还写着 `Temporarily disable force pwm`）：
   ```c
   void msdc_pmic_force_vcore_pwm(bool enable)
   {
       /* Temporarily disable force pwm */
       /* buck_set_mode(VCORE, enable); */
   }
   ```
   审计口径复核：`出现 7 行 / 注释 7 / 声明 0 / 真实调用 0`，修正后计数 **0**。
   该文件已全面改用主线 `regulator_set_voltage` / `regulator_enable` / `regulator_is_enabled`。
2. **平台不匹配**。声明 `extern int buck_set_mode(enum BUCK_TYPE, unsigned char)` 只在
   `pmic_regulator.h` 的 `#ifdef CONFIG_MTK_PMIC_CHIP_MT6335` 块内；而
   `arch/arm64/configs/ares_defconfig` 是 `CONFIG_MTK_PMIC_CHIP_MT6359P=y`
   （MT6893 用 MT6359P + MT6360）。这个 API 属于另一款 PMIC，在 MT6893 上根本不生效。
3. **全树无实现**。遍历 504 个 `pmic/` + `regulator/` + `power/supply/` 的 `.c`，
   `buck_set_mode` 与 `enum BUCK_TYPE` 均**无任何定义**（连声明都只在头文件）。

### 3.2 结论与处理

- 从「链接期必须提供的 33 符号」清单**剔除**（真实调用 0，不会产生 undefined symbol）。
- `solution/platform-symbols.c` 仍给一个安全兜底：返回 **`-EOPNOTSUPP`** 而非
  stage-3 空桩的静默 `0`，避免上层误以为 force-PWM 已生效；仅在目标树误开
  `CONFIG_MTK_PMIC_CHIP_MT6335` 时才有意义。
- 若将来确有设置 buck 模式的需求，**主线 7.3 的等价能力**是：
  ```c
  #include <linux/regulator/consumer.h>
  regulator_set_mode(reg, REGULATOR_MODE_FAST);    /* force PWM */
  regulator_set_mode(reg, REGULATOR_MODE_NORMAL);  /* auto/PFM  */
  ```
  pmode ↔ mode 的映射在 `solution/platform-symbols.c` 注释里有说明。

---

## 4. 交付物与本轮新增文件

| 路径 | 内容 |
|---|---|
| `solution/platform-symbols.c` | 正式实现：`aee_rr_rec_clk`（函数级移植）、`aee_rr_curr_clk`（读回）、`buck_set_mode`（安全兜底）；均 `__weak + EXPORT_SYMBOL_GPL` |
| `solution/verify.sh` | 7.3.3 头文件实际编译 `-c` 产 `.o` + `nm` 核验符号 + 双配置守卫验证 |
| `baseline/reference/drivers/misc/mediatek/ram_console/` | 原厂真实实现保真副本（7 文件，7/7 blob-sha 一致） |
| `tools/export_reference.py` | 保真导出器（jsdelivr + blob-sha 校验，可复现） |
| `tools/reference-manifest.tsv` | 逐文件 blob sha 清单 |
| `tools/audit_symbol_refs.py` | 调用计数复核（剥离注释/声明），产出勘误报告 |
| `reports/SYMBOL-REFS-CORRECTED.md` | 33 符号计数勘误 + 两个符号逐行证据 |
| `reports/SOLUTION-PROBE.md` | 实现文件的编译探针报告（CLEAN=1 / 0 error） |
| `docs/PLATFORM-STUBS.c.template` | 已升级为「指向正式实现」的接线说明，默认 `#if 0` 不编译 |
| `tools/scan_baseline_modules.py` | 增加 `strip_c_comments()`，修掉注释误计 |
| `tools/adapt_platform_impl.py` | 桩生成逻辑更新（幂等重跑通过） |

---

## 5. 量化验证（Linux 7.3.3 头文件，clang）

| 检查 | 结果 |
|---|---|
| `platform-symbols.c` 单文件 `-c` 编译 | `rc=0`，0 error / 0 warning |
| nm 三个符号类型 | 全部 `W`（weak 全局），可被目标树强符号覆盖 |
| `EXPORT_SYMBOL_GPL` 生效 | 三个 `__export_symbol_*` + `__UNIQUE_ID_addressable_*` 均在 `.o` 中 |
| 编译探针 | `CLEAN=1`、`API=0`、累计 error 0 |
| 真实调用单元 `clk-mt6873-pg.c`（RAM_CONSOLE=y/n） | 均 0 error，无 `aee_rr_rec_clk` 隐式声明 |
| 双配置守卫（solution.c y/n） | 均 0 error |

`solution/verify.sh` 一键复现上述全部检查。

---

## 6. 接线建议

1. 把 `solution/platform-symbols.c` 挂进目标树 Kconfig/Makefile
   （例如 `obj-$(CONFIG_MTK_PLATFORM_SYMBOLS) += platform-symbols.o`），
   与「已修基线层」同批编入；`__weak` 保证不与其强符号冲突。
2. 目标树 `CONFIG_MTK_RAM_CONSOLE` 取实际值（`ares_defconfig` 为 `=y`），
   决定 `aee_rr_rec_clk` 段是否启用；关闭时由头文件 `static inline` 兜底。
3. 若目标树已带真实 ram_console，把 `solution/platform-symbols.c` 里
   `AEE_RR_CLK_STORE/LOAD` 改回 `LAST_RR_SET_WITH_ID` / `LAST_RRR_VAL`
   即可把记录写回保留内存，函数逻辑无需改动。
4. **不要**再链接 `docs/PLATFORM-STUBS.c.template`（已 `#if 0`），
   也不要与本实现同时提供同名强符号以外的新空实现。

> 边界（保持不变）：固件仍由 `init/` 从原厂 vendor 提取、不在包内；
> **真机未验证**；本包未编入既有 7.3.3 的 Image/dtb。33 个平台符号中，
> stage-3 已导出 21 个实现 `.c`（覆盖 31 个符号、补 12 个 `EXPORT_SYMBOL_GPL`），
> stage-4 解决这两个符号后，`solution/` 补齐最后一环；这些 4.14 平台驱动
> **本身仍需 4.14→7.3 的 API 适配与全量链接**，B 类（clk/iommu/devfreq）
> 框架符号由目标树「已修基线层」提供——这部分不属本轮（stage-4）范围。
>
> **stage-5 更新（追加勘误，上文保留作审计对照）**：B 类
> （clk/iommu/devfreq）框架符号已在 stage-5 于包内闭环——从 GitHub
> `torvalds/linux@master` 与 `lineage-23.2` 保真导出 26 个文件（26/26
> blob-sha 一致），以「函数级移植 + 主线语义桥接」落地到
> `solution/framework-symbols.c`（8 个 `__weak + EXPORT_SYMBOL_GPL` 符号，
> 7.3.3 实测 0 error）。目标树现在只需按 `docs/CLASS-B-ADAPTATION.md`
> 接线，不再需要自己补齐 B 类框架符号。详见 `reports/CLASS-B-PORTING.md`。

## 7. 复现命令

> **交付自包含**：`baseline/reference/` 的保真副本与 `tools/reference-manifest.tsv`
> 已在包内，目标树接线**不需要**重新克隆源仓库或联网。下面第 1 步只是
> 「审计复现」（想重新核对 blob sha 时才需要 `/tmp/mt6893-lin232` 稀疏克隆）；
> 第 2、3 步只依赖包内文件 + Linux 7.3.3 内核树，可离线运行。

```bash
cd wifi-vendor-port
# 1) 【可选·审计复现】重新保真导出原厂真实实现（需源稀疏克隆）
python3.11 tools/export_reference.py --repo /tmp/mt6893-lin232
# 2) 调用计数复核（勘误，纯本地）
python3.11 tools/audit_symbol_refs.py
# 3) 实现编译 + 符号核验 + 双配置验证（仅需 7.3.3 内核树）
sh solution/verify.sh /path/to/linux-7.3.3
```
