# B 类（clk/iommu/devfreq）框架符号移植素材（stage-5）

本目录是**保真源码副本 + 接线说明**，全部来自 GitHub 开源仓库，逐文件用
`sha1("blob <len>\0" + data)` 与 `git ls-tree` / GitHub contents API 的 blob sha
比对通过（26/26，见 `tools/framework-manifest.tsv`）。**包内自包含，目标树接线
不需要联网**；`tools/export_framework.py` 仅用于审计复现。

## 目录结构

| 路径 | 来源 | 用途 |
|---|---|---|
| `upstream/` | `torvalds/linux@master`（10 文件） | 7.3.3 主线里 B 类符号的**等价框架实现**，作为「为什么不该把 4.14 私有驱动整体并入」的证据与接线锚点 |
| `vendor/` | `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2` `aeb1327b…`（16 文件） | 4.14 私有 B 类符号**真实实现** + MT6893 variant / Makefile / 私有头，语义基准与路径 B 素材 |
| `Kconfig.fragment` | 本包 | 目标树 Kconfig 接线 |
| `Makefile.fragment` | 本包 | 目标树 Makefile 接线（含路径 A / B 说明） |

`upstream/` 与 `vendor/` 里的文件**按源树原路径存放**，均为只读审计副本，
不要直接编辑；实际参与编译的是 `../solution/framework-symbols.c`。

## 5 个 B 类符号的落点

| 符号 | 子系统 | 4.14 实现（`vendor/`） | 7.3.3 主线等价（`upstream/`） | 本包实现 |
|---|---|---|---|---|
| `register_pg_callback` | clk / power-gating | `clk-mt6873-pg.c:827` | `clk-mtk.c` `clk_notifier_func` / `clk-mux.c` `devm_mtk_clk_mux_notifier_register`；`mtk-pm-domains.c` `power_on/off` | `solution/framework-symbols.c` |
| `smi_reg_backup_sec` | iommu / SMI | `pseudo_m4u.c:247` | `mtk-smi.c` `mtk_smi_larb_suspend` + `pm_runtime_force_suspend` | 同上 |
| `smi_reg_restore_sec` | iommu / SMI | `pseudo_m4u.c:262` | `mtk-smi.c` `mtk_smi_larb_resume` + `pm_runtime_force_resume` | 同上 |
| `get_cur_vcore_opp` | devfreq / DVFSRC | `helio-dvfsrc-opp.c:76` | `mtk-dvfsrc.c` `dvfsrc_get_current_opp()->vcore_opp`；`mtk-dvfsrc-regulator.c` | 同上 |
| `aee_rr_rec_clk` | clk（调试） | `mtk_ram_console.c:1129`（`baseline/reference/`） | 无（纯调试记录，不影响功能） | `solution/platform-symbols.c`（stage-4 已解决） |

## 一条重要结论

7.3.3 主线**已原生支持 MT6893 的 SMI / IOMMU / DVFSRC / PM-domain**：

- `mtk-smi.c`：`mediatek,mt6893-smi-larb` / `mediatek,mt6893-smi-common`
- `mtk_iommu.c`：`mt6893_data` / `mediatek,mt6893-iommu-mm`
- `mtk-dvfsrc.c` + `mtk-dvfsrc-regulator.c`：VCORE OPP
- `mt6893-pm-domains.h` + `mtk-pm-domains.c`：power domain

因此把 4.14 的 `clk-mt6873-pg.c` / `pseudo_m4u.c` / `helio-dvfsrc-opp.c`
整体搬进主线同名目录会**重复定义框架结构体与全局符号**，且引入 7.3 已不存在
的 TEE / SSPM IPI 私有运行时。这正是 stage-3 把它们标为 **B 类**并留给目标树
的原因。本轮的解法是「函数级移植 + 主线语义桥接」，不是整体搬移。

## 复现导出

```bash
# 需已有 lineage-23.2 稀疏克隆（仅取 tree，不取全部 blob）
git clone --filter=blob:none --no-checkout --depth=1 --single-branch \
    --branch lineage-23.2 https://github.com/xiaomi-mt6893-dev/kernel_xiaomi_mt6893.git \
    /tmp/mt6893-lin232

python3.11 tools/export_framework.py \
    --root framework --manifest tools/framework-manifest.tsv \
    --vendor-repo /tmp/mt6893-lin232
```

通道说明：`cdn.jsdelivr.net` 对 `torvalds/linux` 返回
`403 Package size exceeded 50 MB`，故主线走 `api.github.com` 取 sha +
`raw.githubusercontent.com` 下载 + GitHub blobs API(base64) 兜底；vendor 侧沿用
stage-3 验证过的 jsdelivr CDN + blob-sha 校验。
