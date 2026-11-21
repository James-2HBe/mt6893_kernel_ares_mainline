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


MODULE_INFO(depends, "snd-soc-core");

MODULE_ALIAS("of:N*T*Callwinner,sun4i-a10-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun4i-a10-spdifC*");
MODULE_ALIAS("of:N*T*Callwinner,sun6i-a31-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun6i-a31-spdifC*");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-spdifC*");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h6-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h6-spdifC*");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h616-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h616-spdifC*");
MODULE_ALIAS("of:N*T*Callwinner,sun55i-a523-spdif");
MODULE_ALIAS("of:N*T*Callwinner,sun55i-a523-spdifC*");
