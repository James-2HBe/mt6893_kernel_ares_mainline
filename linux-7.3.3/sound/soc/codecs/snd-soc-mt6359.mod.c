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

KSYMTAB_FUNC(mt6359_set_mtkaif_protocol, "");
SYMBOL_FLAGS(mt6359_set_mtkaif_protocol, 0x01);
KSYMTAB_FUNC(mt6359_mtkaif_calibration_enable, "");
SYMBOL_FLAGS(mt6359_mtkaif_calibration_enable, 0x01);
KSYMTAB_FUNC(mt6359_mtkaif_calibration_disable, "");
SYMBOL_FLAGS(mt6359_mtkaif_calibration_disable, 0x01);
KSYMTAB_FUNC(mt6359_set_mtkaif_calibration_phase, "");
SYMBOL_FLAGS(mt6359_set_mtkaif_calibration_phase, 0x01);

MODULE_INFO(depends, "snd-soc-core");

