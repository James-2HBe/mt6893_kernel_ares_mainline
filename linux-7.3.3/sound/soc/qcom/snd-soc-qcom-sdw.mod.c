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

KSYMTAB_FUNC(qcom_snd_sdw_startup, "");
SYMBOL_FLAGS(qcom_snd_sdw_startup, 0x01);
KSYMTAB_FUNC(qcom_snd_sdw_prepare, "");
SYMBOL_FLAGS(qcom_snd_sdw_prepare, 0x01);
KSYMTAB_FUNC(qcom_snd_sdw_get_stream, "");
SYMBOL_FLAGS(qcom_snd_sdw_get_stream, 0x01);
KSYMTAB_FUNC(qcom_snd_sdw_shutdown, "");
SYMBOL_FLAGS(qcom_snd_sdw_shutdown, 0x01);
KSYMTAB_FUNC(qcom_snd_sdw_hw_free, "");
SYMBOL_FLAGS(qcom_snd_sdw_hw_free, 0x01);

MODULE_INFO(depends, "soundwire-bus,snd-soc-core");

