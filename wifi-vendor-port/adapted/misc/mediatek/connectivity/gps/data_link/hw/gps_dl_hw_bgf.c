/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2019 - 2021 MediaTek Inc.
 */

/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
#include "mtk_wifi_compat.h"
#include "gps_dl_config.h"
#include "gps_dl_hw_priv_util.h"
/* #include "bgf/bgf_cfg.h" */
/* #include "bgf/bgf_cfg_on.h" */

#define BGF_ADDR_ENTRY_NUM (1)
static const struct gps_dl_addr_map_entry g_bfg_addr_table[BGF_ADDR_ENTRY_NUM] = {
	{0, 0, 0}
	/* {0x18812000, BGF_CFG_BASE, 0x1000}, */
	/* {0x18813000, BGF_CFG_ON_BASE, 0x1000}, */
};

unsigned int bgf_bus_to_host(unsigned int bgf_addr)
{
	unsigned int i;
	const struct gps_dl_addr_map_entry *p = NULL;

	for (i = 0; i < BGF_ADDR_ENTRY_NUM; i++) {
		p = &g_bfg_addr_table[i];
		if (bgf_addr >= p->bus_addr &&
			bgf_addr < (p->bus_addr + p->length))
			return ((bgf_addr - p->bus_addr) + p->host_addr);
	}

	return 0;
}

