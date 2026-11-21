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

KSYMTAB_FUNC(snd_usbmidi_disconnect, "");
SYMBOL_FLAGS(snd_usbmidi_disconnect, 0x00);
KSYMTAB_FUNC(snd_usbmidi_input_stop, "");
SYMBOL_FLAGS(snd_usbmidi_input_stop, 0x00);
KSYMTAB_FUNC(snd_usbmidi_input_start, "");
SYMBOL_FLAGS(snd_usbmidi_input_start, 0x00);
KSYMTAB_FUNC(snd_usbmidi_suspend, "");
SYMBOL_FLAGS(snd_usbmidi_suspend, 0x00);
KSYMTAB_FUNC(snd_usbmidi_resume, "");
SYMBOL_FLAGS(snd_usbmidi_resume, 0x00);
KSYMTAB_FUNC(__snd_usbmidi_create, "");
SYMBOL_FLAGS(__snd_usbmidi_create, 0x00);

MODULE_INFO(depends, "snd-rawmidi,snd");

