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

KSYMTAB_FUNC(qrtr_endpoint_post, "");
SYMBOL_FLAGS(qrtr_endpoint_post, 0x01);
KSYMTAB_FUNC(qrtr_endpoint_hello, "");
SYMBOL_FLAGS(qrtr_endpoint_hello, 0x01);
KSYMTAB_FUNC(qrtr_endpoint_register, "");
SYMBOL_FLAGS(qrtr_endpoint_register, 0x01);
KSYMTAB_FUNC(qrtr_endpoint_unregister, "");
SYMBOL_FLAGS(qrtr_endpoint_unregister, 0x01);
KSYMTAB_FUNC(qrtr_ns_init, "");
SYMBOL_FLAGS(qrtr_ns_init, 0x01);
KSYMTAB_FUNC(qrtr_ns_remove, "");
SYMBOL_FLAGS(qrtr_ns_remove, 0x01);

MODULE_INFO(depends, "");

