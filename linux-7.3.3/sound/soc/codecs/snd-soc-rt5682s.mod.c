#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(rt5682s_sel_asrc_clk_src, "");
SYMBOL_FLAGS(rt5682s_sel_asrc_clk_src, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Crealtek,rt5682s");
MODULE_ALIAS("of:N*T*Crealtek,rt5682sC*");
MODULE_ALIAS("acpi*:RTL5682:*");
MODULE_ALIAS("i2c:rt5682s");
