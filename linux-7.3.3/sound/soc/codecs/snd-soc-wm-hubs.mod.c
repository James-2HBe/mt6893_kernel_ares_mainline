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

KSYMTAB_DATA(wm_hubs_spkmix_tlv, "");
SYMBOL_FLAGS(wm_hubs_spkmix_tlv, 0x01);
KSYMTAB_FUNC(wm_hubs_dcs_done, "");
SYMBOL_FLAGS(wm_hubs_dcs_done, 0x01);
KSYMTAB_FUNC(wm_hubs_update_class_w, "");
SYMBOL_FLAGS(wm_hubs_update_class_w, 0x01);
KSYMTAB_DATA(wm_hubs_hpl_mux, "");
SYMBOL_FLAGS(wm_hubs_hpl_mux, 0x01);
KSYMTAB_DATA(wm_hubs_hpr_mux, "");
SYMBOL_FLAGS(wm_hubs_hpr_mux, 0x01);
KSYMTAB_FUNC(wm_hubs_add_analogue_controls, "");
SYMBOL_FLAGS(wm_hubs_add_analogue_controls, 0x01);
KSYMTAB_FUNC(wm_hubs_add_analogue_routes, "");
SYMBOL_FLAGS(wm_hubs_add_analogue_routes, 0x01);
KSYMTAB_FUNC(wm_hubs_handle_analogue_pdata, "");
SYMBOL_FLAGS(wm_hubs_handle_analogue_pdata, 0x01);
KSYMTAB_FUNC(wm_hubs_vmid_ena, "");
SYMBOL_FLAGS(wm_hubs_vmid_ena, 0x01);
KSYMTAB_FUNC(wm_hubs_set_bias_level, "");
SYMBOL_FLAGS(wm_hubs_set_bias_level, 0x01);

MODULE_INFO(depends, "snd-soc-core");

