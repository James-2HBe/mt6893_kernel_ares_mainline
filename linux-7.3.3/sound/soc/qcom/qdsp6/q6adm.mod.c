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

KSYMTAB_FUNC(q6adm_open, "");
SYMBOL_FLAGS(q6adm_open, 0x01);
KSYMTAB_FUNC(q6adm_get_copp_id, "");
SYMBOL_FLAGS(q6adm_get_copp_id, 0x01);
KSYMTAB_FUNC(q6adm_matrix_map, "");
SYMBOL_FLAGS(q6adm_matrix_map, 0x01);
KSYMTAB_FUNC(q6adm_close, "");
SYMBOL_FLAGS(q6adm_close, 0x01);

MODULE_INFO(depends, "q6afe,snd-q6dsp-common,apr,q6core");

MODULE_ALIAS("of:N*T*Cqcom,q6adm");
MODULE_ALIAS("of:N*T*Cqcom,q6admC*");
