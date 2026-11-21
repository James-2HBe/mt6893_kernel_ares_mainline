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


MODULE_INFO(depends, "snd-soc-core,snd-soc-qcom-common,snd-pcm,snd-soc-qcom-sdw");

MODULE_ALIAS("of:N*T*Cayaneo,pocket-s2-sndcard");
MODULE_ALIAS("of:N*T*Cayaneo,pocket-s2-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,eliza-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,eliza-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,hawi-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,hawi-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,kaanapali-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,kaanapali-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,maili-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,maili-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcm6490-idp-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcm6490-idp-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcs615-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcs615-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcs6490-rb3gen2-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcs6490-rb3gen2-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcs8275-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcs8275-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcs9075-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcs9075-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,qcs9100-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,qcs9100-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8475-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8475-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8650-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8650-sndcardC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8750-sndcard");
MODULE_ALIAS("of:N*T*Cqcom,sm8750-sndcardC*");
