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

KSYMTAB_FUNC(snd_timer_instance_new, "");
SYMBOL_FLAGS(snd_timer_instance_new, 0x00);
KSYMTAB_FUNC(snd_timer_instance_free, "");
SYMBOL_FLAGS(snd_timer_instance_free, 0x00);
KSYMTAB_FUNC(snd_timeri_timer_get, "");
SYMBOL_FLAGS(snd_timeri_timer_get, 0x01);
KSYMTAB_FUNC(snd_timeri_timer_put, "");
SYMBOL_FLAGS(snd_timeri_timer_put, 0x01);
KSYMTAB_FUNC(snd_timer_open, "");
SYMBOL_FLAGS(snd_timer_open, 0x00);
KSYMTAB_FUNC(snd_timer_close, "");
SYMBOL_FLAGS(snd_timer_close, 0x00);
KSYMTAB_FUNC(snd_timer_resolution, "");
SYMBOL_FLAGS(snd_timer_resolution, 0x00);
KSYMTAB_FUNC(snd_timer_start, "");
SYMBOL_FLAGS(snd_timer_start, 0x00);
KSYMTAB_FUNC(snd_timer_stop, "");
SYMBOL_FLAGS(snd_timer_stop, 0x00);
KSYMTAB_FUNC(snd_timer_continue, "");
SYMBOL_FLAGS(snd_timer_continue, 0x00);
KSYMTAB_FUNC(snd_timer_pause, "");
SYMBOL_FLAGS(snd_timer_pause, 0x00);
KSYMTAB_FUNC(snd_timer_interrupt, "");
SYMBOL_FLAGS(snd_timer_interrupt, 0x00);
KSYMTAB_FUNC(snd_timer_new, "");
SYMBOL_FLAGS(snd_timer_new, 0x00);
KSYMTAB_FUNC(snd_timer_notify, "");
SYMBOL_FLAGS(snd_timer_notify, 0x00);
KSYMTAB_FUNC(snd_timer_global_new, "");
SYMBOL_FLAGS(snd_timer_global_new, 0x00);
KSYMTAB_FUNC(snd_timer_global_free, "");
SYMBOL_FLAGS(snd_timer_global_free, 0x00);
KSYMTAB_FUNC(snd_timer_global_register, "");
SYMBOL_FLAGS(snd_timer_global_register, 0x00);

MODULE_INFO(depends, "snd");

