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

KSYMTAB_DATA(sof_xtensa_arch_ops, "SND_SOC_SOF_XTENSA");
SYMBOL_FLAGS(sof_xtensa_arch_ops, 0x00);

MODULE_INFO(depends, "");

