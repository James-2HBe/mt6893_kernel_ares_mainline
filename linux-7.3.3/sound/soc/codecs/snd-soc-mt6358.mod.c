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

KSYMTAB_FUNC(mt6358_set_mtkaif_protocol, "");
SYMBOL_FLAGS(mt6358_set_mtkaif_protocol, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd");

MODULE_ALIAS("of:N*T*Cmediatek,mt6358-sound");
MODULE_ALIAS("of:N*T*Cmediatek,mt6358-soundC*");
MODULE_ALIAS("of:N*T*Cmediatek,mt6366-sound");
MODULE_ALIAS("of:N*T*Cmediatek,mt6366-soundC*");
