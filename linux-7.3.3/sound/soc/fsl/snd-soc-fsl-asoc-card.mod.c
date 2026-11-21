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


MODULE_INFO(depends, "snd-soc-core,snd-soc-simple-card-utils,snd-soc-imx-audmux,snd-pcm");

MODULE_ALIAS("of:N*T*Cfsl,imx-audio-ac97");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-ac97C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-cs42888");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-cs42888C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-cs427x");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-cs427xC*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-tlv320aic32x4");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-tlv320aic32x4C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-tlv320aic31xx");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-tlv320aic31xxC*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-sgtl5000");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-sgtl5000C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8962");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8962C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8960");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8960C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-mqs");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-mqsC*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8524");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8524C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-si476x");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-si476xC*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8958");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8958C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-nau8822");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-nau8822C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8904");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-wm8904C*");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-spdif");
MODULE_ALIAS("of:N*T*Cfsl,imx-audio-spdifC*");
