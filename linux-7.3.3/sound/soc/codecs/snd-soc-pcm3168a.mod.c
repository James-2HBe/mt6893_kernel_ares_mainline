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

KSYMTAB_DATA(pcm3168a_regmap, "");
SYMBOL_FLAGS(pcm3168a_regmap, 0x01);
KSYMTAB_FUNC(pcm3168a_probe, "");
SYMBOL_FLAGS(pcm3168a_probe, 0x01);
KSYMTAB_FUNC(pcm3168a_remove, "");
SYMBOL_FLAGS(pcm3168a_remove, 0x01);
KSYMTAB_DATA(pcm3168a_pm_ops, "");
SYMBOL_FLAGS(pcm3168a_pm_ops, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm");

