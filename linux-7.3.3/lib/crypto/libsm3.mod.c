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

KSYMTAB_FUNC(sm3_init, "");
SYMBOL_FLAGS(sm3_init, 0x01);
KSYMTAB_FUNC(sm3_update, "");
SYMBOL_FLAGS(sm3_update, 0x01);
KSYMTAB_FUNC(sm3_final, "");
SYMBOL_FLAGS(sm3_final, 0x01);
KSYMTAB_FUNC(sm3, "");
SYMBOL_FLAGS(sm3, 0x01);

MODULE_INFO(depends, "");

