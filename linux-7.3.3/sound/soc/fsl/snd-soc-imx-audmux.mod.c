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

KSYMTAB_FUNC(imx_audmux_v1_configure_port, "");
SYMBOL_FLAGS(imx_audmux_v1_configure_port, 0x01);
KSYMTAB_FUNC(imx_audmux_v2_configure_port, "");
SYMBOL_FLAGS(imx_audmux_v2_configure_port, 0x01);

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Cfsl,imx21-audmux");
MODULE_ALIAS("of:N*T*Cfsl,imx21-audmuxC*");
MODULE_ALIAS("of:N*T*Cfsl,imx31-audmux");
MODULE_ALIAS("of:N*T*Cfsl,imx31-audmuxC*");
