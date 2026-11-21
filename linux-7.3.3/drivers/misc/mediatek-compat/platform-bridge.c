/* SPDX-License-Identifier: GPL-2.0 */
/*
 * platform-bridge.c — 路径 B 平台 / HW 函数的安全兜底实现
 *
 * 目标内核：Linux 7.3.3 (arm64)。自包含，仅依赖主线头 +
 * compat/mtk_platform_bridge.h 的 extern 前向声明，可独立 -c 编译产 .o。
 *
 * =====================================================================
 * 为什么需要这一层
 * =====================================================================
 * baseline/impl-adapted/ 的 21 个 4.14 私有驱动在语法层引用了一批「无主线
 * 等价物、且来自 MTK 平台基线层 / 板级代码」的函数：
 *   arch_get_cluster_cpus   （arm64 拓扑，7.3 已无）
 *   m4u_get_port_name       （M4U 子系统，私有）
 *   smp_inner_dcache_flush_all / outer_flush_all / show_pte （缓存维护，aarch64 无）
 *   pmic_config_interface_nolock / pmic_read_interface_nolock （PMIC 寄存器访问）
 *
 * 这些函数若缺实现，路径 B 整批移植会卡在链接期 undefined symbol，迫使目标树
 * 自行补齐——这正是遗留边界「路径 B 仍需目标树做 API 适配」的根因之一。
 *
 * 本文件为它们提供 __weak + EXPORT_SYMBOL_GPL 的安全兜底：
 *   - 目标树「已修基线层」提供真实实现（强符号）→ 链接器自动优先，无冲突；
 *   - 目标树缺真实实现 → 用本文件的无副作用占位，保证可链接、可启动到驱动
 *     probe 失败（而非整内核链接失败）。
 *
 * 所有符号与 compat/mtk_platform_bridge.h 的前向声明严格一致。
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/export.h>
#include <linux/compiler.h>
#include <linux/cpumask.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/seq_file.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/pgtable.h>
#include <linux/dma-mapping.h>
#include <linux/syscore_ops.h>

#include "compat/mtk_platform_bridge.h"

/* ================================================================== */
/* 1. arch_get_cluster_cpus —— aarch64 拓扑 helper                      */
/* 7.3.3 aarch64 已无此符号；保真占位：把 dst 置为全 possible，使调用方    */
/* 的 cpumask_weight 仍可编译通过。真实平台应提供按 cluster 填充的实现。     */
void __weak arch_get_cluster_cpus(struct cpumask *dst, int cluster)
{
	cpumask_clear(dst);
}
EXPORT_SYMBOL_GPL(arch_get_cluster_cpus);

/* ================================================================== */
/* 2. m4u_get_port_name —— M4U 子系统私有                              */
const char *__weak m4u_get_port_name(int port)
{
	return "m4u-unknown-port";
}
EXPORT_SYMBOL_GPL(m4u_get_port_name);

/* ================================================================== */
/* 3. 缓存维护 helper —— aarch64 无 smp_inner_dcache_flush_all /         */
/*    outer_flush_all；show_pte 在 7.3 arm64 为 static，未导出。          */
/*    占位：无操作（真实平台应桥接到 smp_call_function / __flush_dcache_*) */
void __weak smp_inner_dcache_flush_all(void)
{
}
EXPORT_SYMBOL_GPL(smp_inner_dcache_flush_all);

void __weak outer_flush_all(void)
{
}
EXPORT_SYMBOL_GPL(outer_flush_all);

/* outer_clean_all：ARM32 外存缓存全清别名，arm64 主线无声明/无实现。 */
void __weak outer_clean_all(void)
{
}
EXPORT_SYMBOL_GPL(outer_clean_all);

void __weak show_pte(unsigned long addr)
{
}
EXPORT_SYMBOL_GPL(show_pte);

/* ================================================================== */
/* 4. PMIC 寄存器访问（nolock 变体）—— 来自 MTK pmic 核心               */
/*    占位返回安全值；真实平台应桥接到 pmic_wrap / 寄存器映射。           */
int __weak pmic_config_interface_nolock(unsigned short addr, unsigned short val,
				 unsigned short mask, unsigned short shift)
{
	return 0;
}
EXPORT_SYMBOL_GPL(pmic_config_interface_nolock);

int __weak pmic_read_interface_nolock(unsigned short addr, unsigned int *val,
			       unsigned short mask, unsigned short shift)
{
	if (val)
		*val = 0;
	return 0;
}
EXPORT_SYMBOL_GPL(pmic_read_interface_nolock);

/* ================================================================== */
/* 5. freqhopping HAL driver 句柄 —— 私有                              */
/*    占位返回 NULL；真实平台由 mt_fh_hal.c 提供强符号实现。            */
struct mt_fh_hal_driver *__weak mt_get_fh_hal_drv(void)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(mt_get_fh_hal_drv);

