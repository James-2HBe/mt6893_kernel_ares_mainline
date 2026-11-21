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


MODULE_INFO(depends, "snd-soc-core,snd");

MODULE_ALIAS("of:N*T*Cdioo,dio2125");
MODULE_ALIAS("of:N*T*Cdioo,dio2125C*");
MODULE_ALIAS("of:N*T*Csimple-audio-amplifier");
MODULE_ALIAS("of:N*T*Csimple-audio-amplifierC*");
MODULE_ALIAS("of:N*T*Cgpio-audio-amp-mono");
MODULE_ALIAS("of:N*T*Cgpio-audio-amp-monoC*");
MODULE_ALIAS("of:N*T*Cgpio-audio-amp-stereo");
MODULE_ALIAS("of:N*T*Cgpio-audio-amp-stereoC*");
