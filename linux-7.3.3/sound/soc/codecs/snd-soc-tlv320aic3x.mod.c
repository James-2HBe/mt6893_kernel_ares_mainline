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

KSYMTAB_DATA(aic3x_regmap, "");
SYMBOL_FLAGS(aic3x_regmap, 0x01);
KSYMTAB_FUNC(aic3x_probe, "");
SYMBOL_FLAGS(aic3x_probe, 0x00);
KSYMTAB_FUNC(aic3x_remove, "");
SYMBOL_FLAGS(aic3x_remove, 0x00);

MODULE_INFO(depends, "snd-soc-core,snd-pcm");

