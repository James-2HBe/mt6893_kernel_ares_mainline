# stage-5 GitHub 开源仓库调研（B 类框架符号）

> 目标（用户）：「请移植 B 类（clk/iommu/devfreq）框架符号，通过 github 上的
> 开源仓库」。本文件记录**选了哪些 GitHub 仓库、为什么选、以及一条重要的
> 否定结论**（Collabora 的 MT6893 主线 clk 补丁未合入任何公开 GitHub 分支）。

## 1. 结论先行

| 用途 | GitHub 仓库 | 固定 ref | 导出 |
|---|---|---|---|
| **主线框架等价实现（锚点）** | `torvalds/linux` | `master` | `framework/upstream/`，10 文件 |
| **4.14 私有符号真实实现 + MT6893 variant 链（语义基准）** | `xiaomi-mt6893-dev/kernel_xiaomi_mt6893` | `lineage-23.2` @ `aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f` | `framework/vendor/`，16 文件 |
| 平台基线头（stage-2 已用，本轮交叉引用） | `MiCode/Xiaomi_Kernel_OpenSource` | `ares-r-oss` @ `49c02c2d…` | `baseline/`，包内已有 |

逐文件 `git blob sha` 校验 **26/26 通过**（`tools/framework-manifest.tsv`）。
包内保真副本自包含，目标树接线**无需联网**。

stage-3/4 已用过 `xiaomi-mt6893-dev/...@lineage-23.2`（导出 21 个实现 `.c`、
137 个变体头、ram_console 保真副本）。本轮**新引入 `torvalds/linux@master`
作为 GitHub 开源源**，并补导此前遗漏的 MT6893 variant / Makefile / 私有头
完整链，使 B 类符号的「4.14 实现 ↔ 主线等价」两端都有可审计的源码依据。

## 2. 为什么用主线 Linux 作源（而不是再找一棵 4.14 树）

实测目标树 `/workspace/linux-7.3.3`（主线 7.3.3）**已原生支持 MT6893 的
SMI / IOMMU / DVFSRC / PM-domain / clk 框架**：

| 子系统 | 7.3.3 主线证据 |
|---|---|
| SMI | `drivers/memory/mtk-smi.c:286` `mtk_smi_larb_mt6893_ostd`、`:501` `mtk_smi_larb_mt6893`、`:553` `"mediatek,mt6893-smi-larb"`、`:779` `mtk_smi_common_mt6893`、`:848` `"mediatek,mt6893-smi-common"`；`:694/:711` `mtk_smi_larb_resume/suspend` |
| IOMMU | `drivers/iommu/mtk_iommu.c:1616` `mt6893_data`、`:1906` `"mediatek,mt6893-iommu-mm"` |
| DVFSRC | `drivers/soc/mediatek/mtk-dvfsrc.c:188` `dvfsrc_get_current_opp()->vcore_opp`、`:290-337` current/vcore level；`drivers/regulator/mtk-dvfsrc-regulator.c` |
| PM domain | `drivers/pmdomain/mediatek/mt6893-pm-domains.h`、`mtk-pm-domains.c:975-976` `genpd.power_off/power_on` |
| clk 框架 | `drivers/clk/mediatek/clk-mtk.c:571` `clk_notifier_func`、`clk-mux.c:433-440` `devm_mtk_clk_mux_notifier_register` |

其中 SMI/IOMMU 的 MT6893 支持来自 2025 年主线合入：
- `iommu/mediatek: Add support for Dimensity 1200 MT6893 MM IOMMU`
  （commit `f6a1e89ab6e3…`，AngeloGioacchino Del Regno，2025-04）
- `memory: mtk-smi: Add support for Dimensity 1200 MT6893 SMI`（v6.16 合入）

**因此正确的「移植」不是把 4.14 私有驱动搬进主线，而是让私有栈的链接期
符号接到这套已存在的主线框架上。** 主线源码（`torvalds/linux`，GitHub 开源
仓库）就是这套框架的权威来源，故把它保真导出为 `framework/upstream/`。

## 3. 一条重要的否定结论：Collabora MT6893 clk 补丁未进公开 GitHub 分支

调研中发现 2025-04-10 Collabora 投出的补丁系列
`[PATCH v1 0/3] MediaTek Dimensity 1200 - Add Clocks support`
（AngeloGioacchino Del Regno，为 OnePlus Nord 2 / DN2103 主线化准备），
新增 15 文件 / +2455 行，包含：

