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

KSYMTAB_FUNC(ip_vs_conn_in_get_proto, "");
SYMBOL_FLAGS(ip_vs_conn_in_get_proto, 0x01);
KSYMTAB_FUNC(ip_vs_conn_out_get_proto, "");
SYMBOL_FLAGS(ip_vs_conn_out_get_proto, 0x01);
KSYMTAB_FUNC(register_ip_vs_scheduler, "");
SYMBOL_FLAGS(register_ip_vs_scheduler, 0x00);
KSYMTAB_FUNC(unregister_ip_vs_scheduler, "");
SYMBOL_FLAGS(unregister_ip_vs_scheduler, 0x00);
KSYMTAB_FUNC(ip_vs_proto_name, "");
SYMBOL_FLAGS(ip_vs_proto_name, 0x00);
KSYMTAB_FUNC(ip_vs_conn_new, "");
SYMBOL_FLAGS(ip_vs_conn_new, 0x00);
KSYMTAB_FUNC(ip_vs_conn_in_get, "");
SYMBOL_FLAGS(ip_vs_conn_in_get, 0x00);
KSYMTAB_FUNC(ip_vs_conn_out_get, "");
SYMBOL_FLAGS(ip_vs_conn_out_get, 0x00);
KSYMTAB_FUNC(ip_vs_conn_put, "");
SYMBOL_FLAGS(ip_vs_conn_put, 0x00);
KSYMTAB_FUNC(ip_vs_new_conn_out, "");
SYMBOL_FLAGS(ip_vs_new_conn_out, 0x00);
KSYMTAB_FUNC(ip_vs_scheduler_err, "");
SYMBOL_FLAGS(ip_vs_scheduler_err, 0x00);
KSYMTAB_FUNC(register_ip_vs_app, "");
SYMBOL_FLAGS(register_ip_vs_app, 0x00);
KSYMTAB_FUNC(unregister_ip_vs_app, "");
SYMBOL_FLAGS(unregister_ip_vs_app, 0x00);
KSYMTAB_FUNC(register_ip_vs_app_inc, "");
SYMBOL_FLAGS(register_ip_vs_app_inc, 0x00);
KSYMTAB_FUNC(ip_vs_proto_get, "");
SYMBOL_FLAGS(ip_vs_proto_get, 0x00);
KSYMTAB_FUNC(ip_vs_proto_data_get, "");
SYMBOL_FLAGS(ip_vs_proto_data_get, 0x00);
KSYMTAB_FUNC(register_ip_vs_pe, "");
SYMBOL_FLAGS(register_ip_vs_pe, 0x01);
KSYMTAB_FUNC(unregister_ip_vs_pe, "");
SYMBOL_FLAGS(unregister_ip_vs_pe, 0x01);

MODULE_INFO(depends, "nf_conntrack");

