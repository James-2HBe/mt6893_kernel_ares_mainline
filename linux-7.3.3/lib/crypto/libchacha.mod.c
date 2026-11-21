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

KSYMTAB_FUNC(chacha_crypt, "");
SYMBOL_FLAGS(chacha_crypt, 0x01);
KSYMTAB_FUNC(hchacha_block, "");
SYMBOL_FLAGS(hchacha_block, 0x01);

MODULE_INFO(depends, "");

