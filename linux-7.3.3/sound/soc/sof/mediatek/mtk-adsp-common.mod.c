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

KSYMTAB_FUNC(mtk_adsp_dump, "");
SYMBOL_FLAGS(mtk_adsp_dump, 0x00);
KSYMTAB_FUNC(mtk_adsp_send_msg, "");
SYMBOL_FLAGS(mtk_adsp_send_msg, 0x00);
KSYMTAB_FUNC(mtk_adsp_handle_reply, "");
SYMBOL_FLAGS(mtk_adsp_handle_reply, 0x00);
KSYMTAB_FUNC(mtk_adsp_handle_request, "");
SYMBOL_FLAGS(mtk_adsp_handle_request, 0x00);
KSYMTAB_FUNC(mtk_adsp_get_bar_index, "");
SYMBOL_FLAGS(mtk_adsp_get_bar_index, 0x00);
KSYMTAB_FUNC(mtk_adsp_stream_pcm_hw_params, "");
SYMBOL_FLAGS(mtk_adsp_stream_pcm_hw_params, 0x00);
KSYMTAB_FUNC(mtk_adsp_stream_pcm_pointer, "");
SYMBOL_FLAGS(mtk_adsp_stream_pcm_pointer, 0x00);

MODULE_INFO(depends, "snd-sof,mtk-adsp-ipc");

