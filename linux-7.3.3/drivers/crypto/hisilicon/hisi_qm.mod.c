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

KSYMTAB_FUNC(hisi_qm_q_num_set, "");
SYMBOL_FLAGS(hisi_qm_q_num_set, 0x01);
KSYMTAB_FUNC(hisi_qm_wait_mb_ready, "");
SYMBOL_FLAGS(hisi_qm_wait_mb_ready, 0x01);
KSYMTAB_FUNC(hisi_qm_mb, "");
SYMBOL_FLAGS(hisi_qm_mb, 0x01);
KSYMTAB_FUNC(hisi_qm_mb_read, "");
SYMBOL_FLAGS(hisi_qm_mb_read, 0x01);
KSYMTAB_FUNC(hisi_qm_get_hw_info, "");
SYMBOL_FLAGS(hisi_qm_get_hw_info, 0x01);
KSYMTAB_FUNC(hisi_qm_get_cap_value, "");
SYMBOL_FLAGS(hisi_qm_get_cap_value, 0x01);
KSYMTAB_FUNC(hisi_qm_set_algs, "");
SYMBOL_FLAGS(hisi_qm_set_algs, 0x01);
KSYMTAB_FUNC(hisi_qp_send, "");
SYMBOL_FLAGS(hisi_qp_send, 0x01);
KSYMTAB_FUNC(hisi_qm_register_uacce, "");
SYMBOL_FLAGS(hisi_qm_register_uacce, 0x01);
KSYMTAB_FUNC(hisi_qm_wait_task_finish, "");
SYMBOL_FLAGS(hisi_qm_wait_task_finish, 0x01);
KSYMTAB_FUNC(hisi_qm_uninit, "");
SYMBOL_FLAGS(hisi_qm_uninit, 0x01);
KSYMTAB_FUNC(hisi_qm_start, "");
SYMBOL_FLAGS(hisi_qm_start, 0x01);
KSYMTAB_FUNC(hisi_qm_stop, "");
SYMBOL_FLAGS(hisi_qm_stop, 0x01);
KSYMTAB_FUNC(hisi_qm_dev_err_init, "");
SYMBOL_FLAGS(hisi_qm_dev_err_init, 0x01);
KSYMTAB_FUNC(hisi_qm_dev_err_uninit, "");
SYMBOL_FLAGS(hisi_qm_dev_err_uninit, 0x01);
KSYMTAB_FUNC(hisi_qm_free_qps, "");
SYMBOL_FLAGS(hisi_qm_free_qps, 0x01);
KSYMTAB_FUNC(hisi_qm_alloc_qps_node, "");
SYMBOL_FLAGS(hisi_qm_alloc_qps_node, 0x01);
KSYMTAB_FUNC(hisi_qm_sriov_enable, "");
SYMBOL_FLAGS(hisi_qm_sriov_enable, 0x01);
KSYMTAB_FUNC(hisi_qm_sriov_disable, "");
SYMBOL_FLAGS(hisi_qm_sriov_disable, 0x01);
KSYMTAB_FUNC(hisi_qm_sriov_configure, "");
SYMBOL_FLAGS(hisi_qm_sriov_configure, 0x01);
KSYMTAB_FUNC(hisi_qm_dev_err_detected, "");
SYMBOL_FLAGS(hisi_qm_dev_err_detected, 0x01);
KSYMTAB_FUNC(hisi_qm_dev_slot_reset, "");
SYMBOL_FLAGS(hisi_qm_dev_slot_reset, 0x01);
KSYMTAB_FUNC(hisi_qm_reset_prepare, "");
SYMBOL_FLAGS(hisi_qm_reset_prepare, 0x01);
KSYMTAB_FUNC(hisi_qm_reset_done, "");
SYMBOL_FLAGS(hisi_qm_reset_done, 0x01);
KSYMTAB_FUNC(hisi_qm_dev_shutdown, "");
SYMBOL_FLAGS(hisi_qm_dev_shutdown, 0x01);
KSYMTAB_FUNC(hisi_qm_alg_register, "");
SYMBOL_FLAGS(hisi_qm_alg_register, 0x01);
KSYMTAB_FUNC(hisi_qm_alg_unregister, "");
SYMBOL_FLAGS(hisi_qm_alg_unregister, 0x01);
KSYMTAB_FUNC(hisi_qm_init, "");
SYMBOL_FLAGS(hisi_qm_init, 0x01);
KSYMTAB_FUNC(hisi_qm_get_dfx_access, "");
SYMBOL_FLAGS(hisi_qm_get_dfx_access, 0x01);
KSYMTAB_FUNC(hisi_qm_put_dfx_access, "");
SYMBOL_FLAGS(hisi_qm_put_dfx_access, 0x01);
KSYMTAB_FUNC(hisi_qm_pm_init, "");
SYMBOL_FLAGS(hisi_qm_pm_init, 0x01);
KSYMTAB_FUNC(hisi_qm_pm_uninit, "");
SYMBOL_FLAGS(hisi_qm_pm_uninit, 0x01);
KSYMTAB_FUNC(hisi_qm_suspend, "");
SYMBOL_FLAGS(hisi_qm_suspend, 0x01);
KSYMTAB_FUNC(hisi_qm_resume, "");
SYMBOL_FLAGS(hisi_qm_resume, 0x01);
KSYMTAB_FUNC(hisi_acc_create_sgl_pool, "");
SYMBOL_FLAGS(hisi_acc_create_sgl_pool, 0x01);
KSYMTAB_FUNC(hisi_acc_free_sgl_pool, "");
SYMBOL_FLAGS(hisi_acc_free_sgl_pool, 0x01);
KSYMTAB_FUNC(hisi_acc_sg_buf_map_to_hw_sgl, "");
SYMBOL_FLAGS(hisi_acc_sg_buf_map_to_hw_sgl, 0x01);
KSYMTAB_FUNC(hisi_acc_sg_buf_unmap, "");
SYMBOL_FLAGS(hisi_acc_sg_buf_unmap, 0x01);
KSYMTAB_FUNC(hisi_qm_regs_dump, "");
SYMBOL_FLAGS(hisi_qm_regs_dump, 0x01);
KSYMTAB_FUNC(hisi_qm_regs_debugfs_init, "");
SYMBOL_FLAGS(hisi_qm_regs_debugfs_init, 0x01);
KSYMTAB_FUNC(hisi_qm_regs_debugfs_uninit, "");
SYMBOL_FLAGS(hisi_qm_regs_debugfs_uninit, 0x01);
KSYMTAB_FUNC(hisi_qm_acc_diff_regs_dump, "");
SYMBOL_FLAGS(hisi_qm_acc_diff_regs_dump, 0x01);
KSYMTAB_FUNC(hisi_qm_debug_init, "");
SYMBOL_FLAGS(hisi_qm_debug_init, 0x01);
KSYMTAB_FUNC(hisi_qm_debug_regs_clear, "");
SYMBOL_FLAGS(hisi_qm_debug_regs_clear, 0x01);

MODULE_INFO(depends, "uacce");

