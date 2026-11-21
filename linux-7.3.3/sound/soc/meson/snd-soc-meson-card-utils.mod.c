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

KSYMTAB_FUNC(meson_card_i2s_set_sysclk, "");
SYMBOL_FLAGS(meson_card_i2s_set_sysclk, 0x01);
KSYMTAB_FUNC(meson_card_reallocate_links, "");
SYMBOL_FLAGS(meson_card_reallocate_links, 0x01);
KSYMTAB_FUNC(meson_card_parse_dai, "");
SYMBOL_FLAGS(meson_card_parse_dai, 0x01);
KSYMTAB_FUNC(meson_card_parse_daifmt, "");
SYMBOL_FLAGS(meson_card_parse_daifmt, 0x01);
KSYMTAB_FUNC(meson_card_set_be_link, "");
SYMBOL_FLAGS(meson_card_set_be_link, 0x01);
KSYMTAB_FUNC(meson_card_set_fe_link, "");
SYMBOL_FLAGS(meson_card_set_fe_link, 0x01);
KSYMTAB_FUNC(meson_card_probe, "");
SYMBOL_FLAGS(meson_card_probe, 0x01);
KSYMTAB_FUNC(meson_card_remove, "");
SYMBOL_FLAGS(meson_card_remove, 0x01);

MODULE_INFO(depends, "snd-soc-core");

