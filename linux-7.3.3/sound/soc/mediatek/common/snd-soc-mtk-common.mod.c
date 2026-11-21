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

KSYMTAB_FUNC(mtk_afe_combine_sub_dai, "");
SYMBOL_FLAGS(mtk_afe_combine_sub_dai, 0x01);
KSYMTAB_FUNC(mtk_afe_add_sub_dai_control, "");
SYMBOL_FLAGS(mtk_afe_add_sub_dai_control, 0x01);
KSYMTAB_FUNC(mtk_afe_pcm_pointer, "");
SYMBOL_FLAGS(mtk_afe_pcm_pointer, 0x01);
KSYMTAB_FUNC(mtk_afe_pcm_new, "");
SYMBOL_FLAGS(mtk_afe_pcm_new, 0x01);
KSYMTAB_DATA(mtk_afe_pcm_platform, "");
SYMBOL_FLAGS(mtk_afe_pcm_platform, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_startup, "");
SYMBOL_FLAGS(mtk_afe_fe_startup, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_shutdown, "");
SYMBOL_FLAGS(mtk_afe_fe_shutdown, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_hw_params, "");
SYMBOL_FLAGS(mtk_afe_fe_hw_params, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_hw_free, "");
SYMBOL_FLAGS(mtk_afe_fe_hw_free, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_trigger, "");
SYMBOL_FLAGS(mtk_afe_fe_trigger, 0x01);
KSYMTAB_FUNC(mtk_afe_fe_prepare, "");
SYMBOL_FLAGS(mtk_afe_fe_prepare, 0x01);
KSYMTAB_DATA(mtk_afe_fe_ops, "");
SYMBOL_FLAGS(mtk_afe_fe_ops, 0x01);
KSYMTAB_FUNC(mtk_dynamic_irq_acquire, "");
SYMBOL_FLAGS(mtk_dynamic_irq_acquire, 0x01);
KSYMTAB_FUNC(mtk_dynamic_irq_release, "");
SYMBOL_FLAGS(mtk_dynamic_irq_release, 0x01);
KSYMTAB_FUNC(mtk_afe_suspend, "");
SYMBOL_FLAGS(mtk_afe_suspend, 0x01);
KSYMTAB_FUNC(mtk_afe_resume, "");
SYMBOL_FLAGS(mtk_afe_resume, 0x01);
KSYMTAB_FUNC(mtk_memif_set_enable, "");
SYMBOL_FLAGS(mtk_memif_set_enable, 0x01);
KSYMTAB_FUNC(mtk_memif_set_disable, "");
SYMBOL_FLAGS(mtk_memif_set_disable, 0x01);
KSYMTAB_FUNC(mtk_memif_set_addr, "");
SYMBOL_FLAGS(mtk_memif_set_addr, 0x01);
KSYMTAB_FUNC(mtk_memif_set_channel, "");
SYMBOL_FLAGS(mtk_memif_set_channel, 0x01);
KSYMTAB_FUNC(mtk_memif_set_rate_substream, "");
SYMBOL_FLAGS(mtk_memif_set_rate_substream, 0x01);
KSYMTAB_FUNC(mtk_memif_set_format, "");
SYMBOL_FLAGS(mtk_memif_set_format, 0x01);
KSYMTAB_FUNC(mtk_memif_set_pbuf_size, "");
SYMBOL_FLAGS(mtk_memif_set_pbuf_size, 0x01);
KSYMTAB_FUNC(mtk_sof_dai_link_fixup, "");
SYMBOL_FLAGS(mtk_sof_dai_link_fixup, 0x01);
KSYMTAB_FUNC(mtk_sof_card_probe, "");
SYMBOL_FLAGS(mtk_sof_card_probe, 0x01);
KSYMTAB_FUNC(mtk_sof_card_late_probe, "");
SYMBOL_FLAGS(mtk_sof_card_late_probe, 0x01);
KSYMTAB_FUNC(mtk_sof_dailink_parse_of, "");
SYMBOL_FLAGS(mtk_sof_dailink_parse_of, 0x01);
KSYMTAB_FUNC(parse_dai_link_info, "");
SYMBOL_FLAGS(parse_dai_link_info, 0x01);
KSYMTAB_FUNC(clean_card_reference, "");
SYMBOL_FLAGS(clean_card_reference, 0x01);
KSYMTAB_FUNC(mtk_soundcard_startup, "");
SYMBOL_FLAGS(mtk_soundcard_startup, 0x01);
KSYMTAB_DATA(mtk_soundcard_common_playback_ops, "");
SYMBOL_FLAGS(mtk_soundcard_common_playback_ops, 0x01);
KSYMTAB_DATA(mtk_soundcard_common_capture_ops, "");
SYMBOL_FLAGS(mtk_soundcard_common_capture_ops, 0x01);
KSYMTAB_FUNC(mtk_soundcard_common_probe, "");
SYMBOL_FLAGS(mtk_soundcard_common_probe, 0x01);
KSYMTAB_FUNC(mtk_adda_dl_rate_transform, "");
SYMBOL_FLAGS(mtk_adda_dl_rate_transform, 0x01);
KSYMTAB_FUNC(mtk_adda_ul_rate_transform, "");
SYMBOL_FLAGS(mtk_adda_ul_rate_transform, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm");

