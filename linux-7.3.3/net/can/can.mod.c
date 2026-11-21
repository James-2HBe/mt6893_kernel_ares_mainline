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

KSYMTAB_FUNC(can_sock_destruct, "");
SYMBOL_FLAGS(can_sock_destruct, 0x00);
KSYMTAB_FUNC(can_send, "");
SYMBOL_FLAGS(can_send, 0x00);
KSYMTAB_FUNC(can_rx_register, "");
SYMBOL_FLAGS(can_rx_register, 0x00);
KSYMTAB_FUNC(can_rx_unregister, "");
SYMBOL_FLAGS(can_rx_unregister, 0x00);
KSYMTAB_FUNC(can_set_skb_uid, "");
SYMBOL_FLAGS(can_set_skb_uid, 0x00);
KSYMTAB_FUNC(can_proto_register, "");
SYMBOL_FLAGS(can_proto_register, 0x00);
KSYMTAB_FUNC(can_proto_unregister, "");
SYMBOL_FLAGS(can_proto_unregister, 0x00);

MODULE_INFO(depends, "");

