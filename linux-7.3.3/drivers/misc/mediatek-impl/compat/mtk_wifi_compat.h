/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mtk_wifi_compat.h — MT6893 vendor WiFi/connectivity stack (4.14 / GKI 5.x)
 *                     -> Linux 7.3.x mainline API compatibility shim
 *
 * 用法（在目标树里，三选一）：
 *   1) ccflags-y += -include $(srctree)/drivers/misc/mediatek/connectivity/compat/mtk_wifi_compat.h
 *   2) clang ... -include /path/to/mtk_wifi_compat.h foo.c
 *   3) tools/adapt_api.py 会在每个 .c 顶部插入 #include "mtk_wifi_compat.h"
 *
 * 分工（重要）：
 *   - 本头【只补内核已彻底删除、且不便在源码侧逐个改的纯符号】：
 *       set_fs 家族、struct timeval、do_gettimeofday、strlcpy、
 *       dma_zalloc_coherent、VERIFY_*、create_proc_entry。
 *   - 【改签名/改调用点】的适配（class_create 去 owner、access_ok 去 VERIFY、
 *       init_timer->timer_setup、file_operations->proc_ops、vfs_read->kernel_read）
 *     全部由 tools/adapt_api.py 落到 adapted/ 源码里，改动可 diff 审计。
 *   - 平台私有符号（mt-plat / eccci / clkbuf / pmic / emi / conn_md 的导出函数）
 *     不在本头范围，由目标树「已修好的 MTK 平台基线层」提供。
 *
 * 门控：仅当 LINUX_VERSION_CODE >= 6.1 生效；在 4.14/5.x 原厂树上本头为空，
 *       源码自带的 108 文件 / 253 处 KERNEL_VERSION 兼容宏照常工作，互不干扰。
 *
 * 验证：compat/compat_selftest.c 用 7.3.3 真实头文件 -fsyntax-only 编译通过。
 */
#ifndef _MTK_WIFI_COMPAT_H
#define _MTK_WIFI_COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)

#include <linux/types.h>
#include <linux/time64.h>       /* struct timespec64 */
#include <linux/timekeeping.h>  /* ktime_get_real_ts64 */
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/dma-mapping.h>
#include <linux/proc_fs.h>

/* ============================================================
 * A. uaccess：set_fs / get_fs / get_ds / mm_segment_t / KERNEL_DS / USER_DS
 * ------------------------------------------------------------
 * 原厂（ComboA/mt6873/autok_dvfs.c 等）：
 *     mm_segment_t oldfs = get_fs(); set_fs(get_ds());
 *     filp_open(...); vfs_read(file, kernbuf, size, &off); set_fs(oldfs);
 * 7.3：5.18 起整套删除（arm64 无 addr_limit）。真实探针确认 mm_segment_t 未声明。
 * 处理：类型 + 宏做成无副作用空壳；vfs_read/vfs_write 的内核缓冲调用由
 *       adapt_api.py 改为 kernel_read/kernel_write（本头不覆盖 vfs_read，避免污染）。
 * ============================================================ */
typedef unsigned int mm_segment_t;
#define KERNEL_DS ((mm_segment_t)0)
#define USER_DS   ((mm_segment_t)1)
#define get_ds()  (KERNEL_DS)
#define get_fs()  (USER_DS)
#define set_fs(x) do { (void)(x); } while (0)

/* ============================================================
 * B. time：struct timeval / do_gettimeofday()
 * ------------------------------------------------------------
 * 原厂（btif/mtk_btif.c、conn_md_dump.c 等 61 处）：
 *     struct timeval tv; do_gettimeofday(&tv); ... tv.tv_sec/tv.tv_usec
 * 7.3：内核态 struct timeval 只剩【前向声明】(探针实测 incomplete)，
 *      do_gettimeofday 删除，改 timespec64 + ktime_get_real_ts64。
 * 处理：这里给 struct timeval 补一份完整定义（合法：此前仅前向声明），
 *      并提供 do_gettimeofday inline，tv_usec = tv_nsec/1000 语义等价。
 *      成员用 long，兼容原厂 (u64)tv_sec*1000000L+tv_usec 的算术。
 * ============================================================ */
struct timeval {
	long tv_sec;
	long tv_usec;
};
static inline void mtk_compat_do_gettimeofday(struct timeval *tv)
{
	struct timespec64 ts;
	ktime_get_real_ts64(&ts);
	tv->tv_sec  = (long)ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}
#define do_gettimeofday(tv) mtk_compat_do_gettimeofday(tv)

/* ============================================================
 * C. uaccess：VERIFY_READ / VERIFY_WRITE
 * ------------------------------------------------------------
 * 7.3 删除，access_ok 变两参数。adapt_api.py 会把
 *   access_ok(VERIFY_WRITE, p, n) -> access_ok(p, n)
 * 这里定义这两个值，兜住个别未覆盖处（如宏拼接）。
 * ============================================================ */
#ifndef VERIFY_READ
#define VERIFY_READ  0
#endif
#ifndef VERIFY_WRITE
#define VERIFY_WRITE 1
#endif

/* ============================================================
 * D. string：strlcpy -> strscpy（7.3 删除 strlcpy）
 *    语义：strlcpy 返回源长，strscpy 返回拷贝数或 -E2BIG；私有栈基本忽略
 *    返回值，直接替换安全；检查返回值处由 adapt_api.py 标注。
 * ============================================================ */
#ifndef strlcpy
#define strlcpy(dst, src, n) strscpy(dst, src, n)
#endif

/* ============================================================
 * E. DMA：dma_zalloc_coherent -> dma_alloc_coherent
 *    7.3 删除前者；后者本就保证零初始化。
 * ============================================================ */
#ifndef dma_zalloc_coherent
#define dma_zalloc_coherent(dev, size, dma_handle, gfp) \
	dma_alloc_coherent(dev, size, dma_handle, gfp)
#endif

/* ============================================================
 * F. procfs：create_proc_entry（3.x 就删了，私有栈 3 处）
 *    仅保证「调用」不报隐式声明并返回合法类型；返回 NULL 使运行期走
 *    「创建失败」分支。原厂随后对 entry->read_proc/write_proc 的成员赋值
 *    在 7.3 不存在，由 adapt_api.py 一并处理（改 proc_create + proc_ops）。
 * ============================================================ */
static inline struct proc_dir_entry *
mtk_compat_create_proc_entry(const char *name, umode_t mode,
			     struct proc_dir_entry *parent)
{
	(void)name; (void)mode; (void)parent;
	return NULL;
}
#define create_proc_entry(name, mode, parent) \
	mtk_compat_create_proc_entry(name, mode, parent)

/* ============================================================
 * 不在此头处理（交给 tools/adapt_api.py 改源码，语义需逐个确认）：
 *   class_create(THIS_MODULE, x)  -> class_create(x)
 *   access_ok(VERIFY_*, p, n)     -> access_ok(p, n)
 *   init_timer + .function/.data  -> timer_setup + from_timer
 *   struct file_operations(proc)  -> struct proc_ops（字段名/回调签名）
 *   vfs_read/vfs_write(kernbuf)   -> kernel_read/kernel_write
 *   create_proc_entry 后的成员赋值 -> proc_create + proc_ops
 * 平台私有符号：由目标树 MTK 平台基线层提供（见 docs/API-ADAPTATION.md）。
 * ============================================================ */

#else  /* < 6.1：原厂/5.x 树，保持源码原样 */
#endif

#endif /* _MTK_WIFI_COMPAT_H */
