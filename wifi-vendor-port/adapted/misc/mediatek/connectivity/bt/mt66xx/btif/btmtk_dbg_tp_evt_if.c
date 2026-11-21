/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

#define CREATE_TRACE_POINTS
/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
#include "mtk_wifi_compat.h"
#include "btmtk_dbg_tp_evt.h"
#include "btmtk_dbg_tp_evt_if.h"

void bt_dbg_tp_evt(unsigned int pkt_action,
		unsigned int parameter,
		unsigned int data_len,
		char *data)
{
	//struct timespec64 kerneltime;
	//ktime_get_ts64(&kerneltime);
	trace_bt_evt(pkt_action, parameter, data_len, data);
}

