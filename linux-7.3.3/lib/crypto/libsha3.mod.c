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

KSYMTAB_FUNC(__sha3_update, "");
SYMBOL_FLAGS(__sha3_update, 0x01);
KSYMTAB_FUNC(sha3_final, "");
SYMBOL_FLAGS(sha3_final, 0x01);
KSYMTAB_FUNC(shake_squeeze, "");
SYMBOL_FLAGS(shake_squeeze, 0x01);
KSYMTAB_FUNC(sha3_224, "");
SYMBOL_FLAGS(sha3_224, 0x01);
KSYMTAB_FUNC(sha3_256, "");
SYMBOL_FLAGS(sha3_256, 0x01);
KSYMTAB_FUNC(sha3_384, "");
SYMBOL_FLAGS(sha3_384, 0x01);
KSYMTAB_FUNC(sha3_512, "");
SYMBOL_FLAGS(sha3_512, 0x01);
KSYMTAB_FUNC(shake128, "");
SYMBOL_FLAGS(shake128, 0x01);
KSYMTAB_FUNC(shake256, "");
SYMBOL_FLAGS(shake256, 0x01);

MODULE_INFO(depends, "");

