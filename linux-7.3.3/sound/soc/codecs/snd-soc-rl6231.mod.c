#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(rl6231_get_pre_div, "");
SYMBOL_FLAGS(rl6231_get_pre_div, 0x01);
KSYMTAB_FUNC(rl6231_calc_dmic_clk, "");
SYMBOL_FLAGS(rl6231_calc_dmic_clk, 0x01);
KSYMTAB_FUNC(rl6231_pll_calc, "");
SYMBOL_FLAGS(rl6231_pll_calc, 0x01);
KSYMTAB_FUNC(rl6231_get_clk_info, "");
SYMBOL_FLAGS(rl6231_get_clk_info, 0x01);

MODULE_INFO(depends, "");

