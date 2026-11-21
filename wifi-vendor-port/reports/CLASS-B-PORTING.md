# B 类（clk/iommu/devfreq）框架符号移植报告（stage-5）

> **结论先行**：按用户要求「移植 B 类框架符号，通过 GitHub 上的开源仓库」，
> 本轮把 stage-2/3 明确留给目标树「已修基线层」的 **B 类框架符号全部闭环**。
>
> | 项 | 结果 |
> |---|---|
> | B 类符号 | **5 个**：`register_pg_callback`、`smi_reg_backup_sec`、`smi_reg_restore_sec`、`get_cur_vcore_opp`（本轮新闭环 4 个）+ `aee_rr_rec_clk`（stage-4 已解决） |
> | GitHub 开源源 | **`torvalds/linux@master`**（新引入，10 文件）+ `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`（16 文件） |
> | 保真校验 | **26/26** 逐文件 `git blob sha` 一致 |
> | 实现方式 | **函数级移植 + 主线语义桥接**（不把 4.14 私有 clk/iommu/devfreq 驱动整体并入主线目录） |
> | 7.3.3 实测编译 | `framework-symbols.c` `-c` 产 `.o`：**0 error / 0 warning** |
> | 符号核验 | **8 个** `__weak + EXPORT_SYMBOL_GPL` 符号全部 `W` + `__export_symbol_*` 生效 |
> | 双配置 | standalone / `MTK_FRAMEWORK_HAVE_PG_H` 均 **0 error** |
> | `__weak` 覆盖 | `ld.lld -r` 注入强符号后变 `T`，**无 multiple definition**（路径 A / B 均可链接） |
> | 编译探针 | `CLEAN=2 / API=0 / 累计 error 0` |

---

## 1. 起点：B 类是什么、为什么此前没做

`tools/port_platform_impl.py::classify()` 把 33 个链接期平台符号按耦合度分
A/B；`reports/PLATFORM-IMPL.md` §2 的「类」列给出定义：

> **A** = 弱耦合的 MTK 私有实现（7.3.3 无同名子系统，可整体移植）；
> **B** = 与主线 clk/iommu 框架强耦合，需单独适配或提供兼容桩，
> **不建议整体并入主线目录**。

`docs/PLATFORM-IMPL-PORTING.md` §8 与 `docs/UNRESOLVED-SYMBOLS.md` §6
都把它写成边界：

> 「`clk` / `iommu` / `devfreq` 等与主线框架强耦合的模块（报告中 B 类）
> 需要单独适配，不建议整体并入主线目录」；
> 「B 类（clk/iommu/devfreq）框架符号由目标树『已修基线层』提供——
> 这部分不属本轮范围」。

本轮按用户要求把这条边界真正做掉。用户口径把 **devfreq** 也纳入 B 类
（原话「B 类（clk/iommu/devfreq）框架符号」），故 `get_cur_vcore_opp`
虽在 stage-3 报告里标 A，也一并闭环。精确清单见 `reports/CLASS-B-SYMBOLS.md`。

## 2. GitHub 开源源与保真导出

| 仓库 | ref | 文件数 | 落点 | 作用 |
|---|---|---|---|---|
| `torvalds/linux` | `master` | 10 | `framework/upstream/` | 主线 Linux 的 MT6893 框架等价实现（锚点 / 接线依据） |
| `xiaomi-mt6893-dev/kernel_xiaomi_mt6893` | `lineage-23.2` @ `aeb1327b1c73…` | 16 | `framework/vendor/` | 4.14 私有 B 类符号真实实现 + MT6893 variant / Makefile / 私有头 |

工具：`tools/export_framework.py`；清单：`tools/framework-manifest.tsv`
（`side / repo / ref / path / blob_sha / size / status`）。

导出通道（本环境实测）：

