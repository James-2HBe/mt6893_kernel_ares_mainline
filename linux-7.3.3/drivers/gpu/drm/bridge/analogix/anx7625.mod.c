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


MODULE_INFO(depends, "drm,drm_display_helper,drm_dp_aux_bus,typec,drm_kms_helper");

MODULE_ALIAS("i2c:anx7625");
MODULE_ALIAS("of:N*T*Canalogix,anx7625");
MODULE_ALIAS("of:N*T*Canalogix,anx7625C*");

MODULE_INFO(srcversion, "EE6C8F108BF7DDC70D51369");
