/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mtk_framework_compat.h — B 类（clk/iommu/devfreq）框架符号的 7.3.3 契约头
 *
 * 用途：把 4.14 私有栈通过 33 个平台导出符号调用的 B 类框架符号，
 *       接到 Linux 7.3.3 主线已有的 clk / iommu(SMI) / devfreq(DVFSRC) 框架上。
 *
 * 源码依据（均为 GitHub 开源仓库，逐文件 git blob sha 校验）：
 *   - torvalds/linux @ master            -> framework/upstream/   （主线等价框架）
 *   - xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2 (aeb1327b)
 *                                        -> framework/vendor/     （4.14 语义基准）
 *
 * 设计原则：**不把 4.14 私有 clk/iommu/devfreq 驱动整体并入 7.3 主线目录**。
 * 7.3.3 已有 drivers/clk/mediatek/、drivers/memory/mtk-smi.c、
 * drivers/iommu/mtk_iommu.c、drivers/soc/mediatek/mtk-dvfsrc.c、
 * drivers/pmdomain/mediatek/mtk-pm-domains.c 提供同名子系统；整体搬入会
 * 重复定义框架结构体与全局符号。因此本层只提供「链接期符号 + 语义桥接」。
 *
 * 本头自包含、只依赖主线头，可独立编译验证（solution/verify-class-b.sh）。
 */
#ifndef __MTK_FRAMEWORK_COMPAT_H
#define __MTK_FRAMEWORK_COMPAT_H

#include <linux/list.h>
#include <linux/types.h>

/* ------------------------------------------------------------------ */
/* devfreq：VCORE OPP 未请求哨兵                                        */
/* 4.14 定义：drivers/devfreq/helio-dvfsrc-opp.h                        */
/*   #define VCORE_OPP_UNREQ  (-1)                                      */
/* get_cur_vcore_opp() 在 QoS 未使能 / OPP 越界时返回该值。              */
/* ------------------------------------------------------------------ */
#ifndef VCORE_OPP_UNREQ
#define VCORE_OPP_UNREQ  (-1)
#endif

/* ------------------------------------------------------------------ */
/* clk / power-gating：struct pg_callbacks                              */
/*                                                                     */
/* 4.14 私有头 drivers/clk/mediatek/clk-mt6873-pg.h 定义：               */
/*   enum subsys_id { ..., NR_SYSS = 26 };                              */
/*   struct pg_callbacks {                                              */
/*       struct list_head list;                                         */
/*       void (*before_off)(enum subsys_id sys);                         */
/*       void (*after_on)(enum subsys_id sys);                           */
/*       void (*debug_dump)(enum subsys_id sys);                         */
/*   };                                                                 */
/*                                                                     */
/* 7.3.3 主线无此头。standalone 编译（编译探针 / 独立验证）用最小 ABI      */
/* 兼容定义：enum 底层为 int，函数指针调用约定一致，链接期 C 符号不带类型   */
/* 信息，因此二进制兼容。目标树若确实把原厂 clk-mt6873-pg.h 放进了        */
/* include path，定义 MTK_FRAMEWORK_HAVE_PG_H 即改用原厂真实类型。         */
/* ------------------------------------------------------------------ */
#ifdef MTK_FRAMEWORK_HAVE_PG_H
#include <clk-mt6873-pg.h>
#define MTK_PG_SYS_T  enum subsys_id
#else
enum mtk_subsys_id {
	MTK_SUBSYS_UNSPEC = 0,
	MTK_SUBSYS_NR = 26,          /* 对齐原厂 NR_SYSS = 26 */
};
struct pg_callbacks {
	struct list_head list;
	void (*before_off)(int sys);
	void (*after_on)(int sys);
	void (*debug_dump)(int sys);
};
#define MTK_PG_SYS_T  int
#endif

/* ---- 由 solution/framework-symbols.c 提供的符号 ---- */

/* 4.14 原厂符号：注册 power-gating 回调，返回同一指针 */
struct pg_callbacks *register_pg_callback(struct pg_callbacks *pgcb);

/* 新增桥接入口：目标树在 generic_pm_domain 的 power_off/power_on 钩子
 * 或 clk notifier 回调里调用，把主线事件转发给已注册的私有回调链。
 * off=true 触发 before_off，off=false 触发 after_on。 */
void mtk_pg_notify(int sys, bool off);

/* 新增桥接入口：异常/调试路径触发 debug_dump */
void mtk_pg_debug_dump(int sys);

/* 4.14 原厂符号：SMI/M4U 寄存器备份与恢复 */
int smi_reg_backup_sec(void);
int smi_reg_restore_sec(void);

/* 新增桥接入口：目标树可把主线 mtk-smi.c 的
 * mtk_smi_larb_suspend/resume（或 pm_runtime_force_suspend/resume 包装）
 * 注入进来；未注入时由主线 runtime PM 覆盖，返回 0。 */
void mtk_framework_set_smi_backend(int (*fn)(bool backup));

/* 4.14 原厂符号：查询当前 VCORE OPP（未请求返回 VCORE_OPP_UNREQ） */
int get_cur_vcore_opp(void);

/* 新增桥接入口：目标树 mtk-dvfsrc 驱动在 OPP 切换时同步当前值。
 * 主线等价：drivers/soc/mediatek/mtk-dvfsrc.c 的
 * dvfsrc_get_current_opp()->vcore_opp / dvfsrc_get_current_level_v*()。 */
void mtk_framework_set_vcore_opp(int opp);

#endif /* __MTK_FRAMEWORK_COMPAT_H */
