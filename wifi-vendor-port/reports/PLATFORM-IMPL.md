# 平台符号实现移植报告（ares → 7.3.3）

由 `tools/port_platform_impl.py` 生成。来源：`xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f`（固定 commit）。

通道：`cdn.jsdelivr.net`（主）→ `fastly.jsdelivr.net`（备）→ `github.com/.../raw`（兜底）；逐文件用 `git ls-tree` 的 blob sha 校验。`raw.githubusercontent.com` 在本环境实测超时，已弃用。

## 1. 为什么选这棵树

- `ares-r-oss` 是小米官方 GPL 树，`connectivity/` 只有空壳，且只带 `mt6885-*` 变体头；上轮已用它导出 36 个平台头与 182 个构建文件，**但未导出任何平台实现 `.c`**。
- 本树 `lineage-23.2` 是 4.14 + 大量高版本 `BACKPORT:`/`UPSTREAM:` 回移的合并维护树（覆盖 ares/agate/pissarro/choppin），相对 ares-r-oss 多 5032 个路径（2291 头 / 2073 源），含完整 `connectivity/` 私有栈（473 个 `.c`）与 MT6893 变体头。
- 其 API 写法比原生 4.14 更接近新内核，移植到 7.3.3 的差异面更小。

| 指标 | 值 |
|---|---|
| 来源 commit | `aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f` |
| 候选实现 `.c` / 下载成功 | 397 / 413 |
| 平台符号 | 33 |
| 定位到实现的符号 | **28** |
| 导出实现 `.c` | 18 |
| 残余缺头可补齐 | **18 / 47** |

## 2. 33 个平台导出符号 → 实现落点

`类` 列：**A** = 弱耦合的 MTK 私有实现（7.3.3 无同名子系统，可整体移植）；**B** = 与主线 clk/iommu 框架强耦合，需单独适配或提供兼容桩，不建议整体并入主线目录。

