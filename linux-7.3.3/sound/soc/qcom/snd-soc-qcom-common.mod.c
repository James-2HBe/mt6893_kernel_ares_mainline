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

KSYMTAB_FUNC(qcom_snd_get_dai_tdm_slots, "");
SYMBOL_FLAGS(qcom_snd_get_dai_tdm_slots, 0x01);
KSYMTAB_FUNC(qcom_snd_apply_dai_tdm_slots_cfg, "");
SYMBOL_FLAGS(qcom_snd_apply_dai_tdm_slots_cfg, 0x01);
KSYMTAB_FUNC(qcom_snd_apply_dai_tdm_slots, "");
SYMBOL_FLAGS(qcom_snd_apply_dai_tdm_slots, 0x01);
KSYMTAB_FUNC(qcom_snd_parse_of, "");
SYMBOL_FLAGS(qcom_snd_parse_of, 0x01);
KSYMTAB_FUNC(qcom_snd_wcd_jack_setup, "");
SYMBOL_FLAGS(qcom_snd_wcd_jack_setup, 0x01);
KSYMTAB_FUNC(qcom_snd_dp_jack_setup, "");
SYMBOL_FLAGS(qcom_snd_dp_jack_setup, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd");

