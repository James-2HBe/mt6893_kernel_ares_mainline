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


MODULE_INFO(depends, "sun8i-adda-pr-regmap,snd-soc-core");

MODULE_ALIAS("of:N*T*Callwinner,sun8i-a23-codec-analog");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-a23-codec-analogC*");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-codec-analog");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-codec-analogC*");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-v3s-codec-analog");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-v3s-codec-analogC*");
