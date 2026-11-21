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

KSYMTAB_FUNC(mt8192_afe_gpio_init, "");
SYMBOL_FLAGS(mt8192_afe_gpio_init, 0x00);
KSYMTAB_FUNC(mt8192_afe_gpio_request, "");
SYMBOL_FLAGS(mt8192_afe_gpio_request, 0x00);
KSYMTAB_FUNC(mt8192_dai_i2s_set_share, "");
SYMBOL_FLAGS(mt8192_dai_i2s_set_share, 0x01);

MODULE_INFO(depends, "snd-soc-mtk-common,snd-soc-core,snd-pcm,snd");

MODULE_ALIAS("of:N*T*Cmediatek,mt8192-audio");
MODULE_ALIAS("of:N*T*Cmediatek,mt8192-audioC*");
