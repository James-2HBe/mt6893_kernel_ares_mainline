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


MODULE_INFO(depends, "snd-soc-ti-edma,snd-soc-ti-udma,snd-soc-ti-sdma,snd-pcm,snd-soc-core");

MODULE_ALIAS("of:N*T*Cti,dm646x-mcasp-audio");
MODULE_ALIAS("of:N*T*Cti,dm646x-mcasp-audioC*");
MODULE_ALIAS("of:N*T*Cti,da830-mcasp-audio");
MODULE_ALIAS("of:N*T*Cti,da830-mcasp-audioC*");
MODULE_ALIAS("of:N*T*Cti,am33xx-mcasp-audio");
MODULE_ALIAS("of:N*T*Cti,am33xx-mcasp-audioC*");
MODULE_ALIAS("of:N*T*Cti,dra7-mcasp-audio");
MODULE_ALIAS("of:N*T*Cti,dra7-mcasp-audioC*");
MODULE_ALIAS("of:N*T*Cti,omap4-mcasp-audio");
MODULE_ALIAS("of:N*T*Cti,omap4-mcasp-audioC*");
