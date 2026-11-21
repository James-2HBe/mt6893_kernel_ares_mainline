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

MODULE_ALIAS("of:N*T*Cnuvoton,nau8315");
MODULE_ALIAS("of:N*T*Cnuvoton,nau8315C*");
MODULE_ALIAS("of:N*T*Cnuvoton,nau8318");
MODULE_ALIAS("of:N*T*Cnuvoton,nau8318C*");
MODULE_ALIAS("acpi*:NVTN2010:*");
MODULE_ALIAS("acpi*:NVTN2012:*");
