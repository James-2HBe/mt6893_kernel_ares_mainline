#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.arch = MODULE_ARCH_INIT,
};

MODULE_INFO(intree, "Y");

KSYMTAB_FUNC(garp_request_join, "");
SYMBOL_FLAGS(garp_request_join, 0x01);
KSYMTAB_FUNC(garp_request_leave, "");
SYMBOL_FLAGS(garp_request_leave, 0x01);
KSYMTAB_FUNC(garp_init_applicant, "");
SYMBOL_FLAGS(garp_init_applicant, 0x01);
KSYMTAB_FUNC(garp_uninit_applicant, "");
SYMBOL_FLAGS(garp_uninit_applicant, 0x01);
KSYMTAB_FUNC(garp_register_application, "");
SYMBOL_FLAGS(garp_register_application, 0x01);
KSYMTAB_FUNC(garp_unregister_application, "");
SYMBOL_FLAGS(garp_unregister_application, 0x01);

MODULE_INFO(depends, "llc,stp");

