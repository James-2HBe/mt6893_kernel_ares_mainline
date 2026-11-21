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

KSYMTAB_DATA(llc_sap_list, "");
SYMBOL_FLAGS(llc_sap_list, 0x00);
KSYMTAB_FUNC(llc_sap_find, "");
SYMBOL_FLAGS(llc_sap_find, 0x00);
KSYMTAB_FUNC(llc_sap_open, "");
SYMBOL_FLAGS(llc_sap_open, 0x00);
KSYMTAB_FUNC(llc_sap_close, "");
SYMBOL_FLAGS(llc_sap_close, 0x00);
KSYMTAB_FUNC(llc_add_pack, "");
SYMBOL_FLAGS(llc_add_pack, 0x00);
KSYMTAB_FUNC(llc_remove_pack, "");
SYMBOL_FLAGS(llc_remove_pack, 0x00);
KSYMTAB_FUNC(llc_set_station_handler, "");
SYMBOL_FLAGS(llc_set_station_handler, 0x00);
KSYMTAB_FUNC(llc_mac_hdr_init, "");
SYMBOL_FLAGS(llc_mac_hdr_init, 0x00);
KSYMTAB_FUNC(llc_build_and_send_ui_pkt, "");
SYMBOL_FLAGS(llc_build_and_send_ui_pkt, 0x00);

MODULE_INFO(depends, "");

