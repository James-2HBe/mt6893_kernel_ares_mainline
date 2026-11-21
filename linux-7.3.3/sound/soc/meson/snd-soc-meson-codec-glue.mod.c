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

KSYMTAB_FUNC(meson_codec_glue_input_get_data, "");
SYMBOL_FLAGS(meson_codec_glue_input_get_data, 0x01);
KSYMTAB_FUNC(meson_codec_glue_input_hw_params, "");
SYMBOL_FLAGS(meson_codec_glue_input_hw_params, 0x01);
KSYMTAB_FUNC(meson_codec_glue_input_set_fmt, "");
SYMBOL_FLAGS(meson_codec_glue_input_set_fmt, 0x01);
KSYMTAB_FUNC(meson_codec_glue_output_startup, "");
SYMBOL_FLAGS(meson_codec_glue_output_startup, 0x01);
KSYMTAB_FUNC(meson_codec_glue_input_dai_probe, "");
SYMBOL_FLAGS(meson_codec_glue_input_dai_probe, 0x01);
KSYMTAB_FUNC(meson_codec_glue_input_dai_remove, "");
SYMBOL_FLAGS(meson_codec_glue_input_dai_remove, 0x01);

MODULE_INFO(depends, "snd-pcm,snd-soc-core");