- 主线：`api.github.com` contents 取 blob sha → `raw.githubusercontent.com`
  下载 → `sha1("blob <len>\0"+data)` 比对；`git blobs` API(base64) 兜底。
  **`cdn.jsdelivr.net` 对 `torvalds/linux` 返回 403（package > 50 MB），弃用**。
- vendor：`git ls-tree` 取 blob sha → `cdn.jsdelivr.net/gh/<repo>@<commit>/<path>`
  下载 → 同样校验（与 stage-3 一致）。

**26/26 校验通过**，包内保真副本自包含，目标树接线无需联网。

### 一条否定结论（如实记录）

Collabora 2025-04-10 的 `[PATCH v1 0/3] MediaTek Dimensity 1200 - Add Clocks
support`（`clk-mt6893-topckgen.c` 等 15 文件 / +2455 行，主线风格）**尚未
合入任何可公开访问的 GitHub 分支**：`torvalds/linux@master` 的
`drivers/clk/mediatek/` 221 条目无 6893 文件，`v6.16` tag、`gregkh/linux`、
以及若干个人 fork 均 404。因此不作保真源，仅记为「主线化方向参考」。
逐条探测记录见 `docs/GITHUB-SOURCES-CLASS-B.md` §3。

## 3. 核心技术判断：不整体搬 4.14 私有驱动

实测目标树 7.3.3 **已原生支持 MT6893 的对应框架**：

| 子系统 | 主线证据 |
|---|---|
| SMI | `mtk-smi.c:286/501/553/779/848`（`mediatek,mt6893-smi-larb` / `-smi-common`）；`:694/:711` `mtk_smi_larb_resume/suspend` |
| IOMMU | `mtk_iommu.c:1616` `mt6893_data`、`:1906` `"mediatek,mt6893-iommu-mm"` |
| DVFSRC | `mtk-dvfsrc.c:188` `dvfsrc_get_current_opp()->vcore_opp`、`:290-337` current/vcore level；`mtk-dvfsrc-regulator.c` |
| PM domain | `mt6893-pm-domains.h` + `mtk-pm-domains.c:975-976` `genpd.power_off/power_on` |
| clk 框架 | `clk-mtk.c:571` `clk_notifier_func`、`clk-mux.c:433-440` `devm_mtk_clk_mux_notifier_register` |

把 4.14 的 `clk-mt6873-pg.c`（160 KB）、`pseudo_m4u.c`（76 KB）、
`helio-dvfsrc-opp*.c` 整体搬进主线同名目录会**重复定义框架结构体与全局
符号**，并引入 7.3 已不存在的 TEE（`KREE_TeeServiceCall`）/ SSPM IPI 私有
运行时。所以本轮做的是「**函数级移植 + 主线语义桥接**」：

- 保留 4.14 的对外契约与关键语义（回调链、reverse 遍历、边界检查、
  `VCORE_OPP_UNREQ` 哨兵）；
- 把存储/触发后端换成 7.3 主线可接线入口；
- `framework/vendor/` 的 4.14 实现作为**语义基准与路径 B 素材**，默认不编译。

## 4. 5 个符号的实现落点

