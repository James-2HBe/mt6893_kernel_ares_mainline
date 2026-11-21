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


MODULE_INFO(depends, "snd-soc-wcd-common,snd-soc-wcd-mbhc,soundwire-bus,snd-soc-core,snd-soc-wcd-classh,snd-soc-wcd937x-sdw");

MODULE_ALIAS("of:N*T*Cqcom,wcd9370-codec");
MODULE_ALIAS("of:N*T*Cqcom,wcd9370-codecC*");
MODULE_ALIAS("of:N*T*Cqcom,wcd9375-codec");
MODULE_ALIAS("of:N*T*Cqcom,wcd9375-codecC*");
