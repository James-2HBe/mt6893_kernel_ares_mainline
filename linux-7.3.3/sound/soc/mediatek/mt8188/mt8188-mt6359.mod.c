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


MODULE_INFO(depends, "snd-soc-core,snd,snd-soc-mt6359,snd-pcm,snd-soc-mtk-common");

MODULE_ALIAS("of:N*T*Cmediatek,mt8188-mt6359-evb");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-mt6359-evbC*");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-nau8825");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-nau8825C*");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-rt5682s");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-rt5682sC*");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-es8326");
MODULE_ALIAS("of:N*T*Cmediatek,mt8188-es8326C*");
