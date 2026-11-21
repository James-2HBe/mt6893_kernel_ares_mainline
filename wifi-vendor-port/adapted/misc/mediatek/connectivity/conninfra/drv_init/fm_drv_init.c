// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019 MediaTek Inc.
 */

/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
#include "mtk_wifi_compat.h"
#include <linux/kernel.h>

#include "fm_drv_init.h"

#ifdef CONFIG_MTK_FMRADIO
int __attribute__((weak)) mtk_wcn_fm_init(void)
{
	pr_err("no impl. mtk_wcn_fm_init\n");
	return 0;
}
#endif

int do_fm_drv_init(int chip_id)
{
	pr_info("Start to do fm module init\n");

#ifdef CONFIG_MTK_FMRADIO
	mtk_wcn_fm_init();
#endif

	pr_info("Finish fm module init\n");
	return 0;
}
