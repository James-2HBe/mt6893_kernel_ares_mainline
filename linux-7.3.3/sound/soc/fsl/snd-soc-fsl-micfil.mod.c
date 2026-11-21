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


MODULE_INFO(depends, "snd-soc-fsl-utils,snd-soc-core,snd,snd-pcm");

MODULE_ALIAS("of:N*T*Cfsl,imx8mm-micfil");
MODULE_ALIAS("of:N*T*Cfsl,imx8mm-micfilC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8mp-micfil");
MODULE_ALIAS("of:N*T*Cfsl,imx8mp-micfilC*");
MODULE_ALIAS("of:N*T*Cfsl,imx93-micfil");
MODULE_ALIAS("of:N*T*Cfsl,imx93-micfilC*");
MODULE_ALIAS("of:N*T*Cfsl,imx943-micfil");
MODULE_ALIAS("of:N*T*Cfsl,imx943-micfilC*");
