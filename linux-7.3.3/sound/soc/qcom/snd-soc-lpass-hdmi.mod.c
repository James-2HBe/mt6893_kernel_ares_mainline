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

KSYMTAB_DATA(asoc_qcom_lpass_hdmi_dai_ops, "");
SYMBOL_FLAGS(asoc_qcom_lpass_hdmi_dai_ops, 0x01);

MODULE_INFO(depends, "snd-pcm");

