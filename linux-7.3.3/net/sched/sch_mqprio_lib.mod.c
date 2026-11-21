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

KSYMTAB_FUNC(mqprio_validate_qopt, "");
SYMBOL_FLAGS(mqprio_validate_qopt, 0x01);
KSYMTAB_FUNC(mqprio_qopt_reconstruct, "");
SYMBOL_FLAGS(mqprio_qopt_reconstruct, 0x01);
KSYMTAB_FUNC(mqprio_fp_to_offload, "");
SYMBOL_FLAGS(mqprio_fp_to_offload, 0x01);

MODULE_INFO(depends, "");

