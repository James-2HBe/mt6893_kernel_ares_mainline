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

KSYMTAB_FUNC(crypto_ecdh_key_len, "");
SYMBOL_FLAGS(crypto_ecdh_key_len, 0x01);
KSYMTAB_FUNC(crypto_ecdh_encode_key, "");
SYMBOL_FLAGS(crypto_ecdh_encode_key, 0x01);
KSYMTAB_FUNC(crypto_ecdh_decode_key, "");
SYMBOL_FLAGS(crypto_ecdh_decode_key, 0x01);

MODULE_INFO(depends, "kpp,ecc");

