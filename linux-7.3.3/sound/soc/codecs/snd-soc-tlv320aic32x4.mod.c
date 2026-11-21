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

KSYMTAB_DATA(aic32x4_regmap_pages, "");
SYMBOL_FLAGS(aic32x4_regmap_pages, 0x01);
KSYMTAB_FUNC(aic32x4_probe, "");
SYMBOL_FLAGS(aic32x4_probe, 0x00);
KSYMTAB_FUNC(aic32x4_remove, "");
SYMBOL_FLAGS(aic32x4_remove, 0x00);
KSYMTAB_FUNC(aic32x4_register_clocks, "");
SYMBOL_FLAGS(aic32x4_register_clocks, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm,snd");

