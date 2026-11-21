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

KSYMTAB_FUNC(q6dsp_map_channels, "");
SYMBOL_FLAGS(q6dsp_map_channels, 0x01);
KSYMTAB_FUNC(q6dsp_get_channel_allocation, "");
SYMBOL_FLAGS(q6dsp_get_channel_allocation, 0x01);
KSYMTAB_FUNC(q6dsp_audio_ports_of_xlate_dai_name, "");
SYMBOL_FLAGS(q6dsp_audio_ports_of_xlate_dai_name, 0x01);
KSYMTAB_FUNC(q6dsp_audio_ports_set_config, "");
SYMBOL_FLAGS(q6dsp_audio_ports_set_config, 0x01);
KSYMTAB_FUNC(q6dsp_clock_dev_probe, "");
SYMBOL_FLAGS(q6dsp_clock_dev_probe, 0x01);

MODULE_INFO(depends, "");

