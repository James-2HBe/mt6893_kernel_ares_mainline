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


MODULE_INFO(depends, "snd-soc-core,snd-soc-mt8183-afe,snd-pcm,snd");

MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_max98357");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_max98357C*");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_rt1015");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_rt1015C*");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_rt1015p");
MODULE_ALIAS("of:N*T*Cmediatek,mt8183_da7219_rt1015pC*");
