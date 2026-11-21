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

KSYMTAB_FUNC(md5_init, "");
SYMBOL_FLAGS(md5_init, 0x01);
KSYMTAB_FUNC(md5_update, "");
SYMBOL_FLAGS(md5_update, 0x01);
KSYMTAB_FUNC(md5_final, "");
SYMBOL_FLAGS(md5_final, 0x01);
KSYMTAB_FUNC(md5, "");
SYMBOL_FLAGS(md5, 0x01);
KSYMTAB_FUNC(hmac_md5_preparekey, "");
SYMBOL_FLAGS(hmac_md5_preparekey, 0x01);
KSYMTAB_FUNC(hmac_md5_init, "");
SYMBOL_FLAGS(hmac_md5_init, 0x01);
KSYMTAB_FUNC(hmac_md5_init_usingrawkey, "");
SYMBOL_FLAGS(hmac_md5_init_usingrawkey, 0x01);
KSYMTAB_FUNC(hmac_md5_final, "");
SYMBOL_FLAGS(hmac_md5_final, 0x01);
KSYMTAB_FUNC(hmac_md5, "");
SYMBOL_FLAGS(hmac_md5, 0x01);
KSYMTAB_FUNC(hmac_md5_usingrawkey, "");
SYMBOL_FLAGS(hmac_md5_usingrawkey, 0x01);

MODULE_INFO(depends, "");

