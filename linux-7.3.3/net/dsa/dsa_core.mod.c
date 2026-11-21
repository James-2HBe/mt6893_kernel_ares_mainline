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

KSYMTAB_FUNC(dsa_devlink_param_get, "");
SYMBOL_FLAGS(dsa_devlink_param_get, 0x01);
KSYMTAB_FUNC(dsa_devlink_param_set, "");
SYMBOL_FLAGS(dsa_devlink_param_set, 0x01);
KSYMTAB_FUNC(dsa_devlink_params_register, "");
SYMBOL_FLAGS(dsa_devlink_params_register, 0x01);
KSYMTAB_FUNC(dsa_devlink_params_unregister, "");
SYMBOL_FLAGS(dsa_devlink_params_unregister, 0x01);
KSYMTAB_FUNC(dsa_devlink_resource_register, "");
SYMBOL_FLAGS(dsa_devlink_resource_register, 0x01);
KSYMTAB_FUNC(dsa_devlink_resources_unregister, "");
SYMBOL_FLAGS(dsa_devlink_resources_unregister, 0x01);
KSYMTAB_FUNC(dsa_devlink_resource_occ_get_register, "");
SYMBOL_FLAGS(dsa_devlink_resource_occ_get_register, 0x01);
KSYMTAB_FUNC(dsa_devlink_resource_occ_get_unregister, "");
SYMBOL_FLAGS(dsa_devlink_resource_occ_get_unregister, 0x01);
KSYMTAB_FUNC(dsa_devlink_region_create, "");
SYMBOL_FLAGS(dsa_devlink_region_create, 0x01);
KSYMTAB_FUNC(dsa_devlink_port_region_create, "");
SYMBOL_FLAGS(dsa_devlink_port_region_create, 0x01);
KSYMTAB_FUNC(dsa_devlink_region_destroy, "");
SYMBOL_FLAGS(dsa_devlink_region_destroy, 0x01);
KSYMTAB_FUNC(dsa_flush_workqueue, "");
SYMBOL_FLAGS(dsa_flush_workqueue, 0x01);
KSYMTAB_FUNC(dsa_switch_find, "");
SYMBOL_FLAGS(dsa_switch_find, 0x01);
KSYMTAB_FUNC(dsa_register_switch, "");
SYMBOL_FLAGS(dsa_register_switch, 0x01);
KSYMTAB_FUNC(dsa_unregister_switch, "");
SYMBOL_FLAGS(dsa_unregister_switch, 0x01);
KSYMTAB_FUNC(dsa_switch_shutdown, "");
SYMBOL_FLAGS(dsa_switch_shutdown, 0x01);
KSYMTAB_FUNC(dsa_switch_suspend, "");
SYMBOL_FLAGS(dsa_switch_suspend, 0x01);
KSYMTAB_FUNC(dsa_switch_resume, "");
SYMBOL_FLAGS(dsa_switch_resume, 0x01);
KSYMTAB_FUNC(dsa_port_from_netdev, "");
SYMBOL_FLAGS(dsa_port_from_netdev, 0x01);
KSYMTAB_FUNC(dsa_fdb_present_in_other_db, "");
SYMBOL_FLAGS(dsa_fdb_present_in_other_db, 0x01);
KSYMTAB_FUNC(dsa_mdb_present_in_other_db, "");
SYMBOL_FLAGS(dsa_mdb_present_in_other_db, 0x01);
KSYMTAB_FUNC(dsa_port_simple_hsr_validate, "");
SYMBOL_FLAGS(dsa_port_simple_hsr_validate, 0x01);
KSYMTAB_FUNC(dsa_port_simple_hsr_join, "");
SYMBOL_FLAGS(dsa_port_simple_hsr_join, 0x01);
KSYMTAB_FUNC(dsa_port_simple_hsr_leave, "");
SYMBOL_FLAGS(dsa_port_simple_hsr_leave, 0x01);
KSYMTAB_FUNC(dsa_supports_eee, "");
SYMBOL_FLAGS(dsa_supports_eee, 0x01);
KSYMTAB_FUNC(dsa_tag_drivers_register, "");
SYMBOL_FLAGS(dsa_tag_drivers_register, 0x01);
KSYMTAB_FUNC(dsa_tag_drivers_unregister, "");
SYMBOL_FLAGS(dsa_tag_drivers_unregister, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_bridge_vid, "");
SYMBOL_FLAGS(dsa_tag_8021q_bridge_vid, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_standalone_vid, "");
SYMBOL_FLAGS(dsa_tag_8021q_standalone_vid, 0x01);
KSYMTAB_FUNC(dsa_8021q_rx_switch_id, "");
SYMBOL_FLAGS(dsa_8021q_rx_switch_id, 0x01);
KSYMTAB_FUNC(dsa_8021q_rx_source_port, "");
SYMBOL_FLAGS(dsa_8021q_rx_source_port, 0x01);
KSYMTAB_FUNC(vid_is_dsa_8021q, "");
SYMBOL_FLAGS(vid_is_dsa_8021q, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_bridge_join, "");
SYMBOL_FLAGS(dsa_tag_8021q_bridge_join, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_bridge_leave, "");
SYMBOL_FLAGS(dsa_tag_8021q_bridge_leave, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_register, "");
SYMBOL_FLAGS(dsa_tag_8021q_register, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_unregister, "");
SYMBOL_FLAGS(dsa_tag_8021q_unregister, 0x01);
KSYMTAB_FUNC(dsa_8021q_xmit, "");
SYMBOL_FLAGS(dsa_8021q_xmit, 0x01);
KSYMTAB_FUNC(dsa_tag_8021q_find_user, "");
SYMBOL_FLAGS(dsa_tag_8021q_find_user, 0x01);
KSYMTAB_FUNC(dsa_8021q_rcv, "");
SYMBOL_FLAGS(dsa_8021q_rcv, 0x01);
KSYMTAB_FUNC(dsa_enqueue_skb, "");
SYMBOL_FLAGS(dsa_enqueue_skb, 0x01);
KSYMTAB_FUNC(dsa_port_phylink_mac_change, "");
SYMBOL_FLAGS(dsa_port_phylink_mac_change, 0x01);
KSYMTAB_FUNC(dsa_user_dev_check, "");
SYMBOL_FLAGS(dsa_user_dev_check, 0x01);

MODULE_INFO(depends, "hsr,bridge,phylink");

