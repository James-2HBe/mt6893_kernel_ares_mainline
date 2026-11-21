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

KSYMTAB_FUNC(q6core_get_svc_api_info, "");
SYMBOL_FLAGS(q6core_get_svc_api_info, 0x01);
KSYMTAB_FUNC(q6core_is_adsp_ready, "");
SYMBOL_FLAGS(q6core_is_adsp_ready, 0x01);

MODULE_INFO(depends, "apr");

MODULE_ALIAS("of:N*T*Cqcom,q6core");
MODULE_ALIAS("of:N*T*Cqcom,q6coreC*");
