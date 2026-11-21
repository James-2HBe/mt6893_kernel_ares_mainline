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

KSYMTAB_FUNC(rfkill_get_led_trigger_name, "");
SYMBOL_FLAGS(rfkill_get_led_trigger_name, 0x00);
KSYMTAB_FUNC(rfkill_set_led_trigger_name, "");
SYMBOL_FLAGS(rfkill_set_led_trigger_name, 0x00);
KSYMTAB_FUNC(rfkill_set_hw_state_reason, "");
SYMBOL_FLAGS(rfkill_set_hw_state_reason, 0x00);
KSYMTAB_FUNC(rfkill_set_sw_state, "");
SYMBOL_FLAGS(rfkill_set_sw_state, 0x00);
KSYMTAB_FUNC(rfkill_init_sw_state, "");
SYMBOL_FLAGS(rfkill_init_sw_state, 0x00);
KSYMTAB_FUNC(rfkill_set_states, "");
SYMBOL_FLAGS(rfkill_set_states, 0x00);
KSYMTAB_FUNC(rfkill_find_type, "");
SYMBOL_FLAGS(rfkill_find_type, 0x00);
KSYMTAB_FUNC(rfkill_pause_polling, "");
SYMBOL_FLAGS(rfkill_pause_polling, 0x00);
KSYMTAB_FUNC(rfkill_resume_polling, "");
SYMBOL_FLAGS(rfkill_resume_polling, 0x00);
KSYMTAB_FUNC(rfkill_blocked, "");
SYMBOL_FLAGS(rfkill_blocked, 0x00);
KSYMTAB_FUNC(rfkill_soft_blocked, "");
SYMBOL_FLAGS(rfkill_soft_blocked, 0x00);
KSYMTAB_FUNC(rfkill_alloc, "");
SYMBOL_FLAGS(rfkill_alloc, 0x00);
KSYMTAB_FUNC(rfkill_register, "");
SYMBOL_FLAGS(rfkill_register, 0x00);
KSYMTAB_FUNC(rfkill_unregister, "");
SYMBOL_FLAGS(rfkill_unregister, 0x00);
KSYMTAB_FUNC(rfkill_destroy, "");
SYMBOL_FLAGS(rfkill_destroy, 0x00);

MODULE_INFO(depends, "");