```
drivers/clk/mediatek/clk-mt6893-topckgen.c    (880)
drivers/clk/mediatek/clk-mt6893-apmixedsys.c  (137)
drivers/clk/mediatek/clk-mt6893-infra_ao.c    (185)
drivers/clk/mediatek/clk-mt6893-{mm,mdp,mfg,vdec,venc,scp_adsp,imp_iic_wrap}.c
include/dt-bindings/clock/mediatek,mt6893-clk.h (449)
Documentation/devicetree/bindings/clock/mediatek,mt6893-*.yaml
```

这套是**主线风格**（`mtk_clk_simple_probe` / `mtk_clk_desc` /
`clk_notifier_func`），确实是与 7.3.3 最接近的 MT6893 clk 驱动。但逐一验证
后判定**不能作为本轮保真源**：

| 探测目标 | 结果 |
|---|---|
| `api.github.com/repos/torvalds/linux/contents/drivers/clk/mediatek?ref=master` | 221 条目，**无 6893/6873 文件** |
| `torvalds/linux@v6.16` 同路径 | 404 |
| `gregkh/linux@linux-6.16.y` | 404 |
| `angelogioacchino/linux` @ master / main | 404 |
| `yassineoudjana/linux` @ main | 404 |
| `xdarklight/linux` @ master | 404 |
| `mt6893-mainline/linux` @ main | 404 |

即：**该补丁只存在于邮件列表（lore.kernel.org / patchwork / LWN），尚未合入
任何可公开访问的 GitHub 仓库分支**。`master` 上没有，`v6.16` tag 上也没有。
本轮因此不伪造出处，仅把它记为「主线化方向参考」；若将来合入主线，
`framework/upstream/` 的导出清单加几行即可纳入（导出器已参数化）。

> 这也解释了为什么 7.3.3 树里 `drivers/clk/mediatek/` 有 219 个 `.c`、
> 有完整的 `clk-mt8192-*` 系列（MT8192 与 MT6893 高度兼容），却**没有**
> `clk-mt6893-*.c`。本轮不擅自把邮件列表补丁当作「已合入主线」塞进内核树。

## 4. 通道经验（本环境实测）

| 通道 | 结果 | 采用 |
|---|---|---|
| `raw.githubusercontent.com/torvalds/linux/master/<path>` | **HTTP 200**，字节正确 | 主线主通道 ✅ |
| `api.github.com/repos/torvalds/linux/contents/<path>?ref=master` | 返回 blob `sha` / `size` | 主线取 sha ✅ |
| `api.github.com/repos/torvalds/linux/git/blobs/<sha>` | base64 content，解码后字节一致 | 主线备通道 ✅ |
| `cdn.jsdelivr.net/gh/torvalds/linux@master/...` | **403** `Package size exceeded the configured limit of 50 MB` | ❌ 弃用 |
| `github.com/torvalds/linux/raw/refs/heads/master/...` | 302 空内容（未跟随到正确 CDN） | ❌ |
| `cdn.jsdelivr.net/gh/xiaomi-mt6893-dev/kernel_xiaomi_mt6893@<commit>/<path>` | HTTP 200 | vendor 主通道 ✅（与 stage-3 一致） |
| `fastly.jsdelivr.net/gh/...` | 可用 | vendor 备通道 ✅ |

校验统一用 git object hash：
`sha1(b"blob %d\0" % len(data) + data)` 与 `git ls-tree` / GitHub contents API
的 blob sha 比对。**26/26 全部一致**。

> 与 stage-2 的通道差异：ares-r-oss 用 `raw.githubusercontent.com/<commit>/<path>`
> 成功；torvalds/linux 的 `master` 分支同样可用 raw（此前记录的「raw 超时」是
> 并发/瞬时问题，本轮低并发 + 重试后稳定）。jsdelivr 对超大仓库（torvalds/linux）
> 不可用，但对 lineage-23.2 这类裁剪/合并树可用——这是本轮新增的经验。

## 5. 复现命令

```bash
# 1) vendor 侧稀疏克隆（只需 tree，不取全部 blob）
git clone --filter=blob:none --no-checkout --depth=1 --single-branch \
    --branch lineage-23.2 \
    https://github.com/xiaomi-mt6893-dev/kernel_xiaomi_mt6893.git \
    /tmp/mt6893-lin232

# 2) 保真导出 26 文件（主线 10 + vendor 16）+ manifest
cd wifi-vendor-port
python3.11 tools/export_framework.py \
    --root framework --manifest tools/framework-manifest.tsv \
    --vendor-repo /tmp/mt6893-lin232 --upstream-ref master

# 3) 离线验证（只需 7.3.3 内核树 + 包内文件）
sh solution/verify-class-b.sh /path/to/linux-7.3.3
```

`tools/export_framework.py` 的两个文件清单（`UPSTREAM_FILES` /
`VENDOR_FILES`）是显式白名单，新增主线化 clk 驱动时只需追加路径即可。
