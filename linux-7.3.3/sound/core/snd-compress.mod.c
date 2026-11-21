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

KSYMTAB_FUNC(snd_compr_malloc_pages, "");
SYMBOL_FLAGS(snd_compr_malloc_pages, 0x00);
KSYMTAB_FUNC(snd_compr_free_pages, "");
SYMBOL_FLAGS(snd_compr_free_pages, 0x00);
KSYMTAB_FUNC(snd_compr_stop_error, "");
SYMBOL_FLAGS(snd_compr_stop_error, 0x01);
KSYMTAB_FUNC(snd_compr_task_finished, "");
SYMBOL_FLAGS(snd_compr_task_finished, 0x01);
KSYMTAB_FUNC(snd_compress_new, "");
SYMBOL_FLAGS(snd_compress_new, 0x01);

MODULE_INFO(depends, "snd-pcm,snd");

