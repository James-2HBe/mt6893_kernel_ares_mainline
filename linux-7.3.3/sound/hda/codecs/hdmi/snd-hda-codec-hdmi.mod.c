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

KSYMTAB_FUNC(snd_hda_hdmi_pin_id_to_pin_index, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_pin_id_to_pin_index, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_setup_audio_infoframe, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_setup_audio_infoframe, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_check_presence_and_report, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_check_presence_and_report, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_unsol_event, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_unsol_event, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_setup_stream, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_setup_stream, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_parse_codec, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_parse_codec, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_pcm_prepare, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_pcm_prepare, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_pcm_cleanup, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_pcm_cleanup, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_build_pcms, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_build_pcms, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_build_controls, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_build_controls, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_init_per_pins, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_init_per_pins, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_init, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_init, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_spec_free, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_spec_free, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_remove, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_remove, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_suspend, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_suspend, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_resume, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_resume, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_alloc, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_alloc, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_generic_probe, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_generic_probe, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_acomp_master_bind, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_acomp_master_bind, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_acomp_master_unbind, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_acomp_master_unbind, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_acomp_pin_eld_notify, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_acomp_pin_eld_notify, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_setup_drm_audio_ops, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_setup_drm_audio_ops, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_acomp_init, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_acomp_init, 0x01);

MODULE_INFO(depends, "snd-hda-codec,snd-hda-core,snd,snd-pcm");

MODULE_ALIAS("hdaudio:v00147A47r*a01*");
MODULE_ALIAS("hdaudio:v10951390r*a01*");
MODULE_ALIAS("hdaudio:v10951392r*a01*");
MODULE_ALIAS("hdaudio:v11069F84r*a01*");
MODULE_ALIAS("hdaudio:v11069F85r*a01*");
MODULE_ALIAS("hdaudio:v17E80047r*a01*");
MODULE_ALIAS("hdaudio:v1D179F86r*a01*");
MODULE_ALIAS("hdaudio:v1D179F87r*a01*");
MODULE_ALIAS("hdaudio:v1D179F88r*a01*");
MODULE_ALIAS("hdaudio:v1D179F89r*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Ar*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Br*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Cr*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Dr*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Er*a01*");
MODULE_ALIAS("hdaudio:v1D179F8Fr*a01*");
MODULE_ALIAS("hdaudio:v1D179F90r*a01*");
MODULE_ALIAS("hdaudio:v4C545020r*a01*");
MODULE_ALIAS("hdaudio:v67663D82r*a01*");
MODULE_ALIAS("hdaudio:v67663D83r*a01*");
MODULE_ALIAS("hdaudio:v67663D84r*a01*");
MODULE_ALIAS("hdaudio:v67663D85r*a01*");
MODULE_ALIAS("hdaudio:v67663D86r*a01*");
MODULE_ALIAS("hdaudio:v67663D87r*a01*");
MODULE_ALIAS("hdaudio:v80862801r*a01*");
MODULE_ALIAS("hdaudio:v80862802r*a01*");
MODULE_ALIAS("hdaudio:v80862803r*a01*");
MODULE_ALIAS("hdaudio:v80862880r*a01*");
MODULE_ALIAS("hdaudio:v808629FBr*a01*");
MODULE_ALIAS("hdaudio:v00000101r*a01*");
