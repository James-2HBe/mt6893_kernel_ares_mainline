/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mtk_platform_compat.h — 平台符号实现侧的 4.14 -> 7.3.3 兼容接线头
 *
 * 角色：被统一注入到 baseline/impl-adapted/ 全部 21 个 .c（首个 include），
 * 是集中的兼容接线点。本线（ares-7.3.3 vendoring 线）的目标是「不挂任何
 * 外部平台树」也能让 21 个驱动编译自包含，因此本头：
 *   1) 引入 compat/mtk_platform_bridge.h（576 行：平台私有类型/枚举/宏/
 *      寄存器占位 + 平台 HW 函数前向声明，全部 #ifndef 守卫）；
 *   2) 补 7.3 内核态缺失的 struct timespec（带 _STRUCT_TIMESPEC 守卫）；
 *   3) clkbuf 芯片枚举、pseudo_m4u 的 M4U 枚举/结构体改由 vendoring 到各
 *      .c 同目录的本地头提供，此处刻意不重复定义，避免宏替换/重定义。
 *
 * 仍需避免 mtk_wifi_compat.h 那类补符号（struct timeval / strlcpy 宏 /
 * mm_segment_t 等）与 7.3 主线头产生重定义与宏污染。
 *
 * 244 error 的完整清零过程与资产清单见 reports/IMPL-SELFCONTAINED.md；
 * 后续新增补充请同步登记，不要直接改 baseline/impl/ 下的保真件。
 */
#ifndef _MTK_PLATFORM_COMPAT_H
#define _MTK_PLATFORM_COMPAT_H

/* 路径 B 自包含平台桥接层：平台私有类型/枚举/宏/寄存器占位 +
 * 平台 HW 函数前向声明，集中注入到每个 impl-adapted .c。
 * 本线（ares-7.3.3）额外 vendoring 了 68 个平台头到
 * baseline/impl-adapted/include/，目标是「不挂任何外部平台树」也能编译，
 * 与 ares-7.3.3-ported 走包内 platform 树的路线互补。 */
#include "mtk_platform_bridge.h"

/* clkbuf 芯片枚举：本线把 3 个芯片专属头 mtk_clkbuf_hw.h（各 141 行、
 * 零传递依赖、自带 CLOCK_BUFFER_* / CLK_BUF_OUTPUT_IMPEDANCE_* /
 * CLK_BUF_CONTROLS_FOR_DESENSE_* / xo_id / WCN_* 等全部枚举）vendoring 到
 * 各 .c 同目录，由 .c 的 #include "mtk_clkbuf_hw.h" 直接命中（引号
 * include 优先同目录，无跨芯片错配）。因此这里不再补任何 clkbuf 枚举宏
 * 占位，避免与芯片头 enum 成员产生宏替换冲突。
 * 详见 reports/IMPL-SELFCONTAINED.md。 */

/* pseudo_m4u 的 M4U 枚举/结构体（M4U_CACHE_SYNC_ENUM / M4U_DMA_DIR /
 * struct M4U_CACHE_STRUCT / COMPAT_M4U_CACHE_STRUCT / struct M4U_DMA_STRUCT /
 * M4U_DMA_MAP_AREA 等）：改由 vendoring 到 pseudo_m4u/ 同目录的 framework
 * 版本地头 pseudo_m4u.h 提供（引号 include 优先同目录命中），故这里不再补，
 * 避免与本地头 enum 重复定义。详见 reports/IMPL-SELFCONTAINED.md。 */

/* ---- C. struct timespec -----------------------------------------------
 * 7.3 内核态把 uapi/linux/time.h 的 struct timespec 藏在
 * #ifndef __KERNEL__ 之后（内核代码应改用 timespec64），而 battery 私有头
 * 仍沿用旧类型。补一个带 _STRUCT_TIMESPEC 守卫的最小内核态定义；若某处
 * 已定义则自动让位，绝不重复定义。 */
#ifndef _STRUCT_TIMESPEC
#define _STRUCT_TIMESPEC
struct timespec {
	long tv_sec;
	long tv_nsec;
};
#endif

#endif /* _MTK_PLATFORM_COMPAT_H */