| 符号 | 调用 | 类 | 实现文件 | 依据 | 导出状态 |
|---|---|---|---|---|---|
| `mtk_emimpu_set_apc` | 33 | A | `drivers/memory/mediatek/emimpu.c:797` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mtk_emimpu_init_region` | 15 | A | `drivers/memory/mediatek/emimpu.c:726` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mtk_emimpu_set_addr` | 15 | A | `drivers/memory/mediatek/emimpu.c:764` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mtk_emimpu_set_protection` | 15 | A | `drivers/memory/mediatek/emimpu.c:861` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mtk_emimpu_free_region` | 15 | A | `drivers/memory/mediatek/emimpu.c:743` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `get_cur_vcore_opp` | 13 | A | `drivers/devfreq/helio-dvfsrc-opp.c:76` | DEF（另 3 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `mtk_emimpu_lock_region` | 9 | A | `drivers/memory/mediatek/emimpu.c:811` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `topo_ctrl_get_nr_clusters` | 7 | A | `drivers/misc/mediatek/performance/boost_ctrl/topo_ctrl/topo_ctrl.c:54` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `buck_set_mode` | 7 | A | — | 未定位 | 目标树基线层/需桩 |
| `smi_reg_restore_sec` | 5 | B | — | 未定位 | 目标树基线层/需桩 |
| `smi_reg_backup_sec` | 5 | B | — | 未定位 | 目标树基线层/需桩 |
| `spm_resource_req` | 2 | A | `drivers/misc/mediatek/base/power/spm_v4/mtk_spm_resource_req.c:96` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `mt_ppm_sysboost_freq` | 2 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_policy_sys_boost.c:222` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `mt_ppm_sysboost_core` | 2 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_policy_sys_boost.c:163` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `mt_ppm_sysboost_set_core_limit` | 2 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_policy_sys_boost.c:229` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `mt_ppm_sysboost_set_freq_limit` | 2 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_policy_sys_boost.c:291` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `freqhopping_config` | 2 | A | `drivers/misc/mediatek/freqhopping/mtk_freqhopping_drv.c:578` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `lbat_user_register` | 2 | A | `drivers/misc/mediatek/pmic/common/upmu_lbat_service.c:320` | EXPORT_SYMBOL（另 1 个 variant） | 已有 `EXPORT_SYMBOL` |
| `aee_rr_rec_clk` | 2 | B | — | 未定位 | 目标树基线层/需桩 |
| `clk_buf_ctrl` | 1 | A | `drivers/misc/mediatek/base/power/clkbuf_v1/mt6873/mtk_clkbuf_hw.c:847` | EXPORT_SYMBOL（另 2 个 variant） | 已有 `EXPORT_SYMBOL` |
| `clk_buf_show_status_info` | 1 | A | `drivers/misc/mediatek/base/power/clkbuf_v1/mt6873/mtk_clkbuf_hw.c:1174` | DEF（另 2 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `clk_buf_get_xo_en_sta` | 1 | A | `drivers/misc/mediatek/base/power/clkbuf_v1/mt6853/mtk_clkbuf_hw.c:1208` | EXPORT_SYMBOL（另 2 个 variant） | 已有 `EXPORT_SYMBOL` |
| `pmic_ldo_vcn18_lp` | 1 | A | `drivers/misc/mediatek/pmic/mt6357/v1/pmic_lp_api.c:286` | DEF（另 4 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `pmic_ldo_vfe28_lp` | 1 | A | `drivers/misc/mediatek/pmic/mt6357/v1/pmic_lp_api.c:222` | DEF（另 4 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `register_pg_callback` | 1 | B | `drivers/clk/mediatek/clk-mt6873-pg.c:827` | DEF（另 3 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `mtk_emimpu_prehandle_register` | 1 | A | `drivers/memory/mediatek/emimpu.c:910` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mtk_emimpu_postclear_register` | 1 | A | `drivers/memory/mediatek/emimpu.c:928` | EXPORT_SYMBOL | 已有 `EXPORT_SYMBOL` |
| `mt_usb_is_device` | 1 | A | — | 未定位 | 目标树基线层/需桩 |
| `mt_ppm_userlimit_freq_limit_by_others` | 1 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_dummy.c:92` | DEF（另 1 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `ppm_game_mode_change_cb` | 1 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_main.c:576` | DEF（另 1 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `mt_ppm_userlimit_cpu_freq` | 1 | A | `drivers/misc/mediatek/base/power/ppm_v3/src/mtk_ppm_dummy.c:86` | DEF（另 1 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `force_get_tbat` | 1 | A | `drivers/misc/mediatek/power/mt6873/mtk_battery_intf.c:136` | DEF（另 6 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |
| `vcore_pmic_set_mode` | 1 | A | `drivers/misc/mediatek/pmic/mt6390/v1/pmic.c:72` | DEF（另 1 个 variant） | **需补 `EXPORT_SYMBOL_GPL`** |

> 已有 `EXPORT_SYMBOL` 的符号 **18/33**；其余在适配版中补齐。

未在候选树定位到实现的符号（5）：`buck_set_mode`, `smi_reg_restore_sec`, `smi_reg_backup_sec`, `aee_rr_rec_clk`, `mt_usb_is_device`。

## 3. 残余变体头补齐

| include（原缺失） | 新树定位路径 | 状态 |
|---|---|---|
| `soc/mediatek/emi.h` | `include/memory/mediatek/emi.h` | 已导出 |
| `conninfra_ext.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mtk_disp_notify.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `dt-bindings/memory/mt6877-larb-port.h` | `include/dt-bindings/memory/mt6877-larb-port.h` | 已导出 |
| `mtk_6306_gpio.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `clk-mt6873-pg.h` | `drivers/clk/mediatek/clk-mt6873-pg.h` | 已导出 |
| `mtk_hibernate_dpm.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mt_clkbuf_ctl.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mtk_low_battery_throttling.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `tc1_partition.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `dt-bindings/memory/mt6853-larb-port.h` | `include/dt-bindings/memory/mt6853-larb-port.h` | 已导出 |
| `dt-bindings/memory/mt6873-larb-port.h` | `include/dt-bindings/memory/mt6873-larb-port.h` | 已导出 |
| `mach/mt_reg_base.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mach/mt_irq.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mach/mt_clkmgr.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `platform_mt6885.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `platform_mt6983.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `reset_fsm.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `reset_fsm_def.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `emi_symbol_hook.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-v6877.h` | `drivers/devfreq/helio-dvfsrc-opp-v6877.h` | 已导出 |
| `clk-mt6833-pg.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `clk-mt6853-pg.h` | `drivers/clk/mediatek/clk-mt6853-pg.h` | 已导出 |
| `clk-mt6877-pg.h` | `drivers/clk/mediatek/clk-mt6877-pg.h` | 已导出 |
| `mach/emi_mpu.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `mach/power_loss_test.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `dt-bindings/mmc/mt6873-msdc.h` | `include/dt-bindings/mmc/mt6873-msdc.h` | 已导出 |
| `dt-bindings/clock/mt6873-clk.h` | `include/dt-bindings/clock/mt6873-clk.h` | 已导出 |
| `dt-bindings/mmc/mt6877-msdc.h` | `include/dt-bindings/mmc/mt6877-msdc.h` | 已导出 |
| `dt-bindings/clock/mt6877-clk.h` | `include/dt-bindings/clock/mt6877-clk.h` | 已导出 |
| `dt-bindings/mmc/mt6853-msdc.h` | `include/dt-bindings/mmc/mt6853-msdc.h` | 已导出 |
| `dt-bindings/clock/mt6853-clk.h` | `include/dt-bindings/clock/mt6853-clk.h` | 已导出 |
| `helio-dvfsrc-opp-mt6775.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6771.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6768.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6765.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6785.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6873.h` | `drivers/devfreq/helio-dvfsrc-opp-mt6873.h` | 已导出 |
| `helio-dvfsrc-opp-mt6833.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-opp-mt6853.h` | `drivers/devfreq/helio-dvfsrc-opp-mt6853.h` | 已导出 |
| `spm_v2/mtk_spm_resource_req.h` | `drivers/misc/mediatek/base/power/include/mtk_spm_resource_req.h` | 已导出 |
| `spm_v3/mtk_spm_resource_req.h` | `drivers/misc/mediatek/base/power/include/mtk_spm_resource_req.h` | 已导出 |
| `helio-dvfsrc-mt6768.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-mt6765.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-mt67xx.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-mt6775.h` | — | 两树均不含（裁剪树内部/条件分支） |
| `helio-dvfsrc-mt6771.h` | — | 两树均不含（裁剪树内部/条件分支） |

导出位置：`baseline/platform/<原路径>`；裸 include 另复制一份到 `baseline/platform/flat/`（6 个），保证 `-I` 直接命中。

## 4. 完整性与复现

- `tools/platform-impl-manifest.tsv`：逐文件 `path / blob sha / size / kind`。
- `baseline/impl/`：保真实现 `.c`（按原路径，勿手改，审计基准）。
- `baseline/platform/`：变体头（保真 + flat 副本）。
- 适配版由 `tools/adapt_platform_impl.py` 生成到 `baseline/impl-adapted/`。

```bash
python3.11 tools/port_platform_impl.py \
    --repo /tmp/mt6893-lin232 --files /tmp/files_lin232.txt \
    --modules reports/BASELINE-MODULES.md \
    --export-report reports/BASELINE-EXPORT.md \
    --out baseline --report reports/PLATFORM-IMPL.md \
    --manifest tools/platform-impl-manifest.tsv
