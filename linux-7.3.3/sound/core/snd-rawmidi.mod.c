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

KSYMTAB_FUNC(snd_rawmidi_drop_output, "");
SYMBOL_FLAGS(snd_rawmidi_drop_output, 0x00);
KSYMTAB_FUNC(snd_rawmidi_drain_output, "");
SYMBOL_FLAGS(snd_rawmidi_drain_output, 0x00);
KSYMTAB_FUNC(snd_rawmidi_drain_input, "");
SYMBOL_FLAGS(snd_rawmidi_drain_input, 0x00);
KSYMTAB_FUNC(snd_rawmidi_kernel_open_nested, "");
SYMBOL_FLAGS(snd_rawmidi_kernel_open_nested, 0x00);
KSYMTAB_FUNC(snd_rawmidi_kernel_release_nested, "");
SYMBOL_FLAGS(snd_rawmidi_kernel_release_nested, 0x00);
KSYMTAB_FUNC(snd_rawmidi_info_select, "");
SYMBOL_FLAGS(snd_rawmidi_info_select, 0x00);
KSYMTAB_FUNC(snd_rawmidi_output_params, "");
SYMBOL_FLAGS(snd_rawmidi_output_params, 0x00);
KSYMTAB_FUNC(snd_rawmidi_input_params, "");
SYMBOL_FLAGS(snd_rawmidi_input_params, 0x00);
KSYMTAB_FUNC(snd_rawmidi_receive, "");
SYMBOL_FLAGS(snd_rawmidi_receive, 0x00);
KSYMTAB_FUNC(snd_rawmidi_kernel_read, "");
SYMBOL_FLAGS(snd_rawmidi_kernel_read, 0x00);
KSYMTAB_FUNC(snd_rawmidi_transmit_empty, "");
SYMBOL_FLAGS(snd_rawmidi_transmit_empty, 0x00);
KSYMTAB_FUNC(snd_rawmidi_transmit_peek, "");
SYMBOL_FLAGS(snd_rawmidi_transmit_peek, 0x00);
KSYMTAB_FUNC(snd_rawmidi_transmit_ack, "");
SYMBOL_FLAGS(snd_rawmidi_transmit_ack, 0x00);
KSYMTAB_FUNC(snd_rawmidi_transmit, "");
SYMBOL_FLAGS(snd_rawmidi_transmit, 0x00);
KSYMTAB_FUNC(snd_rawmidi_proceed, "");
SYMBOL_FLAGS(snd_rawmidi_proceed, 0x00);
KSYMTAB_FUNC(snd_rawmidi_kernel_write, "");
SYMBOL_FLAGS(snd_rawmidi_kernel_write, 0x00);
KSYMTAB_FUNC(snd_rawmidi_init, "");
SYMBOL_FLAGS(snd_rawmidi_init, 0x01);
KSYMTAB_FUNC(snd_rawmidi_new, "");
SYMBOL_FLAGS(snd_rawmidi_new, 0x00);
KSYMTAB_FUNC(snd_rawmidi_free, "");
SYMBOL_FLAGS(snd_rawmidi_free, 0x01);
KSYMTAB_FUNC(snd_rawmidi_set_ops, "");
SYMBOL_FLAGS(snd_rawmidi_set_ops, 0x00);

MODULE_INFO(depends, "snd");

