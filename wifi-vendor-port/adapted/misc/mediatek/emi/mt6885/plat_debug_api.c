/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
#include "mtk_wifi_compat.h"
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <mt-plat/sync_write.h>
#include <mt-plat/mtk_io.h>
#ifdef CONFIG_MTK_AEE_FEATURE
#include <mt-plat/aee.h>
#include <mt-plat/mtk_ram_console.h>
#endif

#include "mt_emi.h"

void plat_debug_api_init(void)
{
}

void dump_emi_outstanding(void)
{
}