```

---

## 勘误（stage-4）：两个「未定位/需桩」符号已解决

上表 §2 与「未在候选树定位到实现的符号（5）」中的
`aee_rr_rec_clk`、`buck_set_mode` 两行，stage-4 复核后修正如下：

| 符号 | 修正后结论 | 落点 |
|---|---|---|
| `aee_rr_rec_clk` | 有真实开源实现（stage-3 关键词漏搜 `ram_console/` 路径） | `ram_console/mtk_ram_console.c:1129`，保真副本 `baseline/reference/`；函数级移植 `solution/platform-symbols.c` |
| `buck_set_mode` | 7 次「调用」全是注释、MT6335 专用废 API、真实调用 0 | 从链接期清单剔除；`solution/platform-symbols.c` 给 `-EOPNOTSUPP` 兜底 |

其余 3 个「未定位」符号（`smi_reg_restore_sec` / `smi_reg_backup_sec` /
`mt_usb_is_device`）在 stage-3 二次追查中已定位实现（`pseudo_m4u.c` / `mtu3_dr.c`）。

两个符号均已用 7.3.3 头文件实际编译 + `nm` 核验（`__weak + EXPORT_SYMBOL_GPL`），
详见 `docs/UNRESOLVED-SYMBOLS.md`、`reports/SYMBOL-REFS-CORRECTED.md`、
`reports/SOLUTION-PROBE.md`。上表原文保留作审计对照。

---

## 勘误（stage-5）：B 类框架符号已闭环

上表 §2「类」列标 **B** 的 4 个符号（`smi_reg_restore_sec`、
`smi_reg_backup_sec`、`aee_rr_rec_clk`、`register_pg_callback`），以及用户
口径纳入 B 类的 `get_cur_vcore_opp`（devfreq，原表标 A），此前状态是
「目标树基线层/需桩」或「有 4.14 实现 `.c` 但强耦合、未做 7.3 移植」。
stage-5 已全部闭环：

| 符号 | stage-5 落点 |
|---|---|
| `register_pg_callback` | `solution/framework-symbols.c` 函数级移植 + `mtk_pg_notify` / `mtk_pg_debug_dump` 桥接 |
| `smi_reg_backup_sec` / `smi_reg_restore_sec` | 同上，桥接到主线 `mtk-smi.c` runtime PM（+ 可选 backend 注入） |
| `get_cur_vcore_opp` | 同上，保留 `VCORE_OPP_UNREQ` 哨兵 + `mtk_framework_set_vcore_opp` 同步入口 |
| `aee_rr_rec_clk` | stage-4 `solution/platform-symbols.c`（本轮不重复定义） |

来源为 GitHub 开源仓库 `torvalds/linux@master`（10 文件）与
`xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`（16 文件），
26/26 逐文件 blob-sha 校验通过，见 `tools/framework-manifest.tsv` 与
`framework/`。实现全部 `__weak + EXPORT_SYMBOL_GPL`，7.3.3 实测
`-c` 编译 0 error / 0 warning，`nm` / 双配置 / `ld.lld -r` 强符号覆盖 /
编译探针 5 项全绿（`solution/verify-class-b.sh`）。

**「B 类不建议整体并入主线目录」的技术判断不变**——本轮做的是函数级
移植 + 主线语义桥接，不是把 4.14 私有 clk/iommu/devfreq 驱动整体搬进
`drivers/clk|iommu|devfreq`（那样会与 7.3.3 主线框架重复定义）。
上表原文保留作审计对照；完整报告见 `reports/CLASS-B-PORTING.md`、
`reports/CLASS-B-SYMBOLS.md`。
