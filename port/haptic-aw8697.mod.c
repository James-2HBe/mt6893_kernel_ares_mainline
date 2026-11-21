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


MODULE_INFO(depends, "ff-memless");

MODULE_ALIAS("of:N*T*Cawinic,aw8697_haptic");
MODULE_ALIAS("of:N*T*Cawinic,aw8697_hapticC*");
MODULE_ALIAS("of:N*T*Cawinic,aw8697");
MODULE_ALIAS("of:N*T*Cawinic,aw8697C*");
MODULE_ALIAS("i2c:aw8697_haptic");
