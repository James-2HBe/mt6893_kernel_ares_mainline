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

KSYMTAB_FUNC(tegra_pcm_platform_register, "");
SYMBOL_FLAGS(tegra_pcm_platform_register, 0x01);
KSYMTAB_FUNC(devm_tegra_pcm_platform_register, "");
SYMBOL_FLAGS(devm_tegra_pcm_platform_register, 0x01);
KSYMTAB_FUNC(tegra_pcm_platform_register_with_chan_names, "");
SYMBOL_FLAGS(tegra_pcm_platform_register_with_chan_names, 0x01);
KSYMTAB_FUNC(tegra_pcm_platform_unregister, "");
SYMBOL_FLAGS(tegra_pcm_platform_unregister, 0x01);
KSYMTAB_FUNC(tegra_pcm_open, "");
SYMBOL_FLAGS(tegra_pcm_open, 0x01);
KSYMTAB_FUNC(tegra_pcm_close, "");
SYMBOL_FLAGS(tegra_pcm_close, 0x01);
KSYMTAB_FUNC(tegra_pcm_hw_params, "");
SYMBOL_FLAGS(tegra_pcm_hw_params, 0x01);
KSYMTAB_FUNC(tegra_pcm_pointer, "");
SYMBOL_FLAGS(tegra_pcm_pointer, 0x01);
KSYMTAB_FUNC(tegra_pcm_new, "");
SYMBOL_FLAGS(tegra_pcm_new, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm,snd-pcm-dmaengine");

