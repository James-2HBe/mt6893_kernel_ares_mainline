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

KSYMTAB_FUNC(wcd_mbhc_event_notify, "");
SYMBOL_FLAGS(wcd_mbhc_event_notify, 0x01);
KSYMTAB_FUNC(wcd_mbhc_typec_report_unplug, "");
SYMBOL_FLAGS(wcd_mbhc_typec_report_unplug, 0x01);
KSYMTAB_FUNC(wcd_mbhc_typec_report_plug, "");
SYMBOL_FLAGS(wcd_mbhc_typec_report_plug, 0x01);
KSYMTAB_FUNC(wcd_mbhc_get_impedance, "");
SYMBOL_FLAGS(wcd_mbhc_get_impedance, 0x00);
KSYMTAB_FUNC(wcd_mbhc_set_hph_type, "");
SYMBOL_FLAGS(wcd_mbhc_set_hph_type, 0x00);
KSYMTAB_FUNC(wcd_mbhc_get_hph_type, "");
SYMBOL_FLAGS(wcd_mbhc_get_hph_type, 0x00);
KSYMTAB_FUNC(wcd_mbhc_start, "");
SYMBOL_FLAGS(wcd_mbhc_start, 0x00);
KSYMTAB_FUNC(wcd_mbhc_stop, "");
SYMBOL_FLAGS(wcd_mbhc_stop, 0x00);
KSYMTAB_FUNC(wcd_dt_parse_mbhc_data, "");
SYMBOL_FLAGS(wcd_dt_parse_mbhc_data, 0x00);
KSYMTAB_FUNC(wcd_mbhc_init, "");
SYMBOL_FLAGS(wcd_mbhc_init, 0x00);
KSYMTAB_FUNC(wcd_mbhc_deinit, "");
SYMBOL_FLAGS(wcd_mbhc_deinit, 0x00);

MODULE_INFO(depends, "snd-soc-core");

