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

KSYMTAB_FUNC(free_rs, "");
SYMBOL_FLAGS(free_rs, 0x01);
KSYMTAB_FUNC(init_rs_gfp, "");
SYMBOL_FLAGS(init_rs_gfp, 0x01);
KSYMTAB_FUNC(init_rs_non_canonical, "");
SYMBOL_FLAGS(init_rs_non_canonical, 0x01);
KSYMTAB_FUNC(encode_rs8, "");
SYMBOL_FLAGS(encode_rs8, 0x01);
KSYMTAB_FUNC(decode_rs8, "");
SYMBOL_FLAGS(decode_rs8, 0x01);

MODULE_INFO(depends, "");

