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

KSYMTAB_FUNC(q6asm_unmap_memory_regions, "");
SYMBOL_FLAGS(q6asm_unmap_memory_regions, 0x01);
KSYMTAB_FUNC(q6asm_map_memory_regions, "");
SYMBOL_FLAGS(q6asm_map_memory_regions, 0x01);
KSYMTAB_FUNC(q6asm_audio_client_free, "");
SYMBOL_FLAGS(q6asm_audio_client_free, 0x01);
KSYMTAB_FUNC(q6asm_get_hw_pointer, "");
SYMBOL_FLAGS(q6asm_get_hw_pointer, 0x01);
KSYMTAB_FUNC(q6asm_get_session_id, "");
SYMBOL_FLAGS(q6asm_get_session_id, 0x01);
KSYMTAB_FUNC(q6asm_audio_client_alloc, "");
SYMBOL_FLAGS(q6asm_audio_client_alloc, 0x01);
KSYMTAB_FUNC(q6asm_open_write, "");
SYMBOL_FLAGS(q6asm_open_write, 0x01);
KSYMTAB_FUNC(q6asm_run, "");
SYMBOL_FLAGS(q6asm_run, 0x01);
KSYMTAB_FUNC(q6asm_run_nowait, "");
SYMBOL_FLAGS(q6asm_run_nowait, 0x01);
KSYMTAB_FUNC(q6asm_media_format_block_multi_ch_pcm, "");
SYMBOL_FLAGS(q6asm_media_format_block_multi_ch_pcm, 0x01);
KSYMTAB_FUNC(q6asm_stream_media_format_block_flac, "");
SYMBOL_FLAGS(q6asm_stream_media_format_block_flac, 0x01);
KSYMTAB_FUNC(q6asm_stream_media_format_block_wma_v9, "");
SYMBOL_FLAGS(q6asm_stream_media_format_block_wma_v9, 0x01);
KSYMTAB_FUNC(q6asm_stream_media_format_block_wma_v10, "");
SYMBOL_FLAGS(q6asm_stream_media_format_block_wma_v10, 0x01);
KSYMTAB_FUNC(q6asm_stream_media_format_block_alac, "");
SYMBOL_FLAGS(q6asm_stream_media_format_block_alac, 0x01);
KSYMTAB_FUNC(q6asm_stream_media_format_block_ape, "");
SYMBOL_FLAGS(q6asm_stream_media_format_block_ape, 0x01);
KSYMTAB_FUNC(q6asm_stream_remove_initial_silence, "");
SYMBOL_FLAGS(q6asm_stream_remove_initial_silence, 0x01);
KSYMTAB_FUNC(q6asm_stream_remove_trailing_silence, "");
SYMBOL_FLAGS(q6asm_stream_remove_trailing_silence, 0x01);
KSYMTAB_FUNC(q6asm_enc_cfg_blk_pcm_format_support, "");
SYMBOL_FLAGS(q6asm_enc_cfg_blk_pcm_format_support, 0x01);
KSYMTAB_FUNC(q6asm_read, "");
SYMBOL_FLAGS(q6asm_read, 0x01);
KSYMTAB_FUNC(q6asm_open_read, "");
SYMBOL_FLAGS(q6asm_open_read, 0x01);
KSYMTAB_FUNC(q6asm_write_async, "");
SYMBOL_FLAGS(q6asm_write_async, 0x01);
KSYMTAB_FUNC(q6asm_cmd, "");
SYMBOL_FLAGS(q6asm_cmd, 0x01);
KSYMTAB_FUNC(q6asm_cmd_nowait, "");
SYMBOL_FLAGS(q6asm_cmd_nowait, 0x01);

MODULE_INFO(depends, "apr,snd-q6dsp-common,q6core");

MODULE_ALIAS("of:N*T*Cqcom,q6asm");
MODULE_ALIAS("of:N*T*Cqcom,q6asmC*");
