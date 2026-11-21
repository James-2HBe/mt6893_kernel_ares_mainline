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

KSYMTAB_FUNC(mt8183_dai_i2s_set_share, "");
SYMBOL_FLAGS(mt8183_dai_i2s_set_share, 0x01);

MODULE_INFO(depends, "snd-soc-mtk-common,snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Cmediatek,mt8183-audio");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183-audioC*");
