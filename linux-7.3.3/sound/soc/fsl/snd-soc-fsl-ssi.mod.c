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


MODULE_INFO(depends, "imx-pcm-dma,snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Cfsl,mpc8610-ssi");
MODULE_ALIAS("of:N*T*Cfsl,mpc8610-ssiC*");
MODULE_ALIAS("of:N*T*Cfsl,imx51-ssi");
MODULE_ALIAS("of:N*T*Cfsl,imx51-ssiC*");
MODULE_ALIAS("of:N*T*Cfsl,imx35-ssi");
MODULE_ALIAS("of:N*T*Cfsl,imx35-ssiC*");
MODULE_ALIAS("of:N*T*Cfsl,imx21-ssi");
MODULE_ALIAS("of:N*T*Cfsl,imx21-ssiC*");
