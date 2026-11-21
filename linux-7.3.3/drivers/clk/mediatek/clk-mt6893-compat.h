/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Compatibility shim for porting the ares (MT6893) vendor 4.19 CCF clock
 * drivers to mainline 7.3.3.  The vendor clk-mt6885-*.c files are the clock
 * tables actually used by MT6893 in the ares BSP (MT6893 reuses the mt6885
 * naming).  This header bridges the small CCF API differences between the two
 * kernel versions without touching the vendor clock data tables.
 */

#ifndef __CLK_MT6893_COMPAT_H
#define __CLK_MT6893_COMPAT_H

/* 7.3.3 moved PLL_AO / struct mtk_pll_data into clk-pll.h */
#include "clk-pll.h"

/* Vendor flag: PLL has a separate enable bit controlled via en_mask.
 * Mainline 7.3.3 uses pll_en_bit; bit 3 is unused by mainline flags. */
#ifndef EN_BIT_CTRL
#define EN_BIT_CTRL		(BIT(3))
#endif

/* Vendor gate ops "dummy" variants were removed in mainline: they were
 * gates present in DT but not actually controlling hardware.  Map them to
 * the regular setclr ops -- functionally equivalent for boot. */
#define mtk_clk_gate_ops_setclr_dummy		mtk_clk_gate_ops_setclr
#define mtk_clk_gate_ops_setclr_inv_dummy	mtk_clk_gate_ops_setclr_inv

#endif /* __CLK_MT6893_COMPAT_H */
