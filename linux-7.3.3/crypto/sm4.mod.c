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

KSYMTAB_DATA(crypto_sm4_fk, "");
SYMBOL_FLAGS(crypto_sm4_fk, 0x00);
KSYMTAB_DATA(crypto_sm4_ck, "");
SYMBOL_FLAGS(crypto_sm4_ck, 0x00);
KSYMTAB_DATA(crypto_sm4_sbox, "");
SYMBOL_FLAGS(crypto_sm4_sbox, 0x00);
KSYMTAB_FUNC(sm4_expandkey, "");
SYMBOL_FLAGS(sm4_expandkey, 0x01);
KSYMTAB_FUNC(sm4_crypt_block, "");
SYMBOL_FLAGS(sm4_crypt_block, 0x01);

MODULE_INFO(depends, "");

