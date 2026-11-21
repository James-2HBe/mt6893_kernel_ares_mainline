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


MODULE_INFO(depends, "snd-soc-qcom-common,snd-soc-core,snd-soc-qcom-offload-utils,snd-soc-qcom-sdw");

MODULE_ALIAS("of:N*T*Cfairphone,fp4-sndcard");
MODULE_ALIAS("of:N*T*Cfairphone,fp4-sndcardC*");
MODULE_ALIAS("of:N*T*Cfairphone,fp5-sndcard");
MODULE_ALIAS("of:N*T*Cfairphone,fp5-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qrb2210-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qrb2210-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qrb4210-rb2-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qrb4210-rb2-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qrb5165-rb5-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qrb5165-rb5-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sdm660-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sdm660-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-sndcardC*");
