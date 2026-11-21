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

KSYMTAB_FUNC(snd_hda_hdmi_simple_build_pcms, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_build_pcms, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_unsol_event, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_unsol_event, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_build_controls, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_build_controls, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_init, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_init, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_remove, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_remove, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_pcm_open, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_pcm_open, 0x01);
KSYMTAB_FUNC(snd_hda_hdmi_simple_probe, "SND_HDA_CODEC_HDMI");
SYMBOL_FLAGS(snd_hda_hdmi_simple_probe, 0x01);

MODULE_INFO(depends, "snd-hda-codec,snd,snd-hda-core,snd-pcm");

