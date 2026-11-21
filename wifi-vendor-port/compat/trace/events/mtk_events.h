/* SPDX-License-Identifier: GPL-2.0 */
/*
 * compat/trace/events/mtk_events.h — MTK 私有 tracepoint 头兼容桩。
 *
 * 原 4.14 BSP 的 trace/events/mtk_events.h 由 TRACE_EVENT 宏在构建期生成，
 * 平台源码树中并无静态文件。7.3 目标树同样不带这些私有 tracepoint。
 * 未启用 CONFIG_TRACEPOINTS 时，内核 tracepoint 本身就是 no-op 宏，故此处
 * 以等价的 no-op 可变参数宏补齐，使 impl-adapted 达到编译期自包含。
 * 需要真实追踪时由目标树的 trace 定义覆盖本桩。
 */
#ifndef _COMPAT_TRACE_EVENTS_MTK_EVENTS_H
#define _COMPAT_TRACE_EVENTS_MTK_EVENTS_H

#define trace_ppm_update(...)		do { } while (0)
#define trace_ppm_user_setting(...)	do { } while (0)

#endif /* _COMPAT_TRACE_EVENTS_MTK_EVENTS_H */
