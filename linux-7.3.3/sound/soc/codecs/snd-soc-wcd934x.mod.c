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


MODULE_INFO(depends, "slimbus,regmap-slimbus,snd-soc-wcd-mbhc,snd-soc-core,snd-soc-wcd-classh,snd-soc-wcd-common,snd-pcm");

MODULE_ALIAS("platform:wcd934x-codec");
