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


MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cawinic,aw2028");
MODULE_ALIAS("of:N*T*Cawinic,aw2028C*");
MODULE_ALIAS("of:N*T*Cawinic,aw2028_i2c");
MODULE_ALIAS("of:N*T*Cawinic,aw2028_i2cC*");
MODULE_ALIAS("of:N*T*Cawinic,_aw2028_i2c");
MODULE_ALIAS("of:N*T*Cawinic,_aw2028_i2cC*");
