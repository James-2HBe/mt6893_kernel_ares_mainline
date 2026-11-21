# 链接期平台符号调用计数复核（注释误统计修正）

由 `tools/audit_symbol_refs.py` 生成，只读复核，**不改动** `reports/BASELINE-MODULES.md`，旧表保留作审计对照。

## 1. 问题

`tools/scan_baseline_modules.py::count_refs`（stage-2）直接对源码文本
匹配 `\b(name)\s*\(`，**未剥离 C 注释，也未剔除函数声明行**。因此：

- `adapted/mmc/host/mediatek/ComboA/*/msdc_cust.c` 中 7 行
  `/* buck_set_mode(VCORE, enable); */` 被计成 7 次真实调用；
- 头文件中 `extern`/`static inline` 声明也被计入。

本工具口径：**剥离块注释与行注释 → 剔除以 `;` 结尾的声明行 → 再统计**，
并把扫描范围从 `adapted/` 扩展到 `adapted/ + baseline/impl/`
（stage-3 导出的平台实现 .c 也在其中）。

| 指标 | 值 |
|---|---|
| 旧表符号数 | 33 |
| 计数发生变化的符号 | 26 |
| 修正后真实调用数为 0（可从链接期清单移除） | 2 |

## 2. 计数变化明细（旧 → 新）

| 符号 | 旧计数 | 修正后 | 说明 |
|---|---|---|---|
| `buck_set_mode` | 7 | **0** | 全部为注释/声明，无真实调用 |
| `clk_buf_ctrl` | 1 | **4** | 含声明或注释，真实调用 4 处 |
| `clk_buf_get_xo_en_sta` | 1 | **3** | 含声明或注释，真实调用 3 处 |
| `clk_buf_show_status_info` | 1 | **4** | 含声明或注释，真实调用 4 处 |
| `force_get_tbat` | 1 | **0** | 全部为注释/声明，无真实调用 |
| `freqhopping_config` | 2 | **4** | 含声明或注释，真实调用 4 处 |
| `get_cur_vcore_opp` | 13 | **14** | 含声明或注释，真实调用 14 处 |
| `lbat_user_register` | 2 | **4** | 含声明或注释，真实调用 4 处 |
| `mt_ppm_sysboost_core` | 2 | **3** | 含声明或注释，真实调用 3 处 |
| `mt_ppm_sysboost_freq` | 2 | **4** | 含声明或注释，真实调用 4 处 |
| `mt_ppm_sysboost_set_core_limit` | 2 | **3** | 含声明或注释，真实调用 3 处 |
| `mt_ppm_sysboost_set_freq_limit` | 2 | **5** | 含声明或注释，真实调用 5 处 |
| `mtk_emimpu_free_region` | 15 | **17** | 含声明或注释，真实调用 17 处 |
| `mtk_emimpu_init_region` | 15 | **17** | 含声明或注释，真实调用 17 处 |
| `mtk_emimpu_lock_region` | 9 | **11** | 含声明或注释，真实调用 11 处 |
| `mtk_emimpu_postclear_register` | 1 | **2** | 含声明或注释，真实调用 2 处 |
| `mtk_emimpu_prehandle_register` | 1 | **2** | 含声明或注释，真实调用 2 处 |
| `mtk_emimpu_set_addr` | 15 | **17** | 含声明或注释，真实调用 17 处 |
| `mtk_emimpu_set_apc` | 33 | **35** | 含声明或注释，真实调用 35 处 |
| `mtk_emimpu_set_protection` | 15 | **18** | 含声明或注释，真实调用 18 处 |
| `pmic_ldo_vfe28_lp` | 1 | **2** | 含声明或注释，真实调用 2 处 |
| `ppm_game_mode_change_cb` | 1 | **2** | 含声明或注释，真实调用 2 处 |
| `register_pg_callback` | 1 | **2** | 含声明或注释，真实调用 2 处 |
| `smi_reg_backup_sec` | 5 | **3** | 含声明或注释，真实调用 3 处 |
| `smi_reg_restore_sec` | 5 | **3** | 含声明或注释，真实调用 3 处 |
| `topo_ctrl_get_nr_clusters` | 7 | **8** | 含声明或注释，真实调用 8 处 |

## 3. 两个「未开源符号」专项结论

### `buck_set_mode`

出现 7 行：注释 7 / 声明 0 / **真实调用 0**。修正后计数 **0**（旧表 7）。

| 位置 | 类型 | 内容 |
|---|---|---|
| `adapted/mmc/host/mediatek/ComboA/mt6873/msdc_cust.c:388` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6877/msdc_cust.c:392` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6853/msdc_cust.c:388` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6885/msdc_cust.c:417` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6893/msdc_cust.c:388` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6781/msdc_cust.c:406` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |
| `adapted/mmc/host/mediatek/ComboA/mt6833/msdc_cust.c:393` | COMMENT | `/* buck_set_mode(VCORE, enable); */` |

### `aee_rr_rec_clk`

出现 3 行：注释 0 / 声明 2 / **真实调用 1**。修正后计数 **2**（旧表 2）。

| 位置 | 类型 | 内容 |
|---|---|---|
| `adapted/misc/mediatek/include/mt-plat/mtk_ram_console.h:65` | DECL | `extern void aee_rr_rec_clk(int id, u32 val);` |
| `adapted/misc/mediatek/include/mt-plat/mtk_ram_console.h:246` | INLINE-IMPL | `static inline void aee_rr_rec_clk(int id, u32 val)` |
| `baseline/impl/drivers/clk/mediatek/clk-mt6873-pg.c:1100` | CODE | `aee_rr_rec_clk(j, data[j]);` |

## 4. 复现

```bash
python3 tools/audit_symbol_refs.py \
    --modules reports/BASELINE-MODULES.md \
    --adapted adapted --impl baseline/impl \
    --report reports/SYMBOL-REFS-CORRECTED.md
```
