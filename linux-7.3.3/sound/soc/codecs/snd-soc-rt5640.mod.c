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

KSYMTAB_FUNC(rt5640_dmic_enable, "");
SYMBOL_FLAGS(rt5640_dmic_enable, 0x01);
KSYMTAB_FUNC(rt5640_sel_asrc_clk_src, "");
SYMBOL_FLAGS(rt5640_sel_asrc_clk_src, 0x01);
KSYMTAB_FUNC(rt5640_enable_micbias1_for_ovcd, "");
SYMBOL_FLAGS(rt5640_enable_micbias1_for_ovcd, 0x01);
KSYMTAB_FUNC(rt5640_disable_micbias1_for_ovcd, "");
SYMBOL_FLAGS(rt5640_disable_micbias1_for_ovcd, 0x01);
KSYMTAB_FUNC(rt5640_detect_headset, "");
SYMBOL_FLAGS(rt5640_detect_headset, 0x01);
KSYMTAB_FUNC(rt5640_set_ovcd_params, "");
SYMBOL_FLAGS(rt5640_set_ovcd_params, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-soc-rl6231,snd-pcm");

MODULE_ALIAS("i2c:rt5640");
MODULE_ALIAS("i2c:rt5639");
MODULE_ALIAS("i2c:rt5642");
MODULE_ALIAS("of:N*T*Crealtek,rt5639");
MODULE_ALIAS("of:N*T*Crealtek,rt5639C*");
MODULE_ALIAS("of:N*T*Crealtek,rt5640");
MODULE_ALIAS("of:N*T*Crealtek,rt5640C*");
MODULE_ALIAS("acpi*:10EC3276:*");
MODULE_ALIAS("acpi*:10EC5640:*");
MODULE_ALIAS("acpi*:10EC5642:*");
MODULE_ALIAS("acpi*:INT33CA:*");
MODULE_ALIAS("acpi*:INTCCFFD:*");