| 符号 | 4.14 语义 | 7.3.3 主线等价 | 本轮实现 |
|---|---|---|---|
| `register_pg_callback` | `spinlock + LIST_HEAD` 注册 `struct pg_callbacks`；掉电调试 reverse 遍历 `debug_dump` | `clk_notifier_func` / `devm_clk_notifier_register`；`genpd.power_off/on` | `solution/framework-symbols.c`：原样保留回调链 + NULL 检查；新增 `mtk_pg_notify()` / `mtk_pg_debug_dump()` 桥接入口 |
| `smi_reg_backup_sec` | TEE `M4U_TZCMD_REG_BACKUP` 保存 M4U 寄存器 | `mtk_smi_larb_suspend` + `pm_runtime_force_suspend`（`SET_LATE_SYSTEM_SLEEP_PM_OPS`） | 返回 `0`（语义由主线 PM 覆盖）；提供 `mtk_framework_set_smi_backend()` 注入点 |
| `smi_reg_restore_sec` | TEE `M4U_TZCMD_REG_RESTORE` | `mtk_smi_larb_resume` + `pm_runtime_force_resume` | 同上 |
| `get_cur_vcore_opp` | QoS 守护 + SSPM IPI 查 `vcore_dvfs_to_vcore_opp[]`；未使能返回 `VCORE_OPP_UNREQ` | `dvfsrc_get_current_opp()->vcore_opp`、`dvfsrc_get_current_level_v*` | 保留哨兵语义；新增 `mtk_framework_set_vcore_opp()` 由目标树 DVFSRC 同步 |
| `aee_rr_rec_clk` | 写 `RR_LINUX->clk_data[8]`（异常重启诊断） | 无（纯调试，不影响功能） | **stage-4** `solution/platform-symbols.c` 静态数组后端函数级移植；本轮不重复定义 |

全部 8 个导出符号（4 个原厂 + 4 个新增桥接入口）均为
`__weak + EXPORT_SYMBOL_GPL`：

- **路径 A**（推荐）：目标树只编本实现，不编 4.14 私有驱动 → 消除 undefined；
- **路径 B**：目标树已有 4.14 强实现 → 链接器选强符号，weak 静默退场。

接线代码、Kconfig/Makefile 片段与检查清单见
`docs/CLASS-B-ADAPTATION.md`、`framework/Kconfig.fragment`、
`framework/Makefile.fragment`。

## 5. 量化验证

`sh solution/verify-class-b.sh /workspace/linux-7.3.3`（实跑输出）：

| 检查 | 结果 |
|---|---|
| `[1/5]` 7.3.3 真实 `KBUILD_CFLAGS` 下 `-c` 编译 | `rc=0`，**0 error / 0 warning** |
| `[2/5]` `nm` 8 个符号类型 | 全部 `W`（weak 全局），每个带 1 个 `__export_symbol_*` |
| `[3/5]` 双配置 | `standalone rc=0 error=0`；`HAVE_PG_H rc=0 error=0` |
| `[4/5]` `__weak` 覆盖 | `ld.lld -r` 合并注入强符号后 `get_cur_vcore_opp` / `smi_reg_backup_sec` 变 `T`，无 multiple definition |
| `[5/5]` 编译探针 | `CLEAN=2`、`API=0`、`累计 error 行 0`、首条 fatal 前无旧 API 残留 `2/2` |

符号清单（步骤 2 实跑）：

```
register_pg_callback          W   __export_symbol_register_pg_callback
mtk_pg_notify                 W   __export_symbol_mtk_pg_notify
mtk_pg_debug_dump             W   __export_symbol_mtk_pg_debug_dump
smi_reg_backup_sec            W   __export_symbol_smi_reg_backup_sec
smi_reg_restore_sec           W   __export_symbol_smi_reg_restore_sec
mtk_framework_set_smi_backend W   __export_symbol_mtk_framework_set_smi_backend
get_cur_vcore_opp             W   __export_symbol_get_cur_vcore_opp
mtk_framework_set_vcore_opp   W   __export_symbol_mtk_framework_set_vcore_opp
```

与 stage-4 口径一致：`solution/verify.sh` 核验 3 个符号，本轮
`verify-class-b.sh` 核验 8 个，两者可分别一键复现，互不干扰。

## 6. 本轮新增 / 修改文件

