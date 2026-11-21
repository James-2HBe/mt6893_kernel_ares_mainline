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

KSYMTAB_FUNC(axg_fifo_pcm_trigger, "");
SYMBOL_FLAGS(axg_fifo_pcm_trigger, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_pointer, "");
SYMBOL_FLAGS(axg_fifo_pcm_pointer, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_hw_params, "");
SYMBOL_FLAGS(axg_fifo_pcm_hw_params, 0x01);
KSYMTAB_FUNC(g12a_fifo_pcm_hw_params, "");
SYMBOL_FLAGS(g12a_fifo_pcm_hw_params, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_hw_free, "");
SYMBOL_FLAGS(axg_fifo_pcm_hw_free, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_open, "");
SYMBOL_FLAGS(axg_fifo_pcm_open, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_close, "");
SYMBOL_FLAGS(axg_fifo_pcm_close, 0x01);
KSYMTAB_FUNC(axg_fifo_pcm_new, "");
SYMBOL_FLAGS(axg_fifo_pcm_new, 0x01);
KSYMTAB_FUNC(axg_fifo_probe, "");
SYMBOL_FLAGS(axg_fifo_probe, 0x01);

MODULE_INFO(depends, "snd-pcm,snd-soc-core");

