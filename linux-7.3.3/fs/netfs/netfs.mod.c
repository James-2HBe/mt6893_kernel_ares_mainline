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

KSYMTAB_FUNC(netfs_readahead, "");
SYMBOL_FLAGS(netfs_readahead, 0x00);
KSYMTAB_FUNC(netfs_read_folio, "");
SYMBOL_FLAGS(netfs_read_folio, 0x00);
KSYMTAB_FUNC(netfs_write_begin, "");
SYMBOL_FLAGS(netfs_write_begin, 0x00);
KSYMTAB_FUNC(netfs_buffered_read_iter, "");
SYMBOL_FLAGS(netfs_buffered_read_iter, 0x00);
KSYMTAB_FUNC(netfs_file_read_iter, "");
SYMBOL_FLAGS(netfs_file_read_iter, 0x00);
KSYMTAB_FUNC(netfs_perform_write, "");
SYMBOL_FLAGS(netfs_perform_write, 0x00);
KSYMTAB_FUNC(netfs_buffered_write_iter_locked, "");
SYMBOL_FLAGS(netfs_buffered_write_iter_locked, 0x00);
KSYMTAB_FUNC(netfs_file_write_iter, "");
SYMBOL_FLAGS(netfs_file_write_iter, 0x00);
KSYMTAB_FUNC(netfs_page_mkwrite, "");
SYMBOL_FLAGS(netfs_page_mkwrite, 0x00);
KSYMTAB_FUNC(netfs_unbuffered_read_iter_locked, "");
SYMBOL_FLAGS(netfs_unbuffered_read_iter_locked, 0x00);
KSYMTAB_FUNC(netfs_unbuffered_read_iter, "");
SYMBOL_FLAGS(netfs_unbuffered_read_iter, 0x00);
KSYMTAB_FUNC(netfs_unbuffered_write_iter_locked, "");
SYMBOL_FLAGS(netfs_unbuffered_write_iter_locked, 0x00);
KSYMTAB_FUNC(netfs_unbuffered_write_iter, "");
SYMBOL_FLAGS(netfs_unbuffered_write_iter, 0x00);
KSYMTAB_FUNC(netfs_extract_user_iter, "");
SYMBOL_FLAGS(netfs_extract_user_iter, 0x01);
KSYMTAB_FUNC(netfs_limit_iter, "");
SYMBOL_FLAGS(netfs_limit_iter, 0x00);
KSYMTAB_FUNC(netfs_start_io_read, "");
SYMBOL_FLAGS(netfs_start_io_read, 0x00);
KSYMTAB_FUNC(netfs_end_io_read, "");
SYMBOL_FLAGS(netfs_end_io_read, 0x00);
KSYMTAB_FUNC(netfs_start_io_write, "");
SYMBOL_FLAGS(netfs_start_io_write, 0x00);
KSYMTAB_FUNC(netfs_end_io_write, "");
SYMBOL_FLAGS(netfs_end_io_write, 0x00);
KSYMTAB_FUNC(netfs_start_io_direct, "");
SYMBOL_FLAGS(netfs_start_io_direct, 0x00);
KSYMTAB_FUNC(netfs_end_io_direct, "");
SYMBOL_FLAGS(netfs_end_io_direct, 0x00);
KSYMTAB_FUNC(netfs_wb_begin, "");
SYMBOL_FLAGS(netfs_wb_begin, 0x00);
KSYMTAB_FUNC(netfs_wb_end, "");
SYMBOL_FLAGS(netfs_wb_end, 0x00);
KSYMTAB_FUNC(netfs_alloc_folioq_buffer, "");
SYMBOL_FLAGS(netfs_alloc_folioq_buffer, 0x00);
KSYMTAB_FUNC(netfs_free_folioq_buffer, "");
SYMBOL_FLAGS(netfs_free_folioq_buffer, 0x00);
KSYMTAB_FUNC(netfs_dirty_folio, "");
SYMBOL_FLAGS(netfs_dirty_folio, 0x00);
KSYMTAB_FUNC(netfs_unpin_writeback, "");
SYMBOL_FLAGS(netfs_unpin_writeback, 0x00);
KSYMTAB_FUNC(netfs_clear_inode_writeback, "");
SYMBOL_FLAGS(netfs_clear_inode_writeback, 0x00);
KSYMTAB_FUNC(netfs_invalidate_folio, "");
SYMBOL_FLAGS(netfs_invalidate_folio, 0x00);
KSYMTAB_FUNC(netfs_release_folio, "");
SYMBOL_FLAGS(netfs_release_folio, 0x00);
KSYMTAB_FUNC(netfs_clear_stale_post_isize, "");
SYMBOL_FLAGS(netfs_clear_stale_post_isize, 0x00);
KSYMTAB_FUNC(netfs_read_subreq_progress, "");
SYMBOL_FLAGS(netfs_read_subreq_progress, 0x00);
KSYMTAB_FUNC(netfs_read_subreq_terminated, "");
SYMBOL_FLAGS(netfs_read_subreq_terminated, 0x00);
KSYMTAB_FUNC(netfs_single_mark_inode_dirty, "");
SYMBOL_FLAGS(netfs_single_mark_inode_dirty, 0x00);
KSYMTAB_FUNC(netfs_read_single, "");
SYMBOL_FLAGS(netfs_read_single, 0x00);
KSYMTAB_FUNC(netfs_folioq_alloc, "");
SYMBOL_FLAGS(netfs_folioq_alloc, 0x00);
KSYMTAB_FUNC(netfs_folioq_free, "");
SYMBOL_FLAGS(netfs_folioq_free, 0x00);
KSYMTAB_FUNC(netfs_write_subrequest_terminated, "");
SYMBOL_FLAGS(netfs_write_subrequest_terminated, 0x00);
KSYMTAB_FUNC(netfs_prepare_write_failed, "");
SYMBOL_FLAGS(netfs_prepare_write_failed, 0x00);
KSYMTAB_FUNC(netfs_writepages, "");
SYMBOL_FLAGS(netfs_writepages, 0x00);
KSYMTAB_FUNC(netfs_writeback_single, "");
SYMBOL_FLAGS(netfs_writeback_single, 0x00);

MODULE_INFO(depends, "");

