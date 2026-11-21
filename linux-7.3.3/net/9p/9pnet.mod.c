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

KSYMTAB_FUNC(v9fs_register_trans, "");
SYMBOL_FLAGS(v9fs_register_trans, 0x00);
KSYMTAB_FUNC(v9fs_unregister_trans, "");
SYMBOL_FLAGS(v9fs_unregister_trans, 0x00);
KSYMTAB_FUNC(v9fs_get_trans_by_name, "");
SYMBOL_FLAGS(v9fs_get_trans_by_name, 0x00);
KSYMTAB_FUNC(v9fs_get_default_trans, "");
SYMBOL_FLAGS(v9fs_get_default_trans, 0x00);
KSYMTAB_FUNC(v9fs_put_trans, "");
SYMBOL_FLAGS(v9fs_put_trans, 0x00);
KSYMTAB_FUNC(p9_is_proto_dotl, "");
SYMBOL_FLAGS(p9_is_proto_dotl, 0x00);
KSYMTAB_FUNC(p9_is_proto_dotu, "");
SYMBOL_FLAGS(p9_is_proto_dotu, 0x00);
KSYMTAB_FUNC(p9_show_client_options, "");
SYMBOL_FLAGS(p9_show_client_options, 0x00);
KSYMTAB_FUNC(p9_fcall_fini, "");
SYMBOL_FLAGS(p9_fcall_fini, 0x00);
KSYMTAB_FUNC(p9_tag_lookup, "");
SYMBOL_FLAGS(p9_tag_lookup, 0x00);
KSYMTAB_FUNC(p9_req_put, "");
SYMBOL_FLAGS(p9_req_put, 0x00);
KSYMTAB_FUNC(p9_client_cb, "");
SYMBOL_FLAGS(p9_client_cb, 0x00);
KSYMTAB_FUNC(p9_parse_header, "");
SYMBOL_FLAGS(p9_parse_header, 0x00);
KSYMTAB_FUNC(do_trace_9p_fid_get, "");
SYMBOL_FLAGS(do_trace_9p_fid_get, 0x00);
KSYMTAB_FUNC(do_trace_9p_fid_put, "");
SYMBOL_FLAGS(do_trace_9p_fid_put, 0x00);
KSYMTAB_FUNC(p9_client_create, "");
SYMBOL_FLAGS(p9_client_create, 0x00);
KSYMTAB_FUNC(p9_client_destroy, "");
SYMBOL_FLAGS(p9_client_destroy, 0x00);
KSYMTAB_FUNC(p9_client_disconnect, "");
SYMBOL_FLAGS(p9_client_disconnect, 0x00);
KSYMTAB_FUNC(p9_client_begin_disconnect, "");
SYMBOL_FLAGS(p9_client_begin_disconnect, 0x00);
KSYMTAB_FUNC(p9_client_attach, "");
SYMBOL_FLAGS(p9_client_attach, 0x00);
KSYMTAB_FUNC(p9_client_walk, "");
SYMBOL_FLAGS(p9_client_walk, 0x00);
KSYMTAB_FUNC(p9_client_open, "");
SYMBOL_FLAGS(p9_client_open, 0x00);
KSYMTAB_FUNC(p9_client_create_dotl, "");
SYMBOL_FLAGS(p9_client_create_dotl, 0x00);
KSYMTAB_FUNC(p9_client_fcreate, "");
SYMBOL_FLAGS(p9_client_fcreate, 0x00);
KSYMTAB_FUNC(p9_client_symlink, "");
SYMBOL_FLAGS(p9_client_symlink, 0x00);
KSYMTAB_FUNC(p9_client_link, "");
SYMBOL_FLAGS(p9_client_link, 0x00);
KSYMTAB_FUNC(p9_client_fsync, "");
SYMBOL_FLAGS(p9_client_fsync, 0x00);
KSYMTAB_FUNC(p9_client_clunk, "");
SYMBOL_FLAGS(p9_client_clunk, 0x00);
KSYMTAB_FUNC(p9_client_remove, "");
SYMBOL_FLAGS(p9_client_remove, 0x00);
KSYMTAB_FUNC(p9_client_unlinkat, "");
SYMBOL_FLAGS(p9_client_unlinkat, 0x00);
KSYMTAB_FUNC(p9_client_read, "");
SYMBOL_FLAGS(p9_client_read, 0x00);
KSYMTAB_FUNC(p9_client_read_once, "");
SYMBOL_FLAGS(p9_client_read_once, 0x00);
KSYMTAB_FUNC(p9_client_write, "");
SYMBOL_FLAGS(p9_client_write, 0x00);
KSYMTAB_FUNC(p9_client_write_subreq, "");
SYMBOL_FLAGS(p9_client_write_subreq, 0x00);
KSYMTAB_FUNC(p9_client_stat, "");
SYMBOL_FLAGS(p9_client_stat, 0x00);
KSYMTAB_FUNC(p9_client_getattr_dotl, "");
SYMBOL_FLAGS(p9_client_getattr_dotl, 0x00);
KSYMTAB_FUNC(p9_client_wstat, "");
SYMBOL_FLAGS(p9_client_wstat, 0x00);
KSYMTAB_FUNC(p9_client_setattr, "");
SYMBOL_FLAGS(p9_client_setattr, 0x00);
KSYMTAB_FUNC(p9_client_statfs, "");
SYMBOL_FLAGS(p9_client_statfs, 0x00);
KSYMTAB_FUNC(p9_client_rename, "");
SYMBOL_FLAGS(p9_client_rename, 0x00);
KSYMTAB_FUNC(p9_client_renameat, "");
SYMBOL_FLAGS(p9_client_renameat, 0x00);
KSYMTAB_FUNC(p9_client_xattrwalk, "");
SYMBOL_FLAGS(p9_client_xattrwalk, 0x01);
KSYMTAB_FUNC(p9_client_xattrcreate, "");
SYMBOL_FLAGS(p9_client_xattrcreate, 0x01);
KSYMTAB_FUNC(p9_client_readdir, "");
SYMBOL_FLAGS(p9_client_readdir, 0x00);
KSYMTAB_FUNC(p9_client_mknod_dotl, "");
SYMBOL_FLAGS(p9_client_mknod_dotl, 0x00);
KSYMTAB_FUNC(p9_client_mkdir_dotl, "");
SYMBOL_FLAGS(p9_client_mkdir_dotl, 0x00);
KSYMTAB_FUNC(p9_client_lock_dotl, "");
SYMBOL_FLAGS(p9_client_lock_dotl, 0x00);
KSYMTAB_FUNC(p9_client_getlock_dotl, "");
SYMBOL_FLAGS(p9_client_getlock_dotl, 0x00);
KSYMTAB_FUNC(p9_client_readlink, "");
SYMBOL_FLAGS(p9_client_readlink, 0x00);
KSYMTAB_FUNC(p9_error_init, "");
SYMBOL_FLAGS(p9_error_init, 0x00);
KSYMTAB_FUNC(p9_errstr2errno, "");
SYMBOL_FLAGS(p9_errstr2errno, 0x00);
KSYMTAB_FUNC(p9stat_free, "");
SYMBOL_FLAGS(p9stat_free, 0x00);
KSYMTAB_FUNC(p9stat_read, "");
SYMBOL_FLAGS(p9stat_read, 0x00);
KSYMTAB_FUNC(p9dirent_read, "");
SYMBOL_FLAGS(p9dirent_read, 0x00);
KSYMTAB_FUNC(p9_release_pages, "");
SYMBOL_FLAGS(p9_release_pages, 0x00);

MODULE_INFO(depends, "netfs");

