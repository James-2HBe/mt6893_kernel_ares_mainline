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


MODULE_INFO(depends, "v4l2-async,mc,v4l2-fwnode,videodev,videobuf2-common,videobuf2-v4l2,videobuf2-dma-contig");

MODULE_ALIAS("of:N*T*Crenesas,r9a09g047-cru");
MODULE_ALIAS("of:N*T*Crenesas,r9a09g047-cruC*");
MODULE_ALIAS("of:N*T*Crenesas,rzg2l-cru");
MODULE_ALIAS("of:N*T*Crenesas,rzg2l-cruC*");
