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

KSYMTAB_FUNC(q6afe_get_port_id, "");
SYMBOL_FLAGS(q6afe_get_port_id, 0x01);
KSYMTAB_FUNC(q6afe_set_lpass_clock, "");
SYMBOL_FLAGS(q6afe_set_lpass_clock, 0x01);
KSYMTAB_FUNC(q6afe_port_set_sysclk, "");
SYMBOL_FLAGS(q6afe_port_set_sysclk, 0x01);
KSYMTAB_FUNC(q6afe_port_stop, "");
SYMBOL_FLAGS(q6afe_port_stop, 0x01);
KSYMTAB_FUNC(q6afe_slim_port_prepare, "");
SYMBOL_FLAGS(q6afe_slim_port_prepare, 0x01);
KSYMTAB_FUNC(q6afe_tdm_port_prepare, "");
SYMBOL_FLAGS(q6afe_tdm_port_prepare, 0x01);
KSYMTAB_FUNC(afe_port_send_usb_dev_param, "");
SYMBOL_FLAGS(afe_port_send_usb_dev_param, 0x01);
KSYMTAB_FUNC(q6afe_usb_port_prepare, "");
SYMBOL_FLAGS(q6afe_usb_port_prepare, 0x01);
KSYMTAB_FUNC(q6afe_hdmi_port_prepare, "");
SYMBOL_FLAGS(q6afe_hdmi_port_prepare, 0x01);
KSYMTAB_FUNC(q6afe_i2s_port_prepare, "");
SYMBOL_FLAGS(q6afe_i2s_port_prepare, 0x01);
KSYMTAB_FUNC(q6afe_cdc_dma_port_prepare, "");
SYMBOL_FLAGS(q6afe_cdc_dma_port_prepare, 0x01);
KSYMTAB_FUNC(q6afe_port_start, "");
SYMBOL_FLAGS(q6afe_port_start, 0x01);
KSYMTAB_FUNC(q6afe_port_get_from_id, "");
SYMBOL_FLAGS(q6afe_port_get_from_id, 0x01);
KSYMTAB_FUNC(q6afe_port_put, "");
SYMBOL_FLAGS(q6afe_port_put, 0x01);
KSYMTAB_FUNC(q6afe_unvote_lpass_core_hw, "");
SYMBOL_FLAGS(q6afe_unvote_lpass_core_hw, 0x00);
KSYMTAB_FUNC(q6afe_vote_lpass_core_hw, "");
SYMBOL_FLAGS(q6afe_vote_lpass_core_hw, 0x00);

MODULE_INFO(depends, "apr,q6core");

MODULE_ALIAS("of:N*T*Cqcom,q6afe");
MODULE_ALIAS("of:N*T*Cqcom,q6afeC*");
