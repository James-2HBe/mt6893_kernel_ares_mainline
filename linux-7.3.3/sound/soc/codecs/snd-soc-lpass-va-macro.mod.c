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


MODULE_INFO(depends, "snd-soc-lpass-macro-common,snd-soc-core");

MODULE_ALIAS("of:N*T*Cqcom,sc7280-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sc7280-lpass-va-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm6115-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm6115-lpass-va-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8250-lpass-va-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8450-lpass-va-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sm8550-lpass-va-macroC*");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-lpass-va-macro");
MODULE_ALIAS("of:N*T*Cqcom,sc8280xp-lpass-va-macroC*");
