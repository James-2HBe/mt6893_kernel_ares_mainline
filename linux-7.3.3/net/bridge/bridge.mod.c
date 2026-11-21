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

KSYMTAB_DATA(nf_br_ops, "");
SYMBOL_FLAGS(nf_br_ops, 0x01);
KSYMTAB_FUNC(br_fdb_find_port, "");
SYMBOL_FLAGS(br_fdb_find_port, 0x01);
KSYMTAB_FUNC(br_fdb_clear_offload, "");
SYMBOL_FLAGS(br_fdb_clear_offload, 0x01);
KSYMTAB_FUNC(br_dev_queue_push_xmit, "");
SYMBOL_FLAGS(br_dev_queue_push_xmit, 0x01);
KSYMTAB_FUNC(br_forward_finish, "");
SYMBOL_FLAGS(br_forward_finish, 0x01);
KSYMTAB_FUNC(br_forward, "");
SYMBOL_FLAGS(br_forward, 0x01);
KSYMTAB_FUNC(br_port_flag_is_set, "");
SYMBOL_FLAGS(br_port_flag_is_set, 0x01);
KSYMTAB_FUNC(br_handle_frame_finish, "");
SYMBOL_FLAGS(br_handle_frame_finish, 0x01);
KSYMTAB_FUNC(br_port_get_stp_state, "");
SYMBOL_FLAGS(br_port_get_stp_state, 0x01);
KSYMTAB_FUNC(br_get_ageing_time, "");
SYMBOL_FLAGS(br_get_ageing_time, 0x01);
KSYMTAB_FUNC(br_multicast_enabled, "");
SYMBOL_FLAGS(br_multicast_enabled, 0x01);
KSYMTAB_FUNC(br_multicast_router, "");
SYMBOL_FLAGS(br_multicast_router, 0x01);
KSYMTAB_FUNC(br_multicast_list_adjacent, "");
SYMBOL_FLAGS(br_multicast_list_adjacent, 0x01);
KSYMTAB_FUNC(br_multicast_has_querier_anywhere, "");
SYMBOL_FLAGS(br_multicast_has_querier_anywhere, 0x01);
KSYMTAB_FUNC(br_multicast_has_querier_adjacent, "");
SYMBOL_FLAGS(br_multicast_has_querier_adjacent, 0x01);
KSYMTAB_FUNC(br_multicast_has_router_adjacent, "");
SYMBOL_FLAGS(br_multicast_has_router_adjacent, 0x01);
KSYMTAB_FUNC(br_vlan_enabled, "");
SYMBOL_FLAGS(br_vlan_enabled, 0x01);
KSYMTAB_FUNC(br_vlan_get_proto, "");
SYMBOL_FLAGS(br_vlan_get_proto, 0x01);
KSYMTAB_FUNC(br_vlan_get_pvid, "");
SYMBOL_FLAGS(br_vlan_get_pvid, 0x01);
KSYMTAB_FUNC(br_vlan_get_pvid_rcu, "");
SYMBOL_FLAGS(br_vlan_get_pvid_rcu, 0x01);
KSYMTAB_FUNC(br_vlan_get_info, "");
SYMBOL_FLAGS(br_vlan_get_info, 0x01);
KSYMTAB_FUNC(br_vlan_get_info_rcu, "");
SYMBOL_FLAGS(br_vlan_get_info_rcu, 0x01);
KSYMTAB_FUNC(br_mst_enabled, "");
SYMBOL_FLAGS(br_mst_enabled, 0x01);
KSYMTAB_FUNC(br_mst_get_info, "");
SYMBOL_FLAGS(br_mst_get_info, 0x01);
KSYMTAB_FUNC(br_mst_get_state, "");
SYMBOL_FLAGS(br_mst_get_state, 0x01);

MODULE_INFO(depends, "stp,llc");


MODULE_INFO(srcversion, "5930D9730B9CC7CF20005E1");
