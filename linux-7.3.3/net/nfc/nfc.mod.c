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

KSYMTAB_FUNC(nfc_fw_download_done, "");
SYMBOL_FLAGS(nfc_fw_download_done, 0x00);
KSYMTAB_FUNC(nfc_dep_link_is_up, "");
SYMBOL_FLAGS(nfc_dep_link_is_up, 0x00);
KSYMTAB_FUNC(nfc_find_se, "");
SYMBOL_FLAGS(nfc_find_se, 0x00);
KSYMTAB_FUNC(nfc_set_remote_general_bytes, "");
SYMBOL_FLAGS(nfc_set_remote_general_bytes, 0x00);
KSYMTAB_FUNC(nfc_get_local_general_bytes, "");
SYMBOL_FLAGS(nfc_get_local_general_bytes, 0x00);
KSYMTAB_FUNC(nfc_tm_data_received, "");
SYMBOL_FLAGS(nfc_tm_data_received, 0x00);
KSYMTAB_FUNC(nfc_tm_activated, "");
SYMBOL_FLAGS(nfc_tm_activated, 0x00);
KSYMTAB_FUNC(nfc_tm_deactivated, "");
SYMBOL_FLAGS(nfc_tm_deactivated, 0x00);
KSYMTAB_FUNC(nfc_alloc_recv_skb, "");
SYMBOL_FLAGS(nfc_alloc_recv_skb, 0x00);
KSYMTAB_FUNC(nfc_targets_found, "");
SYMBOL_FLAGS(nfc_targets_found, 0x00);
KSYMTAB_FUNC(nfc_target_lost, "");
SYMBOL_FLAGS(nfc_target_lost, 0x00);
KSYMTAB_FUNC(nfc_driver_failure, "");
SYMBOL_FLAGS(nfc_driver_failure, 0x00);
KSYMTAB_FUNC(nfc_add_se, "");
SYMBOL_FLAGS(nfc_add_se, 0x00);
KSYMTAB_FUNC(nfc_remove_se, "");
SYMBOL_FLAGS(nfc_remove_se, 0x00);
KSYMTAB_FUNC(nfc_se_transaction, "");
SYMBOL_FLAGS(nfc_se_transaction, 0x00);
KSYMTAB_FUNC(nfc_se_connectivity, "");
SYMBOL_FLAGS(nfc_se_connectivity, 0x00);
KSYMTAB_DATA(nfc_class, "");
SYMBOL_FLAGS(nfc_class, 0x00);
KSYMTAB_FUNC(nfc_allocate_device, "");
SYMBOL_FLAGS(nfc_allocate_device, 0x00);
KSYMTAB_FUNC(nfc_register_device, "");
SYMBOL_FLAGS(nfc_register_device, 0x00);
KSYMTAB_FUNC(nfc_unregister_rfkill, "");
SYMBOL_FLAGS(nfc_unregister_rfkill, 0x00);
KSYMTAB_FUNC(nfc_remove_device, "");
SYMBOL_FLAGS(nfc_remove_device, 0x00);
KSYMTAB_FUNC(nfc_unregister_device, "");
SYMBOL_FLAGS(nfc_unregister_device, 0x00);
KSYMTAB_FUNC(__nfc_alloc_vendor_cmd_reply_skb, "");
SYMBOL_FLAGS(__nfc_alloc_vendor_cmd_reply_skb, 0x00);
KSYMTAB_FUNC(nfc_vendor_cmd_reply, "");
SYMBOL_FLAGS(nfc_vendor_cmd_reply, 0x00);
KSYMTAB_FUNC(nfc_proto_register, "");
SYMBOL_FLAGS(nfc_proto_register, 0x00);
KSYMTAB_FUNC(nfc_proto_unregister, "");
SYMBOL_FLAGS(nfc_proto_unregister, 0x00);
KSYMTAB_FUNC(nfc_send_to_raw_sock, "");
SYMBOL_FLAGS(nfc_send_to_raw_sock, 0x00);

MODULE_INFO(depends, "rfkill");


MODULE_INFO(srcversion, "36375FCB0C663081CCC3051");