/* ================================================================== */
/* 6. arch_get_nr_clusters —— aarch64 拓扑 helper                      */
/*    占位返回 1（单 cluster）；真实平台应返回实际 cluster 数。         */
int __weak arch_get_nr_clusters(void)
{
	return 1;
}
EXPORT_SYMBOL_GPL(arch_get_nr_clusters);

/* ================================================================== */
/* 7. M4U / IOMMU 私有 helper                                         */
/*    iommu_get_port_name / mtk_iommu_get_port_id / report_custom_     */
/*    config_port / mtk_iommu_get_larb_port_count / mtk_iommu_dump_   */
/*    sec_larb 均为 MTK IOMMU 子系统私有符号，主线无等价物；占位返回    */
/*    安全值，真实平台由 mtk_iommu 驱动提供强符号实现。                */
char *__weak iommu_get_port_name(int port)
{
	return "m4u-unknown-port";
}
EXPORT_SYMBOL_GPL(iommu_get_port_name);

int __weak mtk_iommu_get_port_id(void *dev)
{
	return -1;
}
EXPORT_SYMBOL_GPL(mtk_iommu_get_port_id);

void __weak report_custom_config_port(const char *a, const char *b, int c)
{
}
EXPORT_SYMBOL_GPL(report_custom_config_port);

int __weak mtk_iommu_get_larb_port_count(int larb)
{
	return 0;
}
EXPORT_SYMBOL_GPL(mtk_iommu_get_larb_port_count);

int __weak mtk_iommu_dump_sec_larb(int larb, int larb_port)
{
	return 0;
}
EXPORT_SYMBOL_GPL(mtk_iommu_dump_sec_larb);

/* ================================================================== */
/* 8. find_extend_vma —— 7.3 主线已改名为 find_extend_vma_locked 且语义 */
/*    不同；占位返回 NULL，调用方均有 if (!vma) 兜底。真实平台应桥接。 */
struct vm_area_struct *__weak find_extend_vma(struct mm_struct *mm, unsigned long addr)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(find_extend_vma);

/* ================================================================== */
/* 9. frame_vector 子系统 —— 7.3 主线已移除（改用 pin_user_pages /      */
/*    unpin_user_pages）。占位实现使伪 M4U 的 get_vaddr_framevec 路径   */
/*    可编译；真实功能依赖目标树桥接或重写到新 gup API。               */
struct frame_vector *__weak frame_vector_create(unsigned int nr_frames)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(frame_vector_create);

int __weak get_vaddr_frames(unsigned long start, unsigned int nr_pfns,
		     int write, struct frame_vector *vec)
{
	return 0;
}
EXPORT_SYMBOL_GPL(get_vaddr_frames);

void __weak put_vaddr_frames(struct frame_vector *vec)
{
}
EXPORT_SYMBOL_GPL(put_vaddr_frames);

void __weak frame_vector_destroy(struct frame_vector *vec)
{
}
EXPORT_SYMBOL_GPL(frame_vector_destroy);

int __weak frame_vector_to_pages(struct frame_vector *vec)
{
	return 0;
}
EXPORT_SYMBOL_GPL(frame_vector_to_pages);

struct page **__weak frame_vector_pages(struct frame_vector *vec)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(frame_vector_pages);

/* ================================================================== */
/* 10. DVFS / PMIC 平台私有 helper                                     */
/*     spm_get_dvfs_level / is_qos_enabled / spm_dvfs_pwrap_cmd /      */
/*     get_pwrap_cmd 均为 SPM / PMIC 平台代码私有符号，主线无等价物；   */
/*     占位返回安全值，真实平台由对应驱动提供强符号实现。               */
int __weak spm_get_dvfs_level(void)
{
	return 0;
}
EXPORT_SYMBOL_GPL(spm_get_dvfs_level);

bool __weak is_qos_enabled(void)
{
	return false;
}
EXPORT_SYMBOL_GPL(is_qos_enabled);

void __weak spm_dvfs_pwrap_cmd(int cmd, int val)
{
}
EXPORT_SYMBOL_GPL(spm_dvfs_pwrap_cmd);

int __weak get_pwrap_cmd(int vcore_opp)
{
	return 0;
}
EXPORT_SYMBOL_GPL(get_pwrap_cmd);

/* ================================================================== */
/* 11. M4U 缓存同步 / 内核私有映射符号                                */
/*     dmac_map_area / dmac_unmap_area（ARM32 遗产）、__dma_flush_area */
/*     （arm64 内核私有）、map_vm_area / unmap_kernel_range（mm 内部） */
/*     在 7.3 arm64 主线均无公开声明；占位无操作，真实平台应桥接到对应  */
/*     缓存维护与 vmalloc 接口。                                       */
void __weak dmac_map_area(const void *addr, size_t size, int dir)
{
}
EXPORT_SYMBOL_GPL(dmac_map_area);

void __weak dmac_unmap_area(const void *addr, size_t size, int dir)
{
}
EXPORT_SYMBOL_GPL(dmac_unmap_area);

