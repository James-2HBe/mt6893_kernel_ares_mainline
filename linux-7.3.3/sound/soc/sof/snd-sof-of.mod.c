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

KSYMTAB_DATA(sof_of_pm, "");
SYMBOL_FLAGS(sof_of_pm, 0x00);
KSYMTAB_FUNC(sof_of_probe, "");
SYMBOL_FLAGS(sof_of_probe, 0x00);
KSYMTAB_FUNC(sof_of_remove, "");
SYMBOL_FLAGS(sof_of_remove, 0x00);
KSYMTAB_FUNC(sof_of_shutdown, "");
SYMBOL_FLAGS(sof_of_shutdown, 0x00);

MODULE_INFO(depends, "snd-sof");

