# B 类（clk / iommu / devfreq）框架符号精确清单

由 stage-5 梳理生成。判据来自 `tools/port_platform_impl.py::classify()` 与
`reports/PLATFORM-IMPL.md` §2 的「类」列：

> **A** = 弱耦合的 MTK 私有实现（7.3.3 无同名子系统，可整体移植）；
> **B** = 与主线 clk / iommu 框架强耦合，需单独适配或提供兼容桩，
> **不建议整体并入主线目录**。

`classify()` 的实际规则是「实现路径以 `drivers/clk/` 或 `drivers/iommu/`
开头即判 B」。本轮按用户口径把 **devfreq** 一并纳入 B 类处理（用户原话
「B 类（clk/iommu/devfreq）框架符号」），因此 `get_cur_vcore_opp` 虽在
stage-3 报告里标 A，也纳入本轮闭环。

## 1. 清单与状态（5 个）

| # | 符号 | 子系统 | 调用次数 | 4.14 实现（保真源） | 声明头 | stage-4 之前 | **stage-5 之后** |
|---|---|---|---|---|---|---|---|
| 1 | `register_pg_callback` | clk / power-gating | 1（修正后 2） | `drivers/clk/mediatek/clk-mt6873-pg.c:827` | `clk-mt6873-pg.h:56`、`clk-mt6885-pg.h:56` | 有实现 `.c`，但标 B「需单独适配」、未做 7.3 移植 | **`solution/framework-symbols.c` 函数级移植 + 主线 PM/notifier 桥接** |
| 2 | `smi_reg_backup_sec` | iommu / SMI | 5（修正后 3） | `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c:247` | `drivers/iommu/mtk_iommu_ext.h:82`、`pseudo_m4u.h:303` | 同上（stage-3 二次追查才定位实现） | **`solution/framework-symbols.c` 桥接到主线 `mtk-smi.c` runtime PM** |
| 3 | `smi_reg_restore_sec` | iommu / SMI | 5（修正后 3） | `pseudo_m4u.c:262` | `mtk_iommu_ext.h:83`、`pseudo_m4u.h:304` | 同上 | **同上** |
| 4 | `get_cur_vcore_opp` | devfreq / DVFSRC | 13（修正后 14） | `drivers/devfreq/helio-dvfsrc-opp.c:76` | `helio-dvfsrc-opp.h:60` | 报告标 A，有实现 `.c` 与补的 `EXPORT_SYMBOL_GPL`，但依赖 SSPM IPI 私有运行时，7.3 上编不进 | **`solution/framework-symbols.c` 哨兵语义 + 主线 DVFSRC 同步入口** |
| 5 | `aee_rr_rec_clk` | clk（调试记录） | 2（真实调用 1） | `drivers/misc/mediatek/ram_console/mtk_ram_console.c:1129` | `mt-plat/mtk_ram_console.h:65` | stage-3 误判「未开源、需桩」 | **stage-4 已解决**：`solution/platform-symbols.c` 函数级移植（本轮不重复定义） |

> 调用次数同时列出 stage-2 旧表与 `reports/SYMBOL-REFS-CORRECTED.md`
> 剥离注释后的修正值；差异不影响「链接期需要该符号」的结论。

**本轮新闭环 4 个**（#1–#4），第 5 个由 stage-4 完成并交叉引用。
至此 33 个链接期平台符号中，B 类全部有 7.3.3 可编译、可链接的实现。

## 2. 每个符号的语义对照

