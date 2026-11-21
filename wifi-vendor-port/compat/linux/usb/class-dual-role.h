/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/linux/usb/class-dual-role.h — 7.3 主线已移除的 dual-role class 兼容桩。
 * mtu3_dr.c 仍 #include <linux/usb/class-dual-role.h>。仅提供编译期自包含所需
 * 的最小声明；运行期由真实平台/USB 栈实现覆盖。
 */
#ifndef _COMPAT_LINUX_USB_CLASS_DUAL_ROLE_H
#define _COMPAT_LINUX_USB_CLASS_DUAL_ROLE_H

#include <linux/device.h>
#include <linux/notifier.h>

#define DUAL_ROLE_UNKNOWN	0
#define DUAL_ROLE_HOST		1
#define DUAL_ROLE_DEVICE		2

struct dual_role_phy_instance;

#endif /* _COMPAT_LINUX_USB_CLASS_DUAL_ROLE_H */
