/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 4.14 老路径兼容别名（非 ares-r-oss 保真文件，见 compat/PATH-ALIASES.md）。
 *
 * Linux 7.3 不再提供 <asm/unaligned.h>，统一使用 <linux/unaligned.h>。
 * 仅做路径转发，不改语义；get_unaligned/put_unaligned 的接口保持一致。
 */
#ifndef __COMPAT_ASM_UNALIGNED_H__
#define __COMPAT_ASM_UNALIGNED_H__

#include <linux/unaligned.h>

#endif /* __COMPAT_ASM_UNALIGNED_H__ */
