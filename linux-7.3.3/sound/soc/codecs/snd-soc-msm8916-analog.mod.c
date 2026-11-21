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


MODULE_INFO(depends, "snd-soc-core");

MODULE_ALIAS("of:N*T*Cqcom,pm8916-wcd-analog-codec");
MODULE_ALIAS("of:N*T*Cqcom,pm8916-wcd-analog-codecC*");
MODULE_ALIAS("of:N*T*Cqcom,pm8950-wcd-analog-codec");
MODULE_ALIAS("of:N*T*Cqcom,pm8950-wcd-analog-codecC*");
MODULE_ALIAS("of:N*T*Cqcom,pm8953-wcd-analog-codec");
MODULE_ALIAS("of:N*T*Cqcom,pm8953-wcd-analog-codecC*");
