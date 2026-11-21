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

KSYMTAB_FUNC(nci_get_conn_info_by_dest_type_params, "");
SYMBOL_FLAGS(nci_get_conn_info_by_dest_type_params, 0x00);
KSYMTAB_FUNC(nci_req_complete, "");
SYMBOL_FLAGS(nci_req_complete, 0x00);
KSYMTAB_FUNC(nci_prop_cmd, "");
SYMBOL_FLAGS(nci_prop_cmd, 0x00);
KSYMTAB_FUNC(nci_core_cmd, "");
SYMBOL_FLAGS(nci_core_cmd, 0x00);
KSYMTAB_FUNC(nci_core_reset, "");
SYMBOL_FLAGS(nci_core_reset, 0x00);
KSYMTAB_FUNC(nci_core_init, "");
SYMBOL_FLAGS(nci_core_init, 0x00);
KSYMTAB_FUNC(nci_nfcc_loopback, "");
SYMBOL_FLAGS(nci_nfcc_loopback, 0x00);
KSYMTAB_FUNC(nci_set_config, "");
SYMBOL_FLAGS(nci_set_config, 0x00);
KSYMTAB_FUNC(nci_nfcee_discover, "");
SYMBOL_FLAGS(nci_nfcee_discover, 0x00);
KSYMTAB_FUNC(nci_nfcee_mode_set, "");
SYMBOL_FLAGS(nci_nfcee_mode_set, 0x00);
KSYMTAB_FUNC(nci_core_conn_create, "");
SYMBOL_FLAGS(nci_core_conn_create, 0x00);
KSYMTAB_FUNC(nci_core_conn_close, "");
SYMBOL_FLAGS(nci_core_conn_close, 0x00);
KSYMTAB_FUNC(nci_allocate_device, "");
SYMBOL_FLAGS(nci_allocate_device, 0x00);
KSYMTAB_FUNC(nci_free_device, "");
SYMBOL_FLAGS(nci_free_device, 0x00);
KSYMTAB_FUNC(nci_register_device, "");
SYMBOL_FLAGS(nci_register_device, 0x00);
KSYMTAB_FUNC(nci_unregister_device, "");
SYMBOL_FLAGS(nci_unregister_device, 0x00);
KSYMTAB_FUNC(nci_recv_frame, "");
SYMBOL_FLAGS(nci_recv_frame, 0x00);
KSYMTAB_FUNC(nci_send_frame, "");
SYMBOL_FLAGS(nci_send_frame, 0x00);
KSYMTAB_FUNC(nci_send_cmd, "");
SYMBOL_FLAGS(nci_send_cmd, 0x00);
KSYMTAB_FUNC(nci_conn_max_data_pkt_payload_size, "");
SYMBOL_FLAGS(nci_conn_max_data_pkt_payload_size, 0x00);
KSYMTAB_FUNC(nci_send_data, "");
SYMBOL_FLAGS(nci_send_data, 0x00);
KSYMTAB_FUNC(nci_to_errno, "");
SYMBOL_FLAGS(nci_to_errno, 0x00);
KSYMTAB_FUNC(nci_hci_send_event, "");
SYMBOL_FLAGS(nci_hci_send_event, 0x00);
KSYMTAB_FUNC(nci_hci_send_cmd, "");
SYMBOL_FLAGS(nci_hci_send_cmd, 0x00);
KSYMTAB_FUNC(nci_hci_clear_all_pipes, "");
SYMBOL_FLAGS(nci_hci_clear_all_pipes, 0x00);
KSYMTAB_FUNC(nci_hci_open_pipe, "");
SYMBOL_FLAGS(nci_hci_open_pipe, 0x00);
KSYMTAB_FUNC(nci_hci_set_param, "");
SYMBOL_FLAGS(nci_hci_set_param, 0x00);
KSYMTAB_FUNC(nci_hci_get_param, "");
SYMBOL_FLAGS(nci_hci_get_param, 0x00);
KSYMTAB_FUNC(nci_hci_connect_gate, "");
SYMBOL_FLAGS(nci_hci_connect_gate, 0x00);
KSYMTAB_FUNC(nci_hci_dev_session_init, "");
SYMBOL_FLAGS(nci_hci_dev_session_init, 0x00);

MODULE_INFO(depends, "nfc");

