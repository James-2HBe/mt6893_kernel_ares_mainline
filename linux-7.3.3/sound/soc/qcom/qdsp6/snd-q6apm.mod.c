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

KSYMTAB_FUNC(q6apm_is_adsp_ready, "");
SYMBOL_FLAGS(q6apm_is_adsp_ready, 0x01);
KSYMTAB_FUNC(q6apm_graph_media_format_shmem, "");
SYMBOL_FLAGS(q6apm_graph_media_format_shmem, 0x01);
KSYMTAB_FUNC(q6apm_map_pos_buffer, "");
SYMBOL_FLAGS(q6apm_map_pos_buffer, 0x01);
KSYMTAB_FUNC(q6apm_map_memory_fixed_region, "");
SYMBOL_FLAGS(q6apm_map_memory_fixed_region, 0x01);
KSYMTAB_FUNC(q6apm_alloc_fragments, "");
SYMBOL_FLAGS(q6apm_alloc_fragments, 0x01);
KSYMTAB_FUNC(q6apm_unmap_memory_fixed_region, "");
SYMBOL_FLAGS(q6apm_unmap_memory_fixed_region, 0x01);
KSYMTAB_FUNC(q6apm_unmap_pos_buffer, "");
SYMBOL_FLAGS(q6apm_unmap_pos_buffer, 0x01);
KSYMTAB_FUNC(q6apm_free_fragments, "");
SYMBOL_FLAGS(q6apm_free_fragments, 0x01);
KSYMTAB_FUNC(q6apm_remove_initial_silence, "");
SYMBOL_FLAGS(q6apm_remove_initial_silence, 0x01);
KSYMTAB_FUNC(q6apm_remove_trailing_silence, "");
SYMBOL_FLAGS(q6apm_remove_trailing_silence, 0x01);
KSYMTAB_FUNC(q6apm_enable_compress_module, "");
SYMBOL_FLAGS(q6apm_enable_compress_module, 0x01);
KSYMTAB_FUNC(q6apm_set_real_module_id, "");
SYMBOL_FLAGS(q6apm_set_real_module_id, 0x01);
KSYMTAB_FUNC(q6apm_graph_media_format_pcm, "");
SYMBOL_FLAGS(q6apm_graph_media_format_pcm, 0x01);
KSYMTAB_FUNC(q6apm_write_async, "");
SYMBOL_FLAGS(q6apm_write_async, 0x01);
KSYMTAB_FUNC(q6apm_read, "");
SYMBOL_FLAGS(q6apm_read, 0x01);
KSYMTAB_FUNC(q6apm_get_hw_pointer, "");
SYMBOL_FLAGS(q6apm_get_hw_pointer, 0x01);
KSYMTAB_FUNC(q6apm_register_watermark_event, "");
SYMBOL_FLAGS(q6apm_register_watermark_event, 0x01);
KSYMTAB_FUNC(q6apm_push_pull_config, "");
SYMBOL_FLAGS(q6apm_push_pull_config, 0x01);
KSYMTAB_FUNC(q6apm_is_graph_in_push_pull_mode_from_id, "");
SYMBOL_FLAGS(q6apm_is_graph_in_push_pull_mode_from_id, 0x01);
KSYMTAB_FUNC(q6apm_is_graph_in_push_pull_mode, "");
SYMBOL_FLAGS(q6apm_is_graph_in_push_pull_mode, 0x01);
KSYMTAB_FUNC(q6apm_graph_open, "");
SYMBOL_FLAGS(q6apm_graph_open, 0x01);
KSYMTAB_FUNC(q6apm_graph_close, "");
SYMBOL_FLAGS(q6apm_graph_close, 0x01);
KSYMTAB_FUNC(q6apm_graph_prepare, "");
SYMBOL_FLAGS(q6apm_graph_prepare, 0x01);
KSYMTAB_FUNC(q6apm_graph_start, "");
SYMBOL_FLAGS(q6apm_graph_start, 0x01);
KSYMTAB_FUNC(q6apm_graph_stop, "");
SYMBOL_FLAGS(q6apm_graph_stop, 0x01);
KSYMTAB_FUNC(q6apm_graph_flush, "");
SYMBOL_FLAGS(q6apm_graph_flush, 0x01);
KSYMTAB_FUNC(audioreach_alloc_pkt, "");
SYMBOL_FLAGS(audioreach_alloc_pkt, 0x01);
KSYMTAB_FUNC(audioreach_alloc_apm_pkt, "");
SYMBOL_FLAGS(audioreach_alloc_apm_pkt, 0x01);
KSYMTAB_FUNC(audioreach_alloc_cmd_pkt, "");
SYMBOL_FLAGS(audioreach_alloc_cmd_pkt, 0x01);
KSYMTAB_FUNC(audioreach_alloc_apm_cmd_pkt, "");
SYMBOL_FLAGS(audioreach_alloc_apm_cmd_pkt, 0x01);
KSYMTAB_FUNC(audioreach_set_default_channel_mapping, "");
SYMBOL_FLAGS(audioreach_set_default_channel_mapping, 0x01);
KSYMTAB_FUNC(audioreach_alloc_graph_pkt, "");
SYMBOL_FLAGS(audioreach_alloc_graph_pkt, 0x01);
KSYMTAB_FUNC(audioreach_send_cmd_sync, "");
SYMBOL_FLAGS(audioreach_send_cmd_sync, 0x01);
KSYMTAB_FUNC(audioreach_graph_send_cmd_sync, "");
SYMBOL_FLAGS(audioreach_graph_send_cmd_sync, 0x01);
KSYMTAB_FUNC(audioreach_send_u32_param, "");
SYMBOL_FLAGS(audioreach_send_u32_param, 0x01);
KSYMTAB_FUNC(audioreach_compr_set_param, "");
SYMBOL_FLAGS(audioreach_compr_set_param, 0x01);
KSYMTAB_FUNC(audioreach_shmem_register_event, "");
SYMBOL_FLAGS(audioreach_shmem_register_event, 0x01);
KSYMTAB_FUNC(audioreach_gain_set_vol_ctrl, "");
SYMBOL_FLAGS(audioreach_gain_set_vol_ctrl, 0x01);
KSYMTAB_FUNC(audioreach_set_media_format, "");
SYMBOL_FLAGS(audioreach_set_media_format, 0x01);
KSYMTAB_FUNC(audioreach_graph_free_buf, "");
SYMBOL_FLAGS(audioreach_graph_free_buf, 0x01);
KSYMTAB_FUNC(audioreach_setup_push_pull, "");
SYMBOL_FLAGS(audioreach_setup_push_pull, 0x01);
KSYMTAB_FUNC(audioreach_shared_memory_send_eos, "");
SYMBOL_FLAGS(audioreach_shared_memory_send_eos, 0x01);
KSYMTAB_FUNC(audioreach_tplg_init, "");
SYMBOL_FLAGS(audioreach_tplg_init, 0x01);

MODULE_INFO(depends, "apr,snd-soc-core");

MODULE_ALIAS("of:N*T*Cqcom,q6apm");
MODULE_ALIAS("of:N*T*Cqcom,q6apmC*");
