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

KSYMTAB_FUNC(wcd_get_micb_vout_ctl_val, "");
SYMBOL_FLAGS(wcd_get_micb_vout_ctl_val, 0x01);
KSYMTAB_FUNC(wcd_dt_parse_micbias_info, "");
SYMBOL_FLAGS(wcd_dt_parse_micbias_info, 0x01);
KSYMTAB_DATA(wcd_sdw_component_ops, "");
SYMBOL_FLAGS(wcd_sdw_component_ops, 0x01);
KSYMTAB_FUNC(wcd_update_status, "");
SYMBOL_FLAGS(wcd_update_status, 0x01);
KSYMTAB_FUNC(wcd_bus_config, "");
SYMBOL_FLAGS(wcd_bus_config, 0x01);
KSYMTAB_FUNC(wcd_interrupt_callback, "");
SYMBOL_FLAGS(wcd_interrupt_callback, 0x01);

MODULE_INFO(depends, "soundwire-bus");