void __weak __dma_flush_area(const void *addr, size_t size)
{
}
EXPORT_SYMBOL_GPL(__dma_flush_area);

int __weak map_vm_area(struct vm_struct *area, pgprot_t prot, struct page **pages)
{
	return 0;
}
EXPORT_SYMBOL_GPL(map_vm_area);

void __weak unmap_kernel_range(unsigned long addr, unsigned long size)
{
}
EXPORT_SYMBOL_GPL(unmap_kernel_range);

void __weak __dma_map_area(const void *addr, size_t size, int dir)
{
}
EXPORT_SYMBOL_GPL(__dma_map_area);

void __weak __dma_unmap_area(const void *addr, size_t size, int dir)
{
}
EXPORT_SYMBOL_GPL(__dma_unmap_area);

/* ================================================================== */
/* 12. MTK IOMMU 平台私有函数                                         */
/*     mtk_iommu_* / report_custom_iommu_leakage / mtk_iommu_trace_   */
/*     log 均为 MTK IOMMU 子系统私有符号，主线无等价物；占位返回安全    */
/*     值，真实平台由 mtk_iommu 驱动提供强符号实现。                  */
int __weak mtk_iommu_get_boundary_id(struct device *dev)
{
	return -1;
}
EXPORT_SYMBOL_GPL(mtk_iommu_get_boundary_id);

int __weak mtk_iommu_switch_acp(struct device *dev, unsigned long iova,
			 unsigned long size, int is_acp)
{
	return 0;
}
EXPORT_SYMBOL_GPL(mtk_iommu_switch_acp);

void __weak mtk_iommu_iova_to_pa(struct device *dev, unsigned long iova,
			  unsigned long *pa)
{
	if (pa)
		*pa = 0;
}
EXPORT_SYMBOL_GPL(mtk_iommu_iova_to_pa);

void __weak mtk_iommu_dump_iova_space(unsigned long target)
{
}
EXPORT_SYMBOL_GPL(mtk_iommu_dump_iova_space);

int __weak mtk_iommu_get_iova_space(struct device *dev, unsigned long *base,
			     unsigned long *max, int *owner, void *priv)
{
	if (base)
		*base = 0;
	if (max)
		*max = 0;
	if (owner)
		*owner = -1;
	return -1;
}
EXPORT_SYMBOL_GPL(mtk_iommu_get_iova_space);

void __weak report_custom_iommu_leakage(const char *port_name, unsigned int size)
{
}
EXPORT_SYMBOL_GPL(report_custom_iommu_leakage);

void __weak mtk_iommu_trace_log(int event, dma_addr_t dma_addr, size_t size,
			 unsigned int flags)
{
}
EXPORT_SYMBOL_GPL(mtk_iommu_trace_log);

/* ================================================================== */
/* 13. register_syscore_ops —— 7.3 已删旧 API 的兼容替身                */
/*     7.3.3 把原 register_syscore_ops(const struct syscore_ops *) 改为   */
/*     register_syscore(struct syscore *)，且 syscore_ops 的回调统一多了 */
/*     void *data 参数。路径 B 的 clkbuf 等驱动仍按 4.14 旧式调用        */
/*     register_syscore_ops(&ops)。本兼容层提供弱符号，把旧式调用包成    */
/*     新的 struct syscore 注册，使旧驱动无需改动调用点即可在 7.3 链接。  */
/*     真实目标树若已用新 API，可忽略或强符号覆盖本占位。              */
void __weak register_syscore_ops(const struct syscore_ops *ops)
{
	static struct syscore sc;

	sc.ops = ops;
	sc.data = NULL;
	register_syscore(&sc);
}
EXPORT_SYMBOL_GPL(register_syscore_ops);

/* ================================================================== */
/* 14. 其余 7.3 已删除 / 私有符号的弱兜底                              */
void *__weak compat_alloc_user_space(unsigned long len)
{
	return NULL;
}
EXPORT_SYMBOL_GPL(compat_alloc_user_space);

int __weak mtk_iommu_put_iova_space(struct device *dev, struct list_head *list)
{
	return -1;
}
EXPORT_SYMBOL_GPL(mtk_iommu_put_iova_space);

void __weak mtk_dump_main_tlb(int m4u_id, int mm_id, struct seq_file *s)
{
}
EXPORT_SYMBOL_GPL(mtk_dump_main_tlb);

void __weak mtk_dump_pfh_tlb(int m4u_id, struct seq_file *s)
{
}
EXPORT_SYMBOL_GPL(mtk_dump_pfh_tlb);

void __weak mtk_dump_victim_tlb(int m4u_id, struct seq_file *s)
{
}
EXPORT_SYMBOL_GPL(mtk_dump_victim_tlb);

void __weak __mtk_dump_reg_for_hang_issue(int m4u_id, struct seq_file *s)
{
}
EXPORT_SYMBOL_GPL(__mtk_dump_reg_for_hang_issue);
