/* SPDX-License-Identifier: GPL-2.0 */
/*
 * framework-symbols.c — B 类（clk / iommu / devfreq）框架符号的 7.3.3 实现
 *
 * 目标内核：Linux 7.3.3 (arm64)。自包含，仅依赖主线头 + compat/mtk_framework_compat.h，
 * 可独立 -c 编译产 .o 并用 nm 核验；不 include 任何 MTK 私有头。
 *
 * =====================================================================
 * 为什么需要这一层
 * =====================================================================
 * stage-2/3 的 reports/PLATFORM-IMPL.md 把 33 个链接期平台符号按耦合度分 A/B：
 *   A = 弱耦合的 MTK 私有实现，可整体移植（stage-3 已导出 21 个 .c）
 *   B = 与主线 clk/iommu 框架强耦合，"需单独适配或提供兼容桩，不建议整体
 *       并入主线目录"，此前明确留给目标树「已修基线层」，未纳入交付。
 *
 * 本轮（stage-5）把 B 类闭环。B 类符号共 5 个：
 *   register_pg_callback   (clk)      <- 本文件
 *   smi_reg_backup_sec     (iommu)    <- 本文件
 *   smi_reg_restore_sec    (iommu)    <- 本文件
 *   get_cur_vcore_opp      (devfreq)  <- 本文件（用户明确把 devfreq 归入 B 类）
 *   aee_rr_rec_clk         (clk)      <- stage-4 已由 solution/platform-symbols.c 解决
 *
 * =====================================================================
 * 源码来源（GitHub 开源仓库，逐文件 git blob sha 校验，见 tools/framework-manifest.tsv）
 * =====================================================================
 *  [U] torvalds/linux @ master —— 主线 Linux 的 MT6893 框架等价实现（framework/upstream/）：
 *      drivers/clk/mediatek/clk-mtk.c       mtk_clk_simple_probe / clk_notifier_func 框架
 *      drivers/clk/mediatek/clk-mux.c       devm_mtk_clk_mux_notifier_register -> devm_clk_notifier_register
 *      drivers/memory/mtk-smi.c             mediatek,mt6893-smi-larb / -smi-common
 *                                           + mtk_smi_larb_suspend/resume (runtime PM)
 *      drivers/iommu/mtk_iommu.c            mt6893_data / mediatek,mt6893-iommu-mm
 *      drivers/soc/mediatek/mtk-dvfsrc.c    dvfsrc_get_current_opp()->vcore_opp
 *      drivers/regulator/mtk-dvfsrc-regulator.c
 *      drivers/pmdomain/mediatek/mtk-pm-domains.c + mt6893-pm-domains.h
 *  [V] xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2 (aeb1327b) —— 4.14 语义基准
 *                                           （framework/vendor/）：
 *      drivers/clk/mediatek/clk-mt6873-pg.c:827   register_pg_callback 真实实现
 *      drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c:247,262  smi_reg_*_sec 真实实现
 *      drivers/devfreq/helio-dvfsrc-opp.c:76      get_cur_vcore_opp 真实实现
 *      + MT6893 variant 与 Makefile/Kconfig 依赖链
 *
 *  补充调研（如实记录，见 docs/GITHUB-SOURCES-CLASS-B.md）：Collabora 于
 *  2025-04-10 投出的「clk: mediatek: Add main clocks drivers for Dimensity
 *  1200 MT6893」v1 补丁（clk-mt6893-{topckgen,apmixedsys,infra_ao}.c 等
 *  15 文件 / +2455 行）**尚未合入任何可公开访问的 GitHub 分支**：
 *  torvalds/linux@master 与 v6.16 的 drivers/clk/mediatek/ 均无 6893 文件，
 *  若干常见个人 fork 亦 404。因此不作为保真源，仅作主线化方向参考。
 *
 * =====================================================================
 * 关键技术判断：不整体并入 4.14 私有驱动
 * =====================================================================
 * 7.3.3 主线已原生提供 MT6893 的：
 *   - SMI：  mtk_smi_larb_mt6893 / mtk_smi_common_mt6893 + SET_RUNTIME_PM_OPS
 *   - IOMMU：mt6893_data / "mediatek,mt6893-iommu-mm"
 *   - DVFSRC：mtk-dvfsrc.c + mtk-dvfsrc-regulator.c（vcore_opp）
 *   - PM domain：mt6893-pm-domains.h + mtk-pm-domains.c（power_on/off）
 *   - clk 框架：clk-mtk.c 的 mtk_clk_simple_probe / devm_clk_notifier_register
 * 把 4.14 的 clk-mt6873-pg.c / pseudo_m4u.c / helio-dvfsrc-opp.c 整体搬进
 * 主线同名目录，会与上述框架重复定义结构体与全局符号，且引入 TEE / SSPM IPI
 * 等 7.3 已不存在的私有运行时，编译期即失败——这正是 B 类被单独列出的原因。
 *
 * 因此本文件做「函数级移植 + 主线语义桥接」：保留 4.14 的对外契约与关键
 * 语义（回调链、边界检查、哨兵值），把存储/触发后端换成 7.3 主线可接线入口。
 *
 * =====================================================================
 * 冲突避免（与 stage-4 同策略）
 * =====================================================================
 * 全部符号声明 __weak：
 *   - 目标树走「路径 A」（7.3.3 主线框架）：只编本文件，不编 framework/vendor/
 *     里的 3 个 4.14 私有驱动；本实现消除 undefined symbol。
 *   - 目标树走「路径 B」（确实移植了整套 4.14 私有框架）：强符号自动覆盖 weak，
 *     不会重复定义；此时可把主线事件转发到本文件的桥接入口。
 * 两条路径均能链接，验证脚本对两种配置都做了检查。
 *
 * aee_rr_rec_clk 不在此文件重复定义，由 solution/platform-symbols.c 提供。
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/export.h>
#include <linux/compiler.h>

#include "compat/mtk_framework_compat.h"

/* ================================================================== */
/* 1. clk / power-gating —— register_pg_callback                        */
/*                                                                     */
/* 4.14 原厂实现（clk-mt6873-pg.c:827）：                                */
/*   spinlock pgcb_lock + LIST_HEAD(pgcb_list)；                          */
/*   register_pg_callback: INIT_LIST_HEAD -> list_add -> return pgcb；    */
/*   掉电调试路径 list_for_each_entry_reverse 调 debug_dump。              */
/* 7.3.3 主线等价：clk-mtk.c 的 devm_clk_notifier_register /              */
/*   clk_notifier_func（rate-change 通知）与 mtk-pm-domains.c 的          */
/*   generic_pm_domain.power_off/power_on。                                */
/* 本实现：保留原厂回调链语义，新增 mtk_pg_notify / mtk_pg_debug_dump       */
/*   两个入口，供目标树在主线 PM / notifier 钩子上转发事件。                */
/* ================================================================== */

