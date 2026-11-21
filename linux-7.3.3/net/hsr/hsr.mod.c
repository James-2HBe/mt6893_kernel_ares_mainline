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

KSYMTAB_FUNC(hsr_get_version, "");
SYMBOL_FLAGS(hsr_get_version, 0x00);
KSYMTAB_FUNC(is_hsr_master, "");
SYMBOL_FLAGS(is_hsr_master, 0x00);
KSYMTAB_FUNC(hsr_get_port_ndev, "");
SYMBOL_FLAGS(hsr_get_port_ndev, 0x00);
KSYMTAB_FUNC(hsr_get_port_type, "");
SYMBOL_FLAGS(hsr_get_port_type, 0x00);

MODULE_INFO(depends, "");