| 路径 | 内容 |
|---|---|
| `framework/upstream/` | GitHub `torvalds/linux@master` 保真副本（10 文件，按源树原路径） |
| `framework/vendor/` | GitHub `lineage-23.2` 保真副本（16 文件，含 MT6893 variant / Makefile / 私有头） |
| `framework/{Kconfig,Makefile}.fragment` | 目标树接线片段（含路径 A / B 说明） |
| `framework/README.md` | 素材目录说明与复现命令 |
| `compat/mtk_framework_compat.h` | B 类契约头（standalone / `HAVE_PG_H` 双形态） |
| `solution/framework-symbols.c` | **主实现**：4 个 B 类符号 + 4 个桥接入口 |
| `solution/verify-class-b.sh` | 5 项验证（编译 / nm / 双配置 / weak 覆盖 / 探针） |
| `tools/export_framework.py` | 两仓库保真导出器（blob-sha 校验，可复现） |
| `tools/framework-manifest.tsv` | 26 文件逐条 sha 清单 |
| `reports/CLASS-B-SYMBOLS.md` | 5 个 B 类符号精确清单与语义对照 |
| `reports/CLASS-B-PORTING.md` | 本报告 |
| `reports/CLASS-B-PROBE.md` | 编译探针报告（由验证脚本生成） |
| `docs/GITHUB-SOURCES-CLASS-B.md` | 仓库调研、通道经验、Collabora 补丁否定结论 |
| `docs/CLASS-B-ADAPTATION.md` | 适配与接线指南 |

**同步更新的既有文档**（边界段落改为「B 类已闭环」）：
`README.md`、`docs/UNRESOLVED-SYMBOLS.md` §6、
`docs/PLATFORM-IMPL-PORTING.md` §8、`reports/PLATFORM-IMPL.md`（追加勘误）。

## 7. 对历史结论的勘误（追加，不静默改写）

| 位置 | 原文 | 本轮修正 |
|---|---|---|
| `docs/PLATFORM-IMPL-PORTING.md` §8 | 「`clk`/`iommu`/`devfreq` …（报告中 B 类）需要单独适配，不建议整体并入主线目录」 | 判断仍成立（确实不整体并入），但**已在 stage-5 用「函数级移植 + 主线桥接」闭环**，不再是未完成的边界 |
| `docs/UNRESOLVED-SYMBOLS.md` §6 | 「B 类（clk/iommu/devfreq）框架符号由目标树『已修基线层』提供——这部分不属本轮范围」 | stage-5 已在包内提供 7.3.3 可编译实现；目标树只需接线 |
| `reports/PLATFORM-IMPL.md` §2 | 4 个 B 类符号标「目标树基线层/需桩」 | 追加勘误段：全部有实现，见 `CLASS-B-SYMBOLS.md` |

原报告正文保留作审计对照，仅在文末追加勘误段（与 stage-4 做法一致）。

## 8. 边界（未变）

- 本包交付**源码 / 头 / 脚本 / 配置 / 报告**，不含闭源固件二进制；
  固件仍由 `init/` 从原厂 vendor 提取。
- **真机未验证**；全部是编译期语法验证、`nm` 符号核验与源码级适配。
- 本包是独立素材目录，**未编入**既有 7.3.3 交付内核的 Image/dtb，
  不改变 `../verify.sh` 原有的 dtc / GPU / USB 口径。
- 路径 B 里那批 4.14 私有驱动若真要整体移植，仍需目标树自行完成
  4.14→7.3 的 API 适配与全量链接——本包只保证路径 A 可链接。
- Collabora MT6893 主线 clk 补丁未合入公开 GitHub 分支，本包未擅自引入。

## 9. 复现命令

```bash
cd wifi-vendor-port

# 1) 【可选·审计复现】重新保真导出（需 lineage-23.2 稀疏克隆 + 联网）
python3.11 tools/export_framework.py \
    --root framework --manifest tools/framework-manifest.tsv \
    --vendor-repo /tmp/mt6893-lin232 --upstream-ref master

# 2) 一键验证（仅需 Linux 7.3.3 内核树 + 包内文件，可离线）
sh solution/verify-class-b.sh /path/to/linux-7.3.3

# 3) stage-4 的三个符号（互不影响）
sh solution/verify.sh /path/to/linux-7.3.3
```
