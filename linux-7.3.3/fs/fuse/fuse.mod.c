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

KSYMTAB_FUNC(fuse_len_args, "");
SYMBOL_FLAGS(fuse_len_args, 0x01);
KSYMTAB_FUNC(fuse_get_unique, "");
SYMBOL_FLAGS(fuse_get_unique, 0x01);
KSYMTAB_FUNC(fuse_req_hash, "");
SYMBOL_FLAGS(fuse_req_hash, 0x01);
KSYMTAB_FUNC(fuse_request_assign_unique, "");
SYMBOL_FLAGS(fuse_request_assign_unique, 0x01);
KSYMTAB_FUNC(fuse_iqueue_init, "");
SYMBOL_FLAGS(fuse_iqueue_init, 0x01);
KSYMTAB_FUNC(fuse_chan_free, "");
SYMBOL_FLAGS(fuse_chan_free, 0x01);
KSYMTAB_FUNC(fuse_chan_new, "");
SYMBOL_FLAGS(fuse_chan_new, 0x01);
KSYMTAB_FUNC(fuse_dev_chan_new, "");
SYMBOL_FLAGS(fuse_dev_chan_new, 0x01);
KSYMTAB_FUNC(fuse_dev_alloc, "");
SYMBOL_FLAGS(fuse_dev_alloc, 0x01);
KSYMTAB_FUNC(fuse_dev_install, "");
SYMBOL_FLAGS(fuse_dev_install, 0x01);
KSYMTAB_FUNC(fuse_dev_alloc_install, "");
SYMBOL_FLAGS(fuse_dev_alloc_install, 0x01);
KSYMTAB_FUNC(fuse_dev_put, "");
SYMBOL_FLAGS(fuse_dev_put, 0x01);
KSYMTAB_FUNC(fuse_request_end, "");
SYMBOL_FLAGS(fuse_request_end, 0x01);
KSYMTAB_FUNC(fuse_chan_abort, "");
SYMBOL_FLAGS(fuse_chan_abort, 0x01);
KSYMTAB_FUNC(fuse_dev_release, "");
SYMBOL_FLAGS(fuse_dev_release, 0x01);
KSYMTAB_DATA(fuse_dev_operations, "");
SYMBOL_FLAGS(fuse_dev_operations, 0x01);
KSYMTAB_FUNC(fuse_do_open, "");
SYMBOL_FLAGS(fuse_do_open, 0x01);
KSYMTAB_FUNC(fuse_sync_release, "");
SYMBOL_FLAGS(fuse_sync_release, 0x01);
KSYMTAB_FUNC(fuse_direct_io, "");
SYMBOL_FLAGS(fuse_direct_io, 0x01);
KSYMTAB_FUNC(fuse_conn_init, "");
SYMBOL_FLAGS(fuse_conn_init, 0x01);
KSYMTAB_FUNC(fuse_conn_put, "");
SYMBOL_FLAGS(fuse_conn_put, 0x01);
KSYMTAB_FUNC(fuse_conn_get, "");
SYMBOL_FLAGS(fuse_conn_get, 0x01);
KSYMTAB_FUNC(fuse_send_init, "");
SYMBOL_FLAGS(fuse_send_init, 0x01);
KSYMTAB_FUNC(fuse_free_conn, "");
SYMBOL_FLAGS(fuse_free_conn, 0x01);
KSYMTAB_FUNC(fuse_init_fs_context_submount, "");
SYMBOL_FLAGS(fuse_init_fs_context_submount, 0x01);
KSYMTAB_FUNC(fuse_fill_super_common, "");
SYMBOL_FLAGS(fuse_fill_super_common, 0x01);
KSYMTAB_FUNC(fuse_mount_remove, "");
SYMBOL_FLAGS(fuse_mount_remove, 0x01);
KSYMTAB_FUNC(fuse_conn_destroy, "");
SYMBOL_FLAGS(fuse_conn_destroy, 0x01);
KSYMTAB_FUNC(fuse_mount_destroy, "");
SYMBOL_FLAGS(fuse_mount_destroy, 0x00);
KSYMTAB_FUNC(fuse_do_ioctl, "");
SYMBOL_FLAGS(fuse_do_ioctl, 0x01);
KSYMTAB_FUNC(fuse_simple_background, "");
SYMBOL_FLAGS(fuse_simple_background, 0x01);
KSYMTAB_FUNC(fuse_file_poll, "");
SYMBOL_FLAGS(fuse_file_poll, 0x01);

MODULE_INFO(depends, "");