| 符号 | 4.14 做了什么 | 7.3.3 主线等价（`framework/upstream/`） | 本包实现策略 |
|---|---|---|---|
| `register_pg_callback` | `spinlock + LIST_HEAD(pgcb_list)`；注册 `struct pg_callbacks`；掉电调试时 `list_for_each_entry_reverse` 调 `debug_dump` | `clk-mtk.c` 的 `clk_notifier_func` / `devm_clk_notifier_register`；`clk-mux.c:433` `devm_mtk_clk_mux_notifier_register`；`mtk-pm-domains.c` 的 `genpd.power_off/power_on` | 保留原厂回调链（含 reverse 遍历顺序、NULL 检查），新增 `mtk_pg_notify()` / `mtk_pg_debug_dump()` 供目标树在主线 PM / notifier 钩子上转发 |
| `smi_reg_backup_sec` | `KREE_TeeServiceCall(m4u_session, M4U_TZCMD_REG_BACKUP, ...)` 经 TEE 安全世界保存 M4U 寄存器，失败返回 `-1` | `mtk-smi.c:711` `mtk_smi_larb_suspend` + `SET_RUNTIME_PM_OPS` / `SET_LATE_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend, ...)` | 返回 `0`（语义已由主线设备级 PM 覆盖，重复备份反而与主线 regmap 冲突）；提供 `mtk_framework_set_smi_backend()` 注入点 |
| `smi_reg_restore_sec` | TEE `M4U_TZCMD_REG_RESTORE` | `mtk-smi.c:694` `mtk_smi_larb_resume` + `pm_runtime_force_resume` | 同上 |
| `get_cur_vcore_opp` | `is_qos_enabled()` 守护 → `get_cur_vcore_dvfs_opp()` → 查 `vcore_dvfs_to_vcore_opp[]`；未使能 / 越界返回 `VCORE_OPP_UNREQ (-1)` | `mtk-dvfsrc.c:188` `dvfsrc_get_current_opp()->vcore_opp`、`:290-325` `dvfsrc_get_current_level_v1/v2/v4()`；`mtk-dvfsrc-regulator.c` | 保留哨兵语义（未同步时返回 `VCORE_OPP_UNREQ`，让私有栈走「未请求」分支而非误判 OPP 0），新增 `mtk_framework_set_vcore_opp()` 由目标树 DVFSRC 在 OPP 切换时同步 |
| `aee_rr_rec_clk` | 写 `RR_LINUX->clk_data[8]`（异常重启诊断） | 无（纯调试，不影响功能） | stage-4 静态数组后端函数级移植 + 配对读回 |

## 3. 为什么这 5 个是「B 类」而 stage-3 没做

`7.3.3` 主线已原生支持 MT6893 的对应框架（实测，见 `docs/GITHUB-SOURCES-CLASS-B.md`）：

| 主线文件 | MT6893 支持证据 |
|---|---|
| `drivers/memory/mtk-smi.c` | `:286` `mtk_smi_larb_mt6893_ostd`、`:501` `mtk_smi_larb_mt6893`、`:553` `"mediatek,mt6893-smi-larb"`、`:779` `mtk_smi_common_mt6893`、`:848` `"mediatek,mt6893-smi-common"` |
| `drivers/iommu/mtk_iommu.c` | `:1616` `mt6893_data`、`:1906` `"mediatek,mt6893-iommu-mm"` |
| `drivers/soc/mediatek/mtk-dvfsrc.c` | `:68` `vcore_opp`、`:188` `dvfsrc_get_current_opp`、`:290-337` current level / vcore level |
| `drivers/regulator/mtk-dvfsrc-regulator.c` | DVFSRC regulator 对外接口 |
| `drivers/pmdomain/mediatek/mt6893-pm-domains.h` + `mtk-pm-domains.c` | `genpd.power_off = scpsys_power_off`、`power_on = scpsys_power_on` |
| `drivers/clk/mediatek/clk-mtk.c` / `clk-mux.c` | `mtk_clk_simple_probe`、`:571` `clk_notifier_func`、`:438` `devm_mtk_clk_mux_notifier_register` |

所以把 4.14 的 `clk-mt6873-pg.c`（16 万行级）、`pseudo_m4u.c`（7.6 万行级）、
`helio-dvfsrc-opp*.c` 整体搬进主线同名目录，会**重复定义框架结构体与全局
符号**，并引入 7.3 已不存在的 TEE / SSPM IPI 私有运行时。stage-3 因此把
它们标 B 并在报告中写明「由目标树已修基线层提供，本包不代做」。

本轮的解法是**函数级移植 + 主线语义桥接**，而不是整体搬移；`framework/vendor/`
保存的 4.14 实现是**语义基准与路径 B 素材**，默认不参与编译。

## 4. 机器可读清单

`tools/framework-manifest.tsv` 记录 26 个保真源文件的
`side / repo / ref / path / blob_sha / size / status`。

本清单的符号级对照另见 `reports/CLASS-B-PORTING.md` §3。