static DEFINE_SPINLOCK(pgcb_lock);
static LIST_HEAD(pgcb_list);

/**
 * register_pg_callback - 注册 power-gating 回调（4.14 契约）
 * @pgcb: 调用方持有的回调结构，其 list 成员由本函数初始化
 *
 * 逐行对齐原厂语义；额外补一个 NULL 检查（原厂未做，7.3 上更安全）。
 * 返回传入指针，与原厂一致。
 */
struct pg_callbacks *__weak register_pg_callback(struct pg_callbacks *pgcb)
{
	unsigned long flags;

	if (!pgcb)
		return NULL;

	spin_lock_irqsave(&pgcb_lock, flags);
	INIT_LIST_HEAD(&pgcb->list);
	list_add(&pgcb->list, &pgcb_list);
	spin_unlock_irqrestore(&pgcb_lock, flags);

	return pgcb;
}
/* register_pg_callback: EXPORT 由驱动真实现提供，此处 weak 定义不再导出 */

/**
 * mtk_pg_notify - 主线 PM 事件 -> 私有回调链（新增桥接入口）
 * @sys: 子系统 id（对齐 4.14 enum subsys_id）
 * @off: true = 掉电前（before_off），false = 上电后（after_on）
 *
 * 目标树接线示例（drivers/pmdomain/mediatek/mtk-pm-domains.c）：
 *   scpsys_power_off()  开头 -> mtk_pg_notify(pd->id, true);
 *   scpsys_power_on()   末尾 -> mtk_pg_notify(pd->id, false);
 * 遍历顺序与原厂 debug_dump 一致（reverse，后注册先调用）。
 */
void __weak mtk_pg_notify(int sys, bool off)
{
	struct pg_callbacks *pgcb;
	unsigned long flags;

	spin_lock_irqsave(&pgcb_lock, flags);
	list_for_each_entry_reverse(pgcb, &pgcb_list, list) {
		if (off && pgcb->before_off)
			pgcb->before_off((MTK_PG_SYS_T)sys);
		if (!off && pgcb->after_on)
			pgcb->after_on((MTK_PG_SYS_T)sys);
	}
	spin_unlock_irqrestore(&pgcb_lock, flags);
}
EXPORT_SYMBOL_GPL(mtk_pg_notify);

/** mtk_pg_debug_dump - 异常/调试路径触发 debug_dump（新增桥接入口） */
void __weak mtk_pg_debug_dump(int sys)
{
	struct pg_callbacks *pgcb;
	unsigned long flags;

	spin_lock_irqsave(&pgcb_lock, flags);
	list_for_each_entry_reverse(pgcb, &pgcb_list, list) {
		if (pgcb->debug_dump)
			pgcb->debug_dump((MTK_PG_SYS_T)sys);
	}
	spin_unlock_irqrestore(&pgcb_lock, flags);
}
EXPORT_SYMBOL_GPL(mtk_pg_debug_dump);

