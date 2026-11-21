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

KSYMTAB_FUNC(ip_tunnel_lookup, "");
SYMBOL_FLAGS(ip_tunnel_lookup, 0x01);
KSYMTAB_FUNC(ip_tunnel_md_udp_encap, "");
SYMBOL_FLAGS(ip_tunnel_md_udp_encap, 0x00);
KSYMTAB_FUNC(ip_tunnel_rcv, "");
SYMBOL_FLAGS(ip_tunnel_rcv, 0x01);
KSYMTAB_FUNC(ip_tunnel_encap_add_ops, "");
SYMBOL_FLAGS(ip_tunnel_encap_add_ops, 0x00);
KSYMTAB_FUNC(ip_tunnel_encap_del_ops, "");
SYMBOL_FLAGS(ip_tunnel_encap_del_ops, 0x00);
KSYMTAB_FUNC(ip_tunnel_encap_setup, "");
SYMBOL_FLAGS(ip_tunnel_encap_setup, 0x01);
KSYMTAB_FUNC(ip_md_tunnel_xmit, "");
SYMBOL_FLAGS(ip_md_tunnel_xmit, 0x01);
KSYMTAB_FUNC(ip_tunnel_xmit, "");
SYMBOL_FLAGS(ip_tunnel_xmit, 0x01);
KSYMTAB_FUNC(ip_tunnel_ctl, "");
SYMBOL_FLAGS(ip_tunnel_ctl, 0x01);
KSYMTAB_FUNC(ip_tunnel_parm_from_user, "");
SYMBOL_FLAGS(ip_tunnel_parm_from_user, 0x01);
KSYMTAB_FUNC(ip_tunnel_parm_to_user, "");
SYMBOL_FLAGS(ip_tunnel_parm_to_user, 0x01);
KSYMTAB_FUNC(ip_tunnel_siocdevprivate, "");
SYMBOL_FLAGS(ip_tunnel_siocdevprivate, 0x01);
KSYMTAB_FUNC(ip_tunnel_change_mtu, "");
SYMBOL_FLAGS(ip_tunnel_change_mtu, 0x01);
KSYMTAB_FUNC(ip_tunnel_dellink, "");
SYMBOL_FLAGS(ip_tunnel_dellink, 0x01);
KSYMTAB_FUNC(ip_tunnel_get_link_net, "");
SYMBOL_FLAGS(ip_tunnel_get_link_net, 0x00);
KSYMTAB_FUNC(ip_tunnel_get_iflink, "");
SYMBOL_FLAGS(ip_tunnel_get_iflink, 0x00);
KSYMTAB_FUNC(ip_tunnel_init_net, "");
SYMBOL_FLAGS(ip_tunnel_init_net, 0x01);
KSYMTAB_FUNC(ip_tunnel_delete_net, "");
SYMBOL_FLAGS(ip_tunnel_delete_net, 0x01);
KSYMTAB_FUNC(ip_tunnel_newlink, "");
SYMBOL_FLAGS(ip_tunnel_newlink, 0x01);
KSYMTAB_FUNC(ip_tunnel_changelink, "");
SYMBOL_FLAGS(ip_tunnel_changelink, 0x01);
KSYMTAB_FUNC(__ip_tunnel_init, "");
SYMBOL_FLAGS(__ip_tunnel_init, 0x01);
KSYMTAB_FUNC(ip_tunnel_uninit, "");
SYMBOL_FLAGS(ip_tunnel_uninit, 0x01);
KSYMTAB_FUNC(ip_tunnel_setup, "");
SYMBOL_FLAGS(ip_tunnel_setup, 0x01);

MODULE_INFO(depends, "");

