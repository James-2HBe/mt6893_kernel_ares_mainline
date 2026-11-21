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

KSYMTAB_FUNC(wcd937x_sdw_hw_params, "");
SYMBOL_FLAGS(wcd937x_sdw_hw_params, 0x01);

MODULE_INFO(depends, "soundwire-bus,regmap-sdw,snd-soc-wcd-common");

MODULE_ALIAS("sdw:m0217p010Av*c*");
