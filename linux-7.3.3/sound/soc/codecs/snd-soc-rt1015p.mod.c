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

MODULE_ALIAS("of:N*T*Crealtek,rt1015p");
MODULE_ALIAS("of:N*T*Crealtek,rt1015pC*");
MODULE_ALIAS("of:N*T*Crealtek,rt1019p");
MODULE_ALIAS("of:N*T*Crealtek,rt1019pC*");
MODULE_ALIAS("acpi*:RTL1015:*");
MODULE_ALIAS("acpi*:RTL1019:*");
