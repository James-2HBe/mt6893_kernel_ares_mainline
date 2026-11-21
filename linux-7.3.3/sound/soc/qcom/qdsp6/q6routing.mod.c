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

KSYMTAB_FUNC(q6routing_stream_open, "");
SYMBOL_FLAGS(q6routing_stream_open, 0x01);
KSYMTAB_FUNC(q6routing_stream_close, "");
SYMBOL_FLAGS(q6routing_stream_close, 0x01);

MODULE_INFO(depends, "q6adm,snd-soc-core");

MODULE_ALIAS("of:N*T*Cqcom,q6adm-routing");
MODULE_ALIAS("of:N*T*Cqcom,q6adm-routingC*");
