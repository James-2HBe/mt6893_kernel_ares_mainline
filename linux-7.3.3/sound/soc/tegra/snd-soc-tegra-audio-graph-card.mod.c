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


MODULE_INFO(depends, "snd-soc-audio-graph-card,snd-soc-simple-card-utils,snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Cnvidia,tegra210-audio-graph-card");
MODULE_ALIAS("of:N*T*Cnvidia,tegra210-audio-graph-cardC*");
MODULE_ALIAS("of:N*T*Cnvidia,tegra186-audio-graph-card");
MODULE_ALIAS("of:N*T*Cnvidia,tegra186-audio-graph-cardC*");
MODULE_ALIAS("of:N*T*Cnvidia,tegra238-audio-graph-card");
MODULE_ALIAS("of:N*T*Cnvidia,tegra238-audio-graph-cardC*");
MODULE_ALIAS("of:N*T*Cnvidia,tegra264-audio-graph-card");
MODULE_ALIAS("of:N*T*Cnvidia,tegra264-audio-graph-cardC*");
