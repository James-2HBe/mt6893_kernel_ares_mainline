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

KSYMTAB_FUNC(af_alg_register_type, "");
SYMBOL_FLAGS(af_alg_register_type, 0x01);
KSYMTAB_FUNC(af_alg_unregister_type, "");
SYMBOL_FLAGS(af_alg_unregister_type, 0x01);
KSYMTAB_FUNC(af_alg_check_restriction, "");
SYMBOL_FLAGS(af_alg_check_restriction, 0x01);
KSYMTAB_FUNC(af_alg_release, "");
SYMBOL_FLAGS(af_alg_release, 0x01);
KSYMTAB_FUNC(af_alg_release_parent, "");
SYMBOL_FLAGS(af_alg_release_parent, 0x01);
KSYMTAB_FUNC(af_alg_accept, "");
SYMBOL_FLAGS(af_alg_accept, 0x01);
KSYMTAB_FUNC(af_alg_free_sg, "");
SYMBOL_FLAGS(af_alg_free_sg, 0x01);
KSYMTAB_FUNC(af_alg_count_tsgl, "");
SYMBOL_FLAGS(af_alg_count_tsgl, 0x01);
KSYMTAB_FUNC(af_alg_pull_tsgl, "");
SYMBOL_FLAGS(af_alg_pull_tsgl, 0x01);
KSYMTAB_FUNC(af_alg_wmem_wakeup, "");
SYMBOL_FLAGS(af_alg_wmem_wakeup, 0x01);
KSYMTAB_FUNC(af_alg_wait_for_data, "");
SYMBOL_FLAGS(af_alg_wait_for_data, 0x01);
KSYMTAB_FUNC(af_alg_sendmsg, "");
SYMBOL_FLAGS(af_alg_sendmsg, 0x01);
KSYMTAB_FUNC(af_alg_free_resources, "");
SYMBOL_FLAGS(af_alg_free_resources, 0x01);
KSYMTAB_FUNC(af_alg_poll, "");
SYMBOL_FLAGS(af_alg_poll, 0x01);
KSYMTAB_FUNC(af_alg_alloc_areq, "");
SYMBOL_FLAGS(af_alg_alloc_areq, 0x01);
KSYMTAB_FUNC(af_alg_get_rsgl, "");
SYMBOL_FLAGS(af_alg_get_rsgl, 0x01);

MODULE_INFO(depends, "");

