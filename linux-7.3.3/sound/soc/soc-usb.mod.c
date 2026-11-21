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

KSYMTAB_FUNC(snd_soc_usb_setup_offload_jack, "");
SYMBOL_FLAGS(snd_soc_usb_setup_offload_jack, 0x01);
KSYMTAB_FUNC(snd_soc_usb_update_offload_route, "");
SYMBOL_FLAGS(snd_soc_usb_update_offload_route, 0x01);
KSYMTAB_FUNC(snd_soc_usb_find_priv_data, "");
SYMBOL_FLAGS(snd_soc_usb_find_priv_data, 0x01);
KSYMTAB_FUNC(snd_soc_usb_find_supported_format, "");
SYMBOL_FLAGS(snd_soc_usb_find_supported_format, 0x01);
KSYMTAB_FUNC(snd_soc_usb_allocate_port, "");
SYMBOL_FLAGS(snd_soc_usb_allocate_port, 0x01);
KSYMTAB_FUNC(snd_soc_usb_free_port, "");
SYMBOL_FLAGS(snd_soc_usb_free_port, 0x01);
KSYMTAB_FUNC(snd_soc_usb_add_port, "");
SYMBOL_FLAGS(snd_soc_usb_add_port, 0x01);
KSYMTAB_FUNC(snd_soc_usb_remove_port, "");
SYMBOL_FLAGS(snd_soc_usb_remove_port, 0x01);
KSYMTAB_FUNC(snd_soc_usb_connect, "");
SYMBOL_FLAGS(snd_soc_usb_connect, 0x01);
KSYMTAB_FUNC(snd_soc_usb_disconnect, "");
SYMBOL_FLAGS(snd_soc_usb_disconnect, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-usb-audio");

