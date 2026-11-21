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

KSYMTAB_FUNC(snd_dmaengine_pcm_get_chan, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_get_chan, 0x01);
KSYMTAB_FUNC(snd_hwparams_to_dma_slave_config, "");
SYMBOL_FLAGS(snd_hwparams_to_dma_slave_config, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_set_config_from_dai_data, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_set_config_from_dai_data, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_trigger, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_trigger, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_pointer_no_residue, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_pointer_no_residue, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_pointer, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_pointer, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_request_channel, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_request_channel, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_open, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_open, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_sync_stop, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_sync_stop, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_close, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_close, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_close_release_chan, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_close_release_chan, 0x01);
KSYMTAB_FUNC(snd_dmaengine_pcm_refine_runtime_hwparams, "");
SYMBOL_FLAGS(snd_dmaengine_pcm_refine_runtime_hwparams, 0x01);

MODULE_INFO(depends, "snd-pcm");

