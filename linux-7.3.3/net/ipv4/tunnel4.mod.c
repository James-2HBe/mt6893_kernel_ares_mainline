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

KSYMTAB_FUNC(xfrm4_tunnel_register, "");
SYMBOL_FLAGS(xfrm4_tunnel_register, 0x00);
KSYMTAB_FUNC(xfrm4_tunnel_deregister, "");
SYMBOL_FLAGS(xfrm4_tunnel_deregister, 0x00);

MODULE_INFO(depends, "");

