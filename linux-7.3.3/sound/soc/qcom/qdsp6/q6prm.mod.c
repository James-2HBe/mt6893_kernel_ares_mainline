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

KSYMTAB_FUNC(q6prm_vote_lpass_core_hw, "");
SYMBOL_FLAGS(q6prm_vote_lpass_core_hw, 0x01);
KSYMTAB_FUNC(q6prm_unvote_lpass_core_hw, "");
SYMBOL_FLAGS(q6prm_unvote_lpass_core_hw, 0x01);
KSYMTAB_FUNC(q6prm_set_lpass_clock, "");
SYMBOL_FLAGS(q6prm_set_lpass_clock, 0x01);

MODULE_INFO(depends, "snd-q6apm,apr");

MODULE_ALIAS("of:N*T*Cqcom,q6prm");
MODULE_ALIAS("of:N*T*Cqcom,q6prmC*");
