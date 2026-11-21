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

KSYMTAB_FUNC(lpass_macro_pds_init, "");
SYMBOL_FLAGS(lpass_macro_pds_init, 0x01);
KSYMTAB_FUNC(lpass_macro_pds_exit, "");
SYMBOL_FLAGS(lpass_macro_pds_exit, 0x01);
KSYMTAB_FUNC(lpass_macro_set_codec_version, "");
SYMBOL_FLAGS(lpass_macro_set_codec_version, 0x01);
KSYMTAB_FUNC(lpass_macro_get_codec_version, "");
SYMBOL_FLAGS(lpass_macro_get_codec_version, 0x01);

MODULE_INFO(depends, "");

