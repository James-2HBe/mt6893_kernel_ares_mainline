#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(raid6_gen_syndrome, "");
SYMBOL_FLAGS(raid6_gen_syndrome, 0x01);
KSYMTAB_FUNC(raid6_xor_syndrome, "");
SYMBOL_FLAGS(raid6_xor_syndrome, 0x01);
KSYMTAB_FUNC(raid6_can_xor_syndrome, "");
SYMBOL_FLAGS(raid6_can_xor_syndrome, 0x01);
KSYMTAB_FUNC(raid6_recov_2data, "");
SYMBOL_FLAGS(raid6_recov_2data, 0x01);
KSYMTAB_FUNC(raid6_recov_datap, "");
SYMBOL_FLAGS(raid6_recov_datap, 0x01);
KSYMTAB_DATA(raid6_gfmul, "");
SYMBOL_FLAGS(raid6_gfmul, 0x00);
KSYMTAB_DATA(raid6_vgfmul, "");
SYMBOL_FLAGS(raid6_vgfmul, 0x00);
KSYMTAB_DATA(raid6_gfexp, "");
SYMBOL_FLAGS(raid6_gfexp, 0x00);
KSYMTAB_DATA(raid6_gflog, "");
SYMBOL_FLAGS(raid6_gflog, 0x00);
KSYMTAB_DATA(raid6_gfinv, "");
SYMBOL_FLAGS(raid6_gfinv, 0x00);
KSYMTAB_DATA(raid6_gfexi, "");
SYMBOL_FLAGS(raid6_gfexi, 0x00);

MODULE_INFO(depends, "");

