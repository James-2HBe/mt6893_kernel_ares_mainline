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

KSYMTAB_FUNC(qcom_snd_usb_offload_jack_setup, "");
SYMBOL_FLAGS(qcom_snd_usb_offload_jack_setup, 0x01);
KSYMTAB_FUNC(qcom_snd_usb_offload_jack_remove, "");
SYMBOL_FLAGS(qcom_snd_usb_offload_jack_remove, 0x01);

MODULE_INFO(depends, "soc-usb,snd-soc-core");

