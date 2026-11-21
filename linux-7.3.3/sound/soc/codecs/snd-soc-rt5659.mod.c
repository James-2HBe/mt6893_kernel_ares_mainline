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

KSYMTAB_FUNC(rt5659_set_jack_detect, "");
SYMBOL_FLAGS(rt5659_set_jack_detect, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-soc-rl6231,snd-pcm");

MODULE_ALIAS("i2c:rt5658");
MODULE_ALIAS("i2c:rt5659");
MODULE_ALIAS("of:N*T*Crealtek,rt5658");
MODULE_ALIAS("of:N*T*Crealtek,rt5658C*");
MODULE_ALIAS("of:N*T*Crealtek,rt5659");
MODULE_ALIAS("of:N*T*Crealtek,rt5659C*");
MODULE_ALIAS("acpi*:10EC5658:*");
MODULE_ALIAS("acpi*:10EC5659:*");
