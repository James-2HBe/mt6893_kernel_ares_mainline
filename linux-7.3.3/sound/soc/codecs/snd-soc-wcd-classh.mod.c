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

KSYMTAB_FUNC(wcd_clsh_set_hph_mode, "");
SYMBOL_FLAGS(wcd_clsh_set_hph_mode, 0x01);
KSYMTAB_FUNC(wcd_clsh_ctrl_set_state, "");
SYMBOL_FLAGS(wcd_clsh_ctrl_set_state, 0x01);
KSYMTAB_FUNC(wcd_clsh_ctrl_get_state, "");
SYMBOL_FLAGS(wcd_clsh_ctrl_get_state, 0x01);
KSYMTAB_FUNC(wcd_clsh_ctrl_alloc, "");
SYMBOL_FLAGS(wcd_clsh_ctrl_alloc, 0x01);
KSYMTAB_FUNC(wcd_clsh_ctrl_free, "");
SYMBOL_FLAGS(wcd_clsh_ctrl_free, 0x01);

MODULE_INFO(depends, "snd-soc-core");