/* ================================================================== */
/* 2. iommu / SMI —— smi_reg_backup_sec / smi_reg_restore_sec           */
/*                                                                     */
/* 4.14 原厂实现（pseudo_m4u.c:247,262）：                               */
/*   KREE_TeeServiceCall(m4u_session, M4U_TZCMD_REG_BACKUP/RESTORE, ...)  */
/*   经 TEE 安全世界保存/恢复 M4U 寄存器，失败返回 -1，成功返回 0。        */
/* 7.3.3 主线等价（drivers/memory/mtk-smi.c:694-729, 928-960）：          */
/*   mtk_smi_larb_suspend/resume + mtk_smi_common_suspend/resume，由       */
/*   SET_RUNTIME_PM_OPS(...) 与 SET_LATE_SYSTEM_SLEEP_PM_OPS(             */
/*     pm_runtime_force_suspend, pm_runtime_force_resume) 在系统休眠时     */
/*   自动完成等价的寄存器保存/恢复。                                       */
/* 结论：7.3 无 TEE 安全调用通道，且重复备份会与主线 regmap 冲突；         */
/*   这里返回 0（成功、语义已由主线 PM 覆盖）而非空桩静默失败。并提供       */
/*   mtk_framework_set_smi_backend() 供目标树显式注入后端（可选）。        */
/* ================================================================== */

static int (*smi_sec_backend)(bool backup);

/**
 * mtk_framework_set_smi_backend - 注入主线 SMI PM 后端（可选）
 * @fn: backup=true 走 suspend、false 走 resume 的包装函数
 */
void __weak mtk_framework_set_smi_backend(int (*fn)(bool backup))
{
	smi_sec_backend = fn;
}
EXPORT_SYMBOL_GPL(mtk_framework_set_smi_backend);

int __weak smi_reg_backup_sec(void)
{
	if (smi_sec_backend)
		return smi_sec_backend(true);

	pr_debug("%s: handled by mainline mtk-smi runtime PM "
		 "(mtk_smi_larb_suspend / pm_runtime_force_suspend)\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(smi_reg_backup_sec);

int __weak smi_reg_restore_sec(void)
{
	if (smi_sec_backend)
		return smi_sec_backend(false);

	pr_debug("%s: handled by mainline mtk-smi runtime PM "
		 "(mtk_smi_larb_resume / pm_runtime_force_resume)\n", __func__);
	return 0;
}
EXPORT_SYMBOL_GPL(smi_reg_restore_sec);

/* ================================================================== */
/* 3. devfreq / DVFSRC —— get_cur_vcore_opp                             */
/*                                                                     */
/* 4.14 原厂实现（helio-dvfsrc-opp.c:76）：                              */
/*   if (!is_qos_enabled()) return VCORE_OPP_UNREQ;                       */
/*   idx = get_cur_vcore_dvfs_opp();                                      */
/*   if (idx >= VCORE_DVFS_OPP_NUM) return VCORE_OPP_UNREQ;               */
/*   return vcore_dvfs_to_vcore_opp[idx];                                 */
/* 依赖 SSPM IPI + QoS 私有运行时，7.3 无对应通道。                        */
/* 7.3.3 主线等价（drivers/soc/mediatek/mtk-dvfsrc.c:188-337）：           */
/*   dvfsrc_get_current_opp(dvfsrc)->vcore_opp、                          */
/*   dvfsrc_get_current_level_v1/v2/v4()、dvfsrc_get_vcore_level_v1()      */
/* 本实现：保留哨兵语义（未同步时返回 VCORE_OPP_UNREQ，让私有栈走           */
/*   "未请求" 分支而非误判成 OPP 0），由目标树 mtk-dvfsrc 在 OPP 切换时     */
/*   调用 mtk_framework_set_vcore_opp() 同步当前值。                       */
/* ================================================================== */

static int cur_vcore_opp = VCORE_OPP_UNREQ;
static DEFINE_SPINLOCK(vcore_opp_lock);

int __weak get_cur_vcore_opp(void)
{
	int opp;
	unsigned long flags;

	spin_lock_irqsave(&vcore_opp_lock, flags);
	opp = cur_vcore_opp;
	spin_unlock_irqrestore(&vcore_opp_lock, flags);
	return opp;
}
/* get_cur_vcore_opp: EXPORT 由驱动真实现提供，此处 weak 定义不再导出 */

/**
 * mtk_framework_set_vcore_opp - 主线 DVFSRC 同步当前 VCORE OPP
 * @opp: 当前 OPP 等级；传 VCORE_OPP_UNREQ 表示未请求/未使能
 *
 * 目标树接线示例（drivers/soc/mediatek/mtk-dvfsrc.c 的 OPP 切换路径）：
 *   mtk_framework_set_vcore_opp(dvfsrc_get_current_opp(dvfsrc)->vcore_opp);
 */
void __weak mtk_framework_set_vcore_opp(int opp)
{
	unsigned long flags;

	spin_lock_irqsave(&vcore_opp_lock, flags);
	cur_vcore_opp = opp;
	spin_unlock_irqrestore(&vcore_opp_lock, flags);
}
EXPORT_SYMBOL_GPL(mtk_framework_set_vcore_opp);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MT6893 class-B (clk/iommu/devfreq) framework symbol bridge for 7.3.3");
MODULE_AUTHOR("ares porting");
