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

KSYMTAB_FUNC(hisi_zip_get_pf_driver, "");
SYMBOL_FLAGS(hisi_zip_get_pf_driver, 0x01);

MODULE_INFO(depends, "hisi_qm");

MODULE_ALIAS("pci:v000019E5d0000A250sv*sd*bc*sc*i*");
MODULE_ALIAS("pci:v000019E5d0000A251sv*sd*bc*sc*i*");
