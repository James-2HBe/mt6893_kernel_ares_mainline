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

KSYMTAB_FUNC(nf_defrag_ipv6_enable, "");
SYMBOL_FLAGS(nf_defrag_ipv6_enable, 0x01);
KSYMTAB_FUNC(nf_defrag_ipv6_disable, "");
SYMBOL_FLAGS(nf_defrag_ipv6_disable, 0x01);
KSYMTAB_FUNC(nf_ct_frag6_gather, "");
SYMBOL_FLAGS(nf_ct_frag6_gather, 0x01);

MODULE_INFO(depends, "");

