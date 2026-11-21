/* SPDX-License-Identifier: GPL-2.0 */
/*
 * 4.14 老路径兼容别名（非 ares-r-oss 保真文件，见 compat/PATH-ALIASES.md）。
 *
 * seriaTvT/kernel_mt6893 的 conninfra/wlan 私有栈写的是
 *     #include <soc/mediatek/emi.h>
 * 而 ares-r-oss 与 Linux 7.3 把同一个 EMI/MPU 头放在
 *     #include <memory/mediatek/emi.h>
 * 本文件只做路径转发，不改动、不重复声明任何符号。
 * 若目标树的 soc/mediatek/emi.h 含额外私有声明，请用真文件替换本别名。
 */
#ifndef __COMPAT_SOC_MEDIATEK_EMI_H__
#define __COMPAT_SOC_MEDIATEK_EMI_H__

#include <memory/mediatek/emi.h>

#endif /* __COMPAT_SOC_MEDIATEK_EMI_H__ */
