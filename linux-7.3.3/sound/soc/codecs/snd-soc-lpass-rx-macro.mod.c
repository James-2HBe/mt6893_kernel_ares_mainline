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


MODULE_INFO(depends, "snd-soc-lpass-macro-common,snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Cqcom,sc7280-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sc7280-lpass-rx-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm6115-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm6115-lpass-rx-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-lpass-rx-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-lpass-rx-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-lpass-rx-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-lpass-rx-macro");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-lpass-rx-macroC*");
