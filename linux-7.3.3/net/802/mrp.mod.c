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

KSYMTAB_FUNC(mrp_request_join, "");
SYMBOL_FLAGS(mrp_request_join, 0x01);
KSYMTAB_FUNC(mrp_request_leave, "");
SYMBOL_FLAGS(mrp_request_leave, 0x01);
KSYMTAB_FUNC(mrp_init_applicant, "");
SYMBOL_FLAGS(mrp_init_applicant, 0x01);
KSYMTAB_FUNC(mrp_uninit_applicant, "");
SYMBOL_FLAGS(mrp_uninit_applicant, 0x01);
KSYMTAB_FUNC(mrp_register_application, "");
SYMBOL_FLAGS(mrp_register_application, 0x01);
KSYMTAB_FUNC(mrp_unregister_application, "");
SYMBOL_FLAGS(mrp_unregister_application, 0x01);

MODULE_INFO(depends, "");

