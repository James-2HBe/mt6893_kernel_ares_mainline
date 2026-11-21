#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(arc4_setkey, "");
SYMBOL_FLAGS(arc4_setkey, 0x00);
KSYMTAB_FUNC(arc4_crypt, "");
SYMBOL_FLAGS(arc4_crypt, 0x00);

MODULE_INFO(depends, "");

