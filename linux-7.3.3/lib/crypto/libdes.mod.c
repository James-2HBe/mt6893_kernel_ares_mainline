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

KSYMTAB_FUNC(des_expand_key, "");
SYMBOL_FLAGS(des_expand_key, 0x01);
KSYMTAB_FUNC(des_encrypt, "");
SYMBOL_FLAGS(des_encrypt, 0x01);
KSYMTAB_FUNC(des_decrypt, "");
SYMBOL_FLAGS(des_decrypt, 0x01);
KSYMTAB_FUNC(des3_ede_expand_key, "");
SYMBOL_FLAGS(des3_ede_expand_key, 0x01);
KSYMTAB_FUNC(des3_ede_encrypt, "");
SYMBOL_FLAGS(des3_ede_encrypt, 0x01);
KSYMTAB_FUNC(des3_ede_decrypt, "");
SYMBOL_FLAGS(des3_ede_decrypt, 0x01);

MODULE_INFO(depends, "");

