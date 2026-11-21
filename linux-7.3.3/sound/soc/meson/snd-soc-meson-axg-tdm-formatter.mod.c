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

KSYMTAB_FUNC(axg_tdm_formatter_set_channel_masks, "");
SYMBOL_FLAGS(axg_tdm_formatter_set_channel_masks, 0x01);
KSYMTAB_FUNC(axg_tdm_formatter_event, "");
SYMBOL_FLAGS(axg_tdm_formatter_event, 0x01);
KSYMTAB_FUNC(axg_tdm_formatter_probe, "");
SYMBOL_FLAGS(axg_tdm_formatter_probe, 0x01);
KSYMTAB_FUNC(axg_tdm_stream_start, "");
SYMBOL_FLAGS(axg_tdm_stream_start, 0x01);
KSYMTAB_FUNC(axg_tdm_stream_stop, "");
SYMBOL_FLAGS(axg_tdm_stream_stop, 0x01);
KSYMTAB_FUNC(axg_tdm_stream_alloc, "");
SYMBOL_FLAGS(axg_tdm_stream_alloc, 0x01);
KSYMTAB_FUNC(axg_tdm_stream_free, "");
SYMBOL_FLAGS(axg_tdm_stream_free, 0x01);
KSYMTAB_FUNC(axg_tdm_stream_set_cont_clocks, "");
SYMBOL_FLAGS(axg_tdm_stream_set_cont_clocks, 0x01);

MODULE_INFO(depends, "snd-soc-core");

