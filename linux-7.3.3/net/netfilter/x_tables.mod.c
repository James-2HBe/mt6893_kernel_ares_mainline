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

KSYMTAB_FUNC(xt_register_target, "");
SYMBOL_FLAGS(xt_register_target, 0x00);
KSYMTAB_FUNC(xt_unregister_target, "");
SYMBOL_FLAGS(xt_unregister_target, 0x00);
KSYMTAB_FUNC(xt_register_targets, "");
SYMBOL_FLAGS(xt_register_targets, 0x00);
KSYMTAB_FUNC(xt_unregister_targets, "");
SYMBOL_FLAGS(xt_unregister_targets, 0x00);
KSYMTAB_FUNC(xt_register_match, "");
SYMBOL_FLAGS(xt_register_match, 0x00);
KSYMTAB_FUNC(xt_unregister_match, "");
SYMBOL_FLAGS(xt_unregister_match, 0x00);
KSYMTAB_FUNC(xt_register_matches, "");
SYMBOL_FLAGS(xt_register_matches, 0x00);
KSYMTAB_FUNC(xt_unregister_matches, "");
SYMBOL_FLAGS(xt_unregister_matches, 0x00);
KSYMTAB_FUNC(xt_find_match, "");
SYMBOL_FLAGS(xt_find_match, 0x00);
KSYMTAB_FUNC(xt_request_find_match, "");
SYMBOL_FLAGS(xt_request_find_match, 0x01);
KSYMTAB_FUNC(xt_request_find_target, "");
SYMBOL_FLAGS(xt_request_find_target, 0x01);
KSYMTAB_FUNC(xt_data_to_user, "");
SYMBOL_FLAGS(xt_data_to_user, 0x01);
KSYMTAB_FUNC(xt_match_to_user, "");
SYMBOL_FLAGS(xt_match_to_user, 0x01);
KSYMTAB_FUNC(xt_target_to_user, "");
SYMBOL_FLAGS(xt_target_to_user, 0x01);
KSYMTAB_FUNC(xt_find_revision, "");
SYMBOL_FLAGS(xt_find_revision, 0x01);
KSYMTAB_FUNC(xt_check_proc_name, "");
SYMBOL_FLAGS(xt_check_proc_name, 0x00);
KSYMTAB_FUNC(xt_check_hooks_match, "");
SYMBOL_FLAGS(xt_check_hooks_match, 0x01);
KSYMTAB_FUNC(xt_check_match, "");
SYMBOL_FLAGS(xt_check_match, 0x01);
KSYMTAB_FUNC(xt_check_table_hooks, "");
SYMBOL_FLAGS(xt_check_table_hooks, 0x00);
KSYMTAB_FUNC(xt_check_entry_offsets, "");
SYMBOL_FLAGS(xt_check_entry_offsets, 0x00);
KSYMTAB_FUNC(xt_alloc_entry_offsets, "");
SYMBOL_FLAGS(xt_alloc_entry_offsets, 0x00);
KSYMTAB_FUNC(xt_find_jump_offset, "");
SYMBOL_FLAGS(xt_find_jump_offset, 0x00);
KSYMTAB_FUNC(xt_check_hooks_target, "");
SYMBOL_FLAGS(xt_check_hooks_target, 0x01);
KSYMTAB_FUNC(xt_check_target, "");
SYMBOL_FLAGS(xt_check_target, 0x01);
KSYMTAB_FUNC(xt_copy_counters, "");
SYMBOL_FLAGS(xt_copy_counters, 0x01);
KSYMTAB_FUNC(xt_alloc_table_info, "");
SYMBOL_FLAGS(xt_alloc_table_info, 0x00);
KSYMTAB_FUNC(xt_free_table_info, "");
SYMBOL_FLAGS(xt_free_table_info, 0x00);
KSYMTAB_FUNC(xt_find_table, "");
SYMBOL_FLAGS(xt_find_table, 0x00);
KSYMTAB_FUNC(xt_find_table_lock, "");
SYMBOL_FLAGS(xt_find_table_lock, 0x01);
KSYMTAB_FUNC(xt_request_find_table_lock, "");
SYMBOL_FLAGS(xt_request_find_table_lock, 0x01);
KSYMTAB_FUNC(xt_table_unlock, "");
SYMBOL_FLAGS(xt_table_unlock, 0x01);
KSYMTAB_DATA(xt_tee_enabled, "");
SYMBOL_FLAGS(xt_tee_enabled, 0x01);
KSYMTAB_FUNC(xt_hook_ops_alloc, "");
SYMBOL_FLAGS(xt_hook_ops_alloc, 0x01);
KSYMTAB_FUNC(xt_register_template, "");
SYMBOL_FLAGS(xt_register_template, 0x01);
KSYMTAB_FUNC(xt_unregister_template, "");
SYMBOL_FLAGS(xt_unregister_template, 0x01);
KSYMTAB_FUNC(xt_proto_init, "");
SYMBOL_FLAGS(xt_proto_init, 0x01);
KSYMTAB_FUNC(xt_proto_fini, "");
SYMBOL_FLAGS(xt_proto_fini, 0x01);

MODULE_INFO(depends, "");

