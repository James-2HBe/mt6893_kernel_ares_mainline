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

KSYMTAB_DATA(fsl_asrc_component, "");
SYMBOL_FLAGS(fsl_asrc_component, 0x01);
KSYMTAB_FUNC(fsl_asrc_m2m_suspend, "");
SYMBOL_FLAGS(fsl_asrc_m2m_suspend, 0x01);
KSYMTAB_FUNC(fsl_asrc_m2m_resume, "");
SYMBOL_FLAGS(fsl_asrc_m2m_resume, 0x01);
KSYMTAB_FUNC(fsl_asrc_m2m_init, "");
SYMBOL_FLAGS(fsl_asrc_m2m_init, 0x01);
KSYMTAB_FUNC(fsl_asrc_m2m_exit, "");
SYMBOL_FLAGS(fsl_asrc_m2m_exit, 0x01);

MODULE_INFO(depends, "snd-soc-core,snd-pcm,snd-pcm-dmaengine,snd,snd-compress");

MODULE_ALIAS("of:N*T*Cfsl,imx35-asrc");
MODULE_ALIAS("of:N*T*Cfsl,imx35-asrcC*");
MODULE_ALIAS("of:N*T*Cfsl,imx53-asrc");
MODULE_ALIAS("of:N*T*Cfsl,imx53-asrcC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8qm-asrc");
MODULE_ALIAS("of:N*T*Cfsl,imx8qm-asrcC*");
MODULE_ALIAS("of:N*T*Cfsl,imx8qxp-asrc");
MODULE_ALIAS("of:N*T*Cfsl,imx8qxp-asrcC*");
MODULE_ALIAS("of:N*T*Cfsl,imx952-asrc");
MODULE_ALIAS("of:N*T*Cfsl,imx952-asrcC*");
