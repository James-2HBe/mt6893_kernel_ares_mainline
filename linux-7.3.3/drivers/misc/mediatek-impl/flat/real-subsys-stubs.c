/* SPDX-License-Identifier: GPL-2.0 */
/*
 * real-subsys-stubs.c — 阶段 D 从 4.19 原厂树搬入真实现后，为满足依赖闭包而
 * 补充的最小 weak 桩。这些符号的私有真实现（MT6893 Cobra 算法）依赖 EARA /
 * upower / SSPM IPI 子系统，主线缺失且 1-hop 闭包外，故用 __weak 安全兜底：
 * 真实平台若提供强符号则自动覆盖。目标仅为代码级自包含，不承诺真机功能。
 */
#include <linux/module.h>
#include <linux/seq_file.h>
#include "mtk_ppm_platform.h"

/* PPM Cobra 算法入口：真实现在 mach/mt6893/mtk_ppm_cobra_algo.c（依赖爆炸，未搬）*/
void __weak ppm_cobra_dump_tbl(struct seq_file *m) {}
EXPORT_SYMBOL_GPL(ppm_cobra_dump_tbl);

void __weak ppm_cobra_lookup_get_result(struct seq_file *m,
					enum ppm_cobra_lookup_type type) {}
EXPORT_SYMBOL_GPL(ppm_cobra_lookup_get_result);

/* Cobra 查表数据全局量（真实现由 cobra_algo.c 维护）*/
struct ppm_cobra_lookup __weak cobra_lookup_data;
EXPORT_SYMBOL_GPL(cobra_lookup_data);
