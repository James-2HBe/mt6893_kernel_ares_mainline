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

KSYMTAB_FUNC(fsl_asoc_get_dma_channel, "");
SYMBOL_FLAGS(fsl_asoc_get_dma_channel, 0x00);
KSYMTAB_FUNC(fsl_asoc_get_pll_clocks, "");
SYMBOL_FLAGS(fsl_asoc_get_pll_clocks, 0x00);
KSYMTAB_FUNC(fsl_asoc_reparent_pll_clocks, "");
SYMBOL_FLAGS(fsl_asoc_reparent_pll_clocks, 0x00);
KSYMTAB_FUNC(fsl_asoc_constrain_rates, "");
SYMBOL_FLAGS(fsl_asoc_constrain_rates, 0x00);
KSYMTAB_FUNC(fsl_asoc_get_xr_sx, "");
SYMBOL_FLAGS(fsl_asoc_get_xr_sx, 0x01);
KSYMTAB_FUNC(fsl_asoc_put_xr_sx, "");
SYMBOL_FLAGS(fsl_asoc_put_xr_sx, 0x01);
KSYMTAB_FUNC(fsl_asoc_get_enum_double, "");
SYMBOL_FLAGS(fsl_asoc_get_enum_double, 0x01);
KSYMTAB_FUNC(fsl_asoc_put_enum_double, "");
SYMBOL_FLAGS(fsl_asoc_put_enum_double, 0x01);
KSYMTAB_FUNC(fsl_asoc_get_volsw, "");
SYMBOL_FLAGS(fsl_asoc_get_volsw, 0x01);
KSYMTAB_FUNC(fsl_asoc_put_volsw, "");
SYMBOL_FLAGS(fsl_asoc_put_volsw, 0x01);

MODULE_INFO(depends, "snd-soc-core");

