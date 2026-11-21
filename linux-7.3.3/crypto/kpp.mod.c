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

KSYMTAB_FUNC(crypto_alloc_kpp, "");
SYMBOL_FLAGS(crypto_alloc_kpp, 0x01);
KSYMTAB_FUNC(crypto_grab_kpp, "");
SYMBOL_FLAGS(crypto_grab_kpp, 0x01);
KSYMTAB_FUNC(crypto_has_kpp, "");
SYMBOL_FLAGS(crypto_has_kpp, 0x01);
KSYMTAB_FUNC(crypto_register_kpp, "");
SYMBOL_FLAGS(crypto_register_kpp, 0x01);
KSYMTAB_FUNC(crypto_unregister_kpp, "");
SYMBOL_FLAGS(crypto_unregister_kpp, 0x01);
KSYMTAB_FUNC(kpp_register_instance, "");
SYMBOL_FLAGS(kpp_register_instance, 0x01);

MODULE_INFO(depends, "");

