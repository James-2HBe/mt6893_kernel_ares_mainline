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


MODULE_INFO(depends, "snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Callwinner,sun4i-a10-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun4i-a10-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun6i-a31-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun6i-a31-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-a83t-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-a83t-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun8i-h3-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-a64-codec-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-a64-codec-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h6-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-h6-i2sC*");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-r329-i2s");
MODULE_ALIAS("of:N*T*Callwinner,sun50i-r329-i2sC*");
