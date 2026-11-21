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

KSYMTAB_DATA(asoc_qcom_lpass_cpu_dai_ops, "");
SYMBOL_FLAGS(asoc_qcom_lpass_cpu_dai_ops, 0x01);
KSYMTAB_DATA(asoc_qcom_lpass_cpu_dai_ops2, "");
SYMBOL_FLAGS(asoc_qcom_lpass_cpu_dai_ops2, 0x01);
KSYMTAB_FUNC(asoc_qcom_lpass_cpu_platform_probe, "");
SYMBOL_FLAGS(asoc_qcom_lpass_cpu_platform_probe, 0x01);
KSYMTAB_FUNC(asoc_qcom_lpass_cpu_platform_remove, "");
SYMBOL_FLAGS(asoc_qcom_lpass_cpu_platform_remove, 0x01);
KSYMTAB_FUNC(asoc_qcom_lpass_cpu_platform_shutdown, "");
SYMBOL_FLAGS(asoc_qcom_lpass_cpu_platform_shutdown, 0x01);

MODULE_INFO(depends, "snd-pcm,snd-soc-core,snd-soc-lpass-platform");

