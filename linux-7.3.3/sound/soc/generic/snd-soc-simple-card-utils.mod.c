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

KSYMTAB_FUNC(simple_util_get_sample_fmt, "");
SYMBOL_FLAGS(simple_util_get_sample_fmt, 0x01);
KSYMTAB_FUNC(simple_util_parse_convert, "");
SYMBOL_FLAGS(simple_util_parse_convert, 0x01);
KSYMTAB_FUNC(simple_util_is_convert_required, "");
SYMBOL_FLAGS(simple_util_is_convert_required, 0x01);
KSYMTAB_FUNC(simple_util_parse_daifmt, "");
SYMBOL_FLAGS(simple_util_parse_daifmt, 0x01);
KSYMTAB_FUNC(simple_util_parse_tdm_width_map, "");
SYMBOL_FLAGS(simple_util_parse_tdm_width_map, 0x01);
KSYMTAB_FUNC(simple_util_set_dailink_name, "");
SYMBOL_FLAGS(simple_util_set_dailink_name, 0x01);
KSYMTAB_FUNC(simple_util_parse_property, "");
SYMBOL_FLAGS(simple_util_parse_property, 0x01);
KSYMTAB_FUNC(simple_util_parse_card_name, "");
SYMBOL_FLAGS(simple_util_parse_card_name, 0x01);
KSYMTAB_FUNC(simple_util_parse_clk, "");
SYMBOL_FLAGS(simple_util_parse_clk, 0x01);
KSYMTAB_FUNC(simple_util_startup, "");
SYMBOL_FLAGS(simple_util_startup, 0x01);
KSYMTAB_FUNC(simple_util_shutdown, "");
SYMBOL_FLAGS(simple_util_shutdown, 0x01);
KSYMTAB_FUNC(simple_util_hw_params, "");
SYMBOL_FLAGS(simple_util_hw_params, 0x01);
KSYMTAB_FUNC(simple_util_be_hw_params_fixup, "");
SYMBOL_FLAGS(simple_util_be_hw_params_fixup, 0x01);
KSYMTAB_FUNC(simple_util_dai_init, "");
SYMBOL_FLAGS(simple_util_dai_init, 0x01);
KSYMTAB_FUNC(simple_util_canonicalize_platform, "");
SYMBOL_FLAGS(simple_util_canonicalize_platform, 0x01);
KSYMTAB_FUNC(simple_util_canonicalize_cpu, "");
SYMBOL_FLAGS(simple_util_canonicalize_cpu, 0x01);
KSYMTAB_FUNC(simple_util_clean_reference, "");
SYMBOL_FLAGS(simple_util_clean_reference, 0x01);
KSYMTAB_FUNC(simple_util_init_jack, "");
SYMBOL_FLAGS(simple_util_init_jack, 0x01);
KSYMTAB_FUNC(simple_util_init_aux_jacks, "");
SYMBOL_FLAGS(simple_util_init_aux_jacks, 0x01);
KSYMTAB_FUNC(simple_util_init_priv, "");
SYMBOL_FLAGS(simple_util_init_priv, 0x01);
KSYMTAB_FUNC(simple_util_remove, "");
SYMBOL_FLAGS(simple_util_remove, 0x01);
KSYMTAB_FUNC(graph_util_card_probe, "");
SYMBOL_FLAGS(graph_util_card_probe, 0x01);
KSYMTAB_FUNC(graph_util_is_ports0, "");
SYMBOL_FLAGS(graph_util_is_ports0, 0x01);
KSYMTAB_FUNC(graph_util_parse_dai, "");
SYMBOL_FLAGS(graph_util_parse_dai, 0x01);
KSYMTAB_FUNC(graph_util_parse_link_direction, "");
SYMBOL_FLAGS(graph_util_parse_link_direction, 0x01);
KSYMTAB_FUNC(graph_util_parse_trigger_order, "");
SYMBOL_FLAGS(graph_util_parse_trigger_order, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm");

