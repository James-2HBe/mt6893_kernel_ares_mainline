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

KSYMTAB_FUNC(audio_graph2_link_normal, "");
SYMBOL_FLAGS(audio_graph2_link_normal, 0x01);
KSYMTAB_FUNC(audio_graph2_link_dpcm, "");
SYMBOL_FLAGS(audio_graph2_link_dpcm, 0x01);
KSYMTAB_FUNC(audio_graph2_link_c2c, "");
SYMBOL_FLAGS(audio_graph2_link_c2c, 0x01);
KSYMTAB_FUNC(audio_graph2_parse_of, "");
SYMBOL_FLAGS(audio_graph2_parse_of, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-soc-simple-card-utils");

MODULE_ALIAS("of:N*T*Caudio-graph-card2");
MODULE_ALIAS("of:N*T*Caudio-graph-card2C*");
