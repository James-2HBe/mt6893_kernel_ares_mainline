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

KSYMTAB_FUNC(crypto_transfer_aead_request_to_engine, "");
SYMBOL_FLAGS(crypto_transfer_aead_request_to_engine, 0x01);
KSYMTAB_FUNC(crypto_transfer_akcipher_request_to_engine, "");
SYMBOL_FLAGS(crypto_transfer_akcipher_request_to_engine, 0x01);
KSYMTAB_FUNC(crypto_transfer_hash_request_to_engine, "");
SYMBOL_FLAGS(crypto_transfer_hash_request_to_engine, 0x01);
KSYMTAB_FUNC(crypto_transfer_kpp_request_to_engine, "");
SYMBOL_FLAGS(crypto_transfer_kpp_request_to_engine, 0x01);
KSYMTAB_FUNC(crypto_transfer_skcipher_request_to_engine, "");
SYMBOL_FLAGS(crypto_transfer_skcipher_request_to_engine, 0x01);
KSYMTAB_FUNC(crypto_finalize_aead_request, "");
SYMBOL_FLAGS(crypto_finalize_aead_request, 0x01);
KSYMTAB_FUNC(crypto_finalize_akcipher_request, "");
SYMBOL_FLAGS(crypto_finalize_akcipher_request, 0x01);
KSYMTAB_FUNC(crypto_finalize_hash_request, "");
SYMBOL_FLAGS(crypto_finalize_hash_request, 0x01);
KSYMTAB_FUNC(crypto_finalize_kpp_request, "");
SYMBOL_FLAGS(crypto_finalize_kpp_request, 0x01);
KSYMTAB_FUNC(crypto_finalize_skcipher_request, "");
SYMBOL_FLAGS(crypto_finalize_skcipher_request, 0x01);
KSYMTAB_FUNC(crypto_engine_start, "");
SYMBOL_FLAGS(crypto_engine_start, 0x01);
KSYMTAB_FUNC(crypto_engine_stop, "");
SYMBOL_FLAGS(crypto_engine_stop, 0x01);
KSYMTAB_FUNC(crypto_engine_alloc_init_and_set, "");
SYMBOL_FLAGS(crypto_engine_alloc_init_and_set, 0x01);
KSYMTAB_FUNC(crypto_engine_alloc_init, "");
SYMBOL_FLAGS(crypto_engine_alloc_init, 0x01);
KSYMTAB_FUNC(crypto_engine_exit, "");
SYMBOL_FLAGS(crypto_engine_exit, 0x01);
KSYMTAB_FUNC(crypto_engine_register_aead, "");
SYMBOL_FLAGS(crypto_engine_register_aead, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_aead, "");
SYMBOL_FLAGS(crypto_engine_unregister_aead, 0x01);
KSYMTAB_FUNC(crypto_engine_register_aeads, "");
SYMBOL_FLAGS(crypto_engine_register_aeads, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_aeads, "");
SYMBOL_FLAGS(crypto_engine_unregister_aeads, 0x01);
KSYMTAB_FUNC(crypto_engine_register_ahash, "");
SYMBOL_FLAGS(crypto_engine_register_ahash, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_ahash, "");
SYMBOL_FLAGS(crypto_engine_unregister_ahash, 0x01);
KSYMTAB_FUNC(crypto_engine_register_ahashes, "");
SYMBOL_FLAGS(crypto_engine_register_ahashes, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_ahashes, "");
SYMBOL_FLAGS(crypto_engine_unregister_ahashes, 0x01);
KSYMTAB_FUNC(crypto_engine_register_akcipher, "");
SYMBOL_FLAGS(crypto_engine_register_akcipher, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_akcipher, "");
SYMBOL_FLAGS(crypto_engine_unregister_akcipher, 0x01);
KSYMTAB_FUNC(crypto_engine_register_kpp, "");
SYMBOL_FLAGS(crypto_engine_register_kpp, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_kpp, "");
SYMBOL_FLAGS(crypto_engine_unregister_kpp, 0x01);
KSYMTAB_FUNC(crypto_engine_register_skcipher, "");
SYMBOL_FLAGS(crypto_engine_register_skcipher, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_skcipher, "");
SYMBOL_FLAGS(crypto_engine_unregister_skcipher, 0x01);
KSYMTAB_FUNC(crypto_engine_register_skciphers, "");
SYMBOL_FLAGS(crypto_engine_register_skciphers, 0x01);
KSYMTAB_FUNC(crypto_engine_unregister_skciphers, "");
SYMBOL_FLAGS(crypto_engine_unregister_skciphers, 0x01);

MODULE_INFO(depends, "kpp");

