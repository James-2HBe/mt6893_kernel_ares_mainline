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


MODULE_INFO(depends, "snd-soc-qcom-common,snd-soc-core,snd-pcm,snd");

MODULE_ALIAS("of:N*T*Cgoogle,sc7180-trogdor");
MODULE_ALIAS("of:N*T*Cgoogle,sc7180-trogdorC*");
MODULE_ALIAS("of:N*T*Cgoogle,sc7180-coachz");
MODULE_ALIAS("of:N*T*Cgoogle,sc7180-coachzC*");
MODULE_ALIAS("of:N*T*Cqcom,sc7180-qdsp6-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sc7180-qdsp6-sndcardC*");
