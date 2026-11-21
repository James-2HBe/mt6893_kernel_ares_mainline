/* SPDX-License-Identifier: GPL-2.0 */
/*
 * impl-symbols.c — 15 个 MTK 私有驱动接入内核所需的 31 个符号 weak 兜底桩
 *
 * 目标内核 Linux 7.3.3。这 31 个符号来自 4.14 MTK 平台基线层 / PPM / clkbuf /
 * ssusb / AEE 子系统，主线无等价、本地 baseline 也无真实现。全部用
 * __weak + EXPORT_SYMBOL_GPL 安全兜底：真实平台提供强符号时自动覆盖，
 * 否则返回安全值/空操作，保证 15 个驱动可链接、可启动。
 *
 * 签名用 void* 代替私有结构指针（C 链接只匹配符号名；weak 桩为无副作用
 * 空操作，指针 ABI 兼容）。详见 reports/DRIVER-INTEGRATION-PLAN.md。
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/compiler.h>
#include <linux/types.h>
#include <linux/spinlock.h>

/* --- AEE 日志（2）：空操作，避免 AEE 子系统依赖 --- */
void __weak aee_kernel_warning_api_func(const char *file, int line, int db_opt,
					const char *module, const char *msg, ...)
{
}
EXPORT_SYMBOL_GPL(aee_kernel_warning_api_func);
void __weak aee_kernel_exception_api_func(const char *file, int line, int db_opt,
					  const char *module, const char *msg, ...)
{
}
EXPORT_SYMBOL_GPL(aee_kernel_exception_api_func);

/* --- clkbuf 状态/查询（7）--- */
int __weak bblpm_cnt;
EXPORT_SYMBOL_GPL(bblpm_cnt);
int __weak clk_buf_ctrl_lock;
EXPORT_SYMBOL_GPL(clk_buf_ctrl_lock);
int __weak clkbuf_ctrl_stat;
EXPORT_SYMBOL_GPL(clkbuf_ctrl_stat);
int __weak clkbuf_debug;
EXPORT_SYMBOL_GPL(clkbuf_debug);
int __weak is_clkbuf_initiated;
EXPORT_SYMBOL_GPL(is_clkbuf_initiated);
bool __weak is_clk_buf_under_flightmode(void) { return false; }
EXPORT_SYMBOL_GPL(is_clk_buf_under_flightmode);
bool __weak is_pmic_clkbuf(void) { return false; }
EXPORT_SYMBOL_GPL(is_pmic_clkbuf);
spinlock_t *__weak get_mtk_clk_lock(void) { return NULL; }
EXPORT_SYMBOL_GPL(get_mtk_clk_lock);

/* --- 平台全局量（4）--- */
int __weak clstr_num;
EXPORT_SYMBOL_GPL(clstr_num);
int __weak policy_dir;
EXPORT_SYMBOL_GPL(policy_dir);
int __weak force_get_tbat(bool update) { return 0; }
EXPORT_SYMBOL_GPL(force_get_tbat);
int __weak get_boot_mode(void) { return 0; }
EXPORT_SYMBOL_GPL(get_boot_mode);

/* --- 平台函数（2）--- */
void __weak mtk_check_subsys_swcg(int id) {}
EXPORT_SYMBOL_GPL(mtk_check_subsys_swcg);
int __weak mtk_lp_sysfs_entry_func_node_add(const char *name, int mode,
					    const void *op, void *parent, void *node)
{ return 0; }
EXPORT_SYMBOL_GPL(mtk_lp_sysfs_entry_func_node_add);

/* --- PPM 子系统内部（7）：空操作/安全返回 --- */
int __weak ppm_procfs_init(void) { return 0; }
EXPORT_SYMBOL_GPL(ppm_procfs_init);
int __weak ppm_profile_init(void) { return 0; }
EXPORT_SYMBOL_GPL(ppm_profile_init);
void __weak ppm_profile_exit(void) {}
EXPORT_SYMBOL_GPL(ppm_profile_exit);
void __weak ppm_profile_update_client_exec_time(int client, unsigned long long time) {}
EXPORT_SYMBOL_GPL(ppm_profile_update_client_exec_time);
void __weak ppm_cobra_update_core_limit(unsigned int cluster, int limit) {}
EXPORT_SYMBOL_GPL(ppm_cobra_update_core_limit);
void __weak ppm_cobra_update_freq_limit(unsigned int cluster, int limit) {}
EXPORT_SYMBOL_GPL(ppm_cobra_update_freq_limit);
char *__weak ppm_copy_from_user_for_proc(const char __user *buffer, size_t count)
{ return NULL; }
EXPORT_SYMBOL_GPL(ppm_copy_from_user_for_proc);

/* --- USB ssusb（5）：主线 mtu3 已完整，桩仅兜底 --- */
void __weak ssusb_clks_disable(void *ssusb) {}
EXPORT_SYMBOL_GPL(ssusb_clks_disable);
int __weak ssusb_clks_enable(void *ssusb) { return 0; }
EXPORT_SYMBOL_GPL(ssusb_clks_enable);
void __weak ssusb_ip_sw_reset(void *ssusb) {}
EXPORT_SYMBOL_GPL(ssusb_ip_sw_reset);
void __weak ssusb_phy_power_off(void *ssusb) {}
EXPORT_SYMBOL_GPL(ssusb_phy_power_off);
int __weak ssusb_phy_power_on(void *ssusb) { return 0; }
EXPORT_SYMBOL_GPL(ssusb_phy_power_on);

/* --- 其他（2）--- */
void __weak print_enabled_clks_once(void) {}
EXPORT_SYMBOL_GPL(print_enabled_clks_once);
int __weak pseudo_debug_init(void *data) { return 0; }
EXPORT_SYMBOL_GPL(pseudo_debug_init);

/* --- strncpy：7.3 主线已从 string.h 移除，这里提供兼容实现 --- */
/* 注意：非 weak（标准库语义），仅供本批驱动链接使用。 */
char *strncpy(char *__restrict dest, const char *__restrict src, size_t n)
{
	char *d = dest;

	while (n && (*d = *src)) {
		d++;
		src++;
		n--;
	}
	while (n) {
		*d++ = '\0';
		n--;
	}
	return dest;
}
EXPORT_SYMBOL_GPL(strncpy);
