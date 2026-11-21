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

KSYMTAB_FUNC(zstd_min_clevel, "");
SYMBOL_FLAGS(zstd_min_clevel, 0x00);
KSYMTAB_FUNC(zstd_max_clevel, "");
SYMBOL_FLAGS(zstd_max_clevel, 0x00);
KSYMTAB_FUNC(zstd_default_clevel, "");
SYMBOL_FLAGS(zstd_default_clevel, 0x00);
KSYMTAB_FUNC(zstd_compress_bound, "");
SYMBOL_FLAGS(zstd_compress_bound, 0x00);
KSYMTAB_FUNC(zstd_get_params, "");
SYMBOL_FLAGS(zstd_get_params, 0x00);
KSYMTAB_FUNC(zstd_get_cparams, "");
SYMBOL_FLAGS(zstd_get_cparams, 0x00);
KSYMTAB_FUNC(zstd_cctx_set_param, "");
SYMBOL_FLAGS(zstd_cctx_set_param, 0x00);
KSYMTAB_FUNC(zstd_cctx_workspace_bound, "");
SYMBOL_FLAGS(zstd_cctx_workspace_bound, 0x00);
KSYMTAB_FUNC(zstd_cctx_workspace_bound_with_ext_seq_prod, "");
SYMBOL_FLAGS(zstd_cctx_workspace_bound_with_ext_seq_prod, 0x00);
KSYMTAB_FUNC(zstd_cstream_workspace_bound_with_ext_seq_prod, "");
SYMBOL_FLAGS(zstd_cstream_workspace_bound_with_ext_seq_prod, 0x00);
KSYMTAB_FUNC(zstd_init_cctx, "");
SYMBOL_FLAGS(zstd_init_cctx, 0x00);
KSYMTAB_FUNC(zstd_create_cctx_advanced, "");
SYMBOL_FLAGS(zstd_create_cctx_advanced, 0x00);
KSYMTAB_FUNC(zstd_free_cctx, "");
SYMBOL_FLAGS(zstd_free_cctx, 0x00);
KSYMTAB_FUNC(zstd_create_cdict_byreference, "");
SYMBOL_FLAGS(zstd_create_cdict_byreference, 0x00);
KSYMTAB_FUNC(zstd_free_cdict, "");
SYMBOL_FLAGS(zstd_free_cdict, 0x00);
KSYMTAB_FUNC(zstd_compress_cctx, "");
SYMBOL_FLAGS(zstd_compress_cctx, 0x00);
KSYMTAB_FUNC(zstd_compress_using_cdict, "");
SYMBOL_FLAGS(zstd_compress_using_cdict, 0x00);
KSYMTAB_FUNC(zstd_cstream_workspace_bound, "");
SYMBOL_FLAGS(zstd_cstream_workspace_bound, 0x00);
KSYMTAB_FUNC(zstd_init_cstream, "");
SYMBOL_FLAGS(zstd_init_cstream, 0x00);
KSYMTAB_FUNC(zstd_reset_cstream, "");
SYMBOL_FLAGS(zstd_reset_cstream, 0x00);
KSYMTAB_FUNC(zstd_compress_stream, "");
SYMBOL_FLAGS(zstd_compress_stream, 0x00);
KSYMTAB_FUNC(zstd_flush_stream, "");
SYMBOL_FLAGS(zstd_flush_stream, 0x00);
KSYMTAB_FUNC(zstd_end_stream, "");
SYMBOL_FLAGS(zstd_end_stream, 0x00);
KSYMTAB_FUNC(zstd_register_sequence_producer, "");
SYMBOL_FLAGS(zstd_register_sequence_producer, 0x00);
KSYMTAB_FUNC(zstd_compress_sequences_and_literals, "");
SYMBOL_FLAGS(zstd_compress_sequences_and_literals, 0x00);

MODULE_INFO(depends, "");

