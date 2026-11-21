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

KSYMTAB_DATA(rt5682_supply_names, "");
SYMBOL_FLAGS(rt5682_supply_names, 0x01);
KSYMTAB_FUNC(rt5682_apply_patch_list, "");
SYMBOL_FLAGS(rt5682_apply_patch_list, 0x01);
KSYMTAB_DATA(rt5682_reg, "");
SYMBOL_FLAGS(rt5682_reg, 0x01);
KSYMTAB_FUNC(rt5682_volatile_register, "");
SYMBOL_FLAGS(rt5682_volatile_register, 0x01);
KSYMTAB_FUNC(rt5682_readable_register, "");
SYMBOL_FLAGS(rt5682_readable_register, 0x01);
KSYMTAB_FUNC(rt5682_reset, "");
SYMBOL_FLAGS(rt5682_reset, 0x01);
KSYMTAB_FUNC(rt5682_sel_asrc_clk_src, "");
SYMBOL_FLAGS(rt5682_sel_asrc_clk_src, 0x01);
KSYMTAB_FUNC(rt5682_jack_detect_handler, "");
SYMBOL_FLAGS(rt5682_jack_detect_handler, 0x01);
KSYMTAB_FUNC(rt5682_register_dai_clks, "");
SYMBOL_FLAGS(rt5682_register_dai_clks, 0x01);
KSYMTAB_DATA(rt5682_aif1_dai_ops, "");
SYMBOL_FLAGS(rt5682_aif1_dai_ops, 0x01);
KSYMTAB_DATA(rt5682_aif2_dai_ops, "");
SYMBOL_FLAGS(rt5682_aif2_dai_ops, 0x01);
KSYMTAB_DATA(rt5682_soc_component_dev, "");
SYMBOL_FLAGS(rt5682_soc_component_dev, 0x01);
KSYMTAB_FUNC(rt5682_parse_dt, "");
SYMBOL_FLAGS(rt5682_parse_dt, 0x01);
KSYMTAB_FUNC(rt5682_get_ldo1, "");
SYMBOL_FLAGS(rt5682_get_ldo1, 0x01);
KSYMTAB_FUNC(rt5682_calibrate, "");
SYMBOL_FLAGS(rt5682_calibrate, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-soc-rl6231,snd-pcm");

