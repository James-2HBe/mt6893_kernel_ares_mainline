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


MODULE_INFO(depends, "snd-soc-fsl-utils,imx-pcm-dma,snd-soc-core,snd");

MODULE_ALIAS("of:N*T*Cfsl,imx35-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx35-spdifC*");
MODULE_ALIAS("of:N*T*Cfsl,vf610-spdif");
MODULE_ALIAS("of:N*T*Cfsl,vf610-spdifC*");
MODULE_ALIAS("of:N*T*Cfsl,imx6sx-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx6sx-spdifC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8qm-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx8qm-spdifC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8mm-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx8mm-spdifC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8ulp-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx8ulp-spdifC*");
