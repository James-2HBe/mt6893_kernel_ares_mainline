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

KSYMTAB_FUNC(ecc_get_curve25519, "");
SYMBOL_FLAGS(ecc_get_curve25519, 0x00);
KSYMTAB_FUNC(ecc_get_curve, "");
SYMBOL_FLAGS(ecc_get_curve, 0x00);
KSYMTAB_FUNC(ecc_digits_from_bytes, "");
SYMBOL_FLAGS(ecc_digits_from_bytes, 0x00);
KSYMTAB_FUNC(ecc_alloc_point, "");
SYMBOL_FLAGS(ecc_alloc_point, 0x00);
KSYMTAB_FUNC(ecc_free_point, "");
SYMBOL_FLAGS(ecc_free_point, 0x00);
KSYMTAB_FUNC(vli_is_zero, "");
SYMBOL_FLAGS(vli_is_zero, 0x00);
KSYMTAB_FUNC(vli_num_bits, "");
SYMBOL_FLAGS(vli_num_bits, 0x00);
KSYMTAB_FUNC(vli_from_be64, "");
SYMBOL_FLAGS(vli_from_be64, 0x00);
KSYMTAB_FUNC(vli_from_le64, "");
SYMBOL_FLAGS(vli_from_le64, 0x00);
KSYMTAB_FUNC(vli_cmp, "");
SYMBOL_FLAGS(vli_cmp, 0x00);
KSYMTAB_FUNC(vli_sub, "");
SYMBOL_FLAGS(vli_sub, 0x00);
KSYMTAB_FUNC(vli_mod_mult_slow, "");
SYMBOL_FLAGS(vli_mod_mult_slow, 0x00);
KSYMTAB_FUNC(vli_mod_inv, "");
SYMBOL_FLAGS(vli_mod_inv, 0x00);
KSYMTAB_FUNC(ecc_point_is_zero, "");
SYMBOL_FLAGS(ecc_point_is_zero, 0x00);
KSYMTAB_FUNC(ecc_point_mult_shamir, "");
SYMBOL_FLAGS(ecc_point_mult_shamir, 0x00);
KSYMTAB_FUNC(ecc_is_key_valid, "");
SYMBOL_FLAGS(ecc_is_key_valid, 0x00);
KSYMTAB_FUNC(ecc_gen_privkey, "");
SYMBOL_FLAGS(ecc_gen_privkey, 0x00);
KSYMTAB_FUNC(ecc_make_pub_key, "");
SYMBOL_FLAGS(ecc_make_pub_key, 0x00);
KSYMTAB_FUNC(ecc_is_pubkey_valid_partial, "");
SYMBOL_FLAGS(ecc_is_pubkey_valid_partial, 0x00);
KSYMTAB_FUNC(ecc_is_pubkey_valid_full, "");
SYMBOL_FLAGS(ecc_is_pubkey_valid_full, 0x00);
KSYMTAB_FUNC(crypto_ecdh_shared_secret, "");
SYMBOL_FLAGS(crypto_ecdh_shared_secret, 0x00);

MODULE_INFO(depends, "");

