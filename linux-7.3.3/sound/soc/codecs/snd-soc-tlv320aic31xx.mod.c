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


MODULE_INFO(depends, "snd-soc-core,snd-pcm");

MODULE_ALIAS("of:N*T*Cti,tlv320aic310x");
MODULE_ALIAS("of:N*T*Cti,tlv320aic310xC*");
MODULE_ALIAS("of:N*T*Cti,tlv320aic311x");
MODULE_ALIAS("of:N*T*Cti,tlv320aic311xC*");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3100");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3100C*");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3110");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3110C*");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3120");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3120C*");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3111");
MODULE_ALIAS("of:N*T*Cti,tlv320aic3111C*");
MODULE_ALIAS("of:N*T*Cti,tlv320dac3100");
MODULE_ALIAS("of:N*T*Cti,tlv320dac3100C*");
MODULE_ALIAS("of:N*T*Cti,tlv320dac3101");
MODULE_ALIAS("of:N*T*Cti,tlv320dac3101C*");
MODULE_ALIAS("acpi*:10TI3100:*");
MODULE_ALIAS("i2c:tlv320aic310x");
MODULE_ALIAS("i2c:tlv320aic311x");
MODULE_ALIAS("i2c:tlv320aic3100");
MODULE_ALIAS("i2c:tlv320aic3110");
MODULE_ALIAS("i2c:tlv320aic3120");
MODULE_ALIAS("i2c:tlv320aic3111");
MODULE_ALIAS("i2c:tlv320dac3100");
MODULE_ALIAS("i2c:tlv320dac3101");
