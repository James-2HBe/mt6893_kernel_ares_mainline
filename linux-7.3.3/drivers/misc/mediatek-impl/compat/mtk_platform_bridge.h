/* SPDX-License-Identifier: GPL-2.0 */
/*
 * mtk_platform_bridge.h — 路径 B 自包含平台桥接层（类型 / 枚举 / 宏 / 寄存器占位）
 *
 * 目的：让 baseline/impl-adapted/ 的 21 个 4.14 私有驱动在「不依赖目标树平台
 * 基线层」的前提下完成 Linux 7.3.3 语法编译，从而消除文档里那句遗留边界：
 *   「路径 B（21 个 4.14 私有驱动）若整体移植仍需目标树自行做 API 适配」。
 *
 * 内容分两类：
 *   1) 平台私有且主线无等价的类型/枚举/宏/函数前向声明 —— 本包必须自带；
 *   2) 构建期由 MTK 私有寄存器生成器产出的芯片寄存器 id —— 用 #ifndef 守卫占位，
 *      目标树真实生成的芯片映射头若提供同名定义，本桥接层自动让位（不重定义）。
 *
 * 所有「会与目标树真实定义撞车」的项都带哨兵宏守卫；若目标树已修基线层提供
 * 真实定义，可在包含本头之前 #define 对应哨兵宏将其抑制，避免重复定义。
 *
 * 本头由 compat/mtk_platform_compat.h 统一注入到每个 impl-adapted .c，是集中接线点。
 */
#ifndef _MTK_PLATFORM_BRIDGE_H
#define _MTK_PLATFORM_BRIDGE_H

#include <linux/proc_fs.h>   /* pde_data() */
#include <linux/types.h>     /* dma_addr_t / size_t / bool 供下方前向声明 */
#include <linux/container_of.h> /* from_timer 宏依赖 container_of */

/* ---- 1. PDE_DATA -> pde_data（7.3 已删宏，主线提供函数替身）------------- */
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif

/* ---- 2. PMIC 日志宏（MTK 私有，部分 pmic_info.h 已定义，故守卫）--------- */
#ifndef PMICLOG
#define PMICLOG(fmt, args...) pr_debug("[PMIC]" fmt, ##args)
#endif

/* ---- 3. FOLL_TOUCH 等 7.3 已删除的 gup flag（占位，OR 0 无副作用）------- */
#ifndef FOLL_TOUCH
#define FOLL_TOUCH 0
#endif

/* ---- 4. PMIC 寄存器 id 占位（构建期生成头；目标树真实映射覆盖）----------- */
#ifndef PMIC_RG_VCORE_FPWM
#define PMIC_RG_VCORE_FPWM 0
#endif
#ifndef PMIC_RG_VPROC_FPWM
#define PMIC_RG_VPROC_FPWM 0
#endif
#ifndef PMIC_AUXADC_LBAT_EN_MAX
#define PMIC_AUXADC_LBAT_EN_MAX 0
#endif
#ifndef PMIC_AUXADC_LBAT_EN_MIN
#define PMIC_AUXADC_LBAT_EN_MIN 0
#endif
#ifndef PMIC_AUXADC_LBAT_DEBT_MAX
#define PMIC_AUXADC_LBAT_DEBT_MAX 0
#endif
#ifndef PMIC_AUXADC_LBAT_DEBT_MIN
#define PMIC_AUXADC_LBAT_DEBT_MIN 0
#endif
#ifndef PMIC_AUXADC_LBAT_DET_PRD_15_0
#define PMIC_AUXADC_LBAT_DET_PRD_15_0 0
#endif
#ifndef PMIC_AUXADC_LBAT_DET_PRD_19_16
#define PMIC_AUXADC_LBAT_DET_PRD_19_16 0
#endif
#ifndef MT6357_BUCK_VPROC_OP_EN
#define MT6357_BUCK_VPROC_OP_EN 0
#endif
#ifndef MT6357_BUCK_VCORE_OP_EN
#define MT6357_BUCK_VCORE_OP_EN 0
#endif
#ifndef MT6357_BUCK_VMODEM_OP_EN
#define MT6357_BUCK_VMODEM_OP_EN 0
#endif
#ifndef MT6357_BUCK_VS1_OP_EN
#define MT6357_BUCK_VS1_OP_EN 0
#endif

/* ---- 5. SPM 资源请求常量（MTK 平台私有）------------------------------- */
#ifndef NF_SPM_RESOURCE
#define NF_SPM_RESOURCE 64
#endif
#ifndef NF_SPM_RESOURCE_USER
#define NF_SPM_RESOURCE_USER 64
#endif
#ifndef NF_SPM_USER_USAGE_STRUCT
#define NF_SPM_USER_USAGE_STRUCT ((NF_SPM_RESOURCE_USER + 31) / 32)
#endif
#ifndef SPM_RESOURCE_ALL
#define SPM_RESOURCE_ALL 0xFFFFFFFFU
#endif

/* EMIMPU SMC 调用 id（MTK 私有 SIP 功能号；占位，目标树真实定义覆盖）*/
#ifndef MTK_SIP_EMIMPU_CONTROL
#define MTK_SIP_EMIMPU_CONTROL 0xC2000501
#endif

/* SPM console req 类型（MTK 平台私有；仅有 name / bit_number 两字段被使用）*/
#ifndef _MTK_SPM_RESOURCE_CONSOLE_REQ_DEFINED
#define _MTK_SPM_RESOURCE_CONSOLE_REQ_DEFINED
struct spm_resource_console_req {
	const char *name;
	int bit_number;
};
#endif

/* ---- 6. PMIC LP 枚举（MTK 平台私有）----------------------------------- */
#ifndef _MTK_PMU_LP_TABLE_ENUM_DEFINED
#define _MTK_PMU_LP_TABLE_ENUM_DEFINED
enum PMU_LP_TABLE_ENUM {
	PMU_LP_TABLE_ENTRY_FIRST = 0,
	PMU_LP_TABLE_ENTRY_MAX,
};
#endif

#ifndef _MTK_BUCK_LDO_EN_USER_DEFINED
#define _MTK_BUCK_LDO_EN_USER_DEFINED
enum BUCK_LDO_EN_USER {
	SW = 0,
	SPM,
	VPROC,
	VCORE,
	VMODEM,
	VS1,
	VS2,
	VPA,
	VSRAM_PROC,
	VSRAM_GPU,
	BUCK_LDO_EN_USER_MAX,
};
#endif

/* ---- 7. freqhopping 私有宏 / 枚举 / 类型 -------------------------------- */
#ifndef FH_MSG
#define FH_MSG(fmt, args...) pr_debug("[FH]" fmt, ##args)
#endif

/* FH_CMD_* 仅用于命令区间比较，数值只需互异即可（构建期占位）。
 * 注意：FH_DCTL_CMD_ID / FH_DCTL_CMD_MAX 故意不在此处定义 —— 它们由真实
 * freqhopping 头（mtk_freqhopping_drv.h 的 enum FH_DEVCTL_CMD_ID）以枚举量
 * 提供。早期本桥接层曾用宏占位为 200/300，导致 mtk_freqhopping_drv.h:49/67
 * 的枚举成员被宏展开成 "200 = 0x1000" 触发 "expected identifier"。自包含后
 * 应直接复用真实头里的枚举值，故此处不再定义。 */
#ifndef FH_CMD_ENABLE
#define FH_CMD_ENABLE 0
#define FH_CMD_DISABLE 1
#define FH_CMD_INTERNAL_MAX_CMD 100
#endif

/* struct freqhopping_ssc / struct freqhopping_ioctl 由真实 MTK 头
 * （如 drivers/misc/mediatek/freqhopping/mt6873/mtk_freqhopping.h）完整定义，
 * 本桥接层不再重复定义，避免「redefinition」。如需在没有真实头的环境下占位，
 * 可在此处自行定义并 #define _MTK_FREQHOPPING_SSC_DEFINED 抑制真实头。 */
#ifndef _MTK_FREQHOPPING_SSC_DEFINED
/* 仅前向声明，供下方 struct fh_pll_t 的指针字段使用；具体成员由真实头提供。 */
struct freqhopping_ssc;
#endif

/* struct fh_pll_t 仅被用作指针/数组元素（g_fh_drv_pll），本包提供最小字段集
 * 占位；字段仅取 .c 实际访问的 fh_status/pll_status/setting_id/curr_freq/
 * user_defined。真实 mt_fh_hal_driver.fh_pll 即指向此类型。若目标树真实
 * freqhopping 头已定义完整结构，请 #define _MTK_FH_PLL_T_DEFINED 抑制本定义。 */
#ifndef _MTK_FH_PLL_T_DEFINED
#define _MTK_FH_PLL_T_DEFINED
struct fh_pll_t {
	unsigned int fh_status;
	unsigned int pll_status;
	unsigned int setting_id;
	unsigned int curr_freq;
	unsigned int user_defined;
	struct freqhopping_ssc *ssc_setting; /* 未在本包 .c 中使用，仅占位 */
};
#endif

/* FH_PLL_* / FH_FH_* 为 freqhopping 私有使能状态枚举（构建期占位，数值互异即可）*/
#ifndef FH_PLL_ENABLE
#define FH_PLL_ENABLE  1
#define FH_PLL_DISABLE 0
#define FH_FH_DISABLE  0
#endif

/* ---- 8. 平台 / HW 函数前向声明 ----------------------------------------
 * 这些函数无主线等价物，真实实现来自目标树已修平台基线层或板级代码。
 * 本包在 solution/platform-bridge.c 提供 __weak + EXPORT_SYMBOL_GPL 安全兜底，
 * 使路径 B 驱动可在缺真实实现时仍能通过语法/链接；若有真实实现（强符号），
 * 链接器自动优先采用真实版本。 */
struct mm_struct;
struct cpumask;
struct device;
extern void arch_get_cluster_cpus(struct cpumask *dst, int cluster);
extern const char *m4u_get_port_name(int port);
extern void smp_inner_dcache_flush_all(void);
extern void outer_flush_all(void);
extern void show_pte(unsigned long addr);
extern int pmic_config_interface_nolock(unsigned short addr, unsigned short val,
					unsigned short mask, unsigned short shift);
extern int pmic_read_interface_nolock(unsigned short addr, unsigned int *val,
				      unsigned short mask, unsigned short shift);

/* frame_vector 子系统前向声明。7.3 主线把这套 API 收敛到 include/media/
 * frame_vector.h（仅媒体子系统包含），而伪 M4U 驱动并不包含该头，故在此
 * 无条件前向声明，与 solution/platform-bridge.c 的 __weak 占位实现保持同签名。
 * 若目标树真实包含 frame_vector.h，同名 extern 声明不冲突（签名一致）。 */
struct frame_vector;
struct page;
struct frame_vector *frame_vector_create(unsigned int nr_frames);
int get_vaddr_frames(unsigned long start, unsigned int nr_pfns,
		     int write, struct frame_vector *vec);
void put_vaddr_frames(struct frame_vector *vec);
void frame_vector_destroy(struct frame_vector *vec);
int frame_vector_to_pages(struct frame_vector *vec);
struct page **frame_vector_pages(struct frame_vector *vec);

/* ---- 9. PMIC LDO 寄存器 id / LDO 枚举占位（构建期生成头；目标树真实映射覆盖）
 * helio/pmic 驱动里直接引用的 MT6357_LDO_*_OP_EN 与裸 LDO 名（VSRAM_OTHERS 等）
 * 来自 MTK 私有寄存器生成头，任何公开树均无源码；本包按 #ifndef 守卫占位为 0，
 * 目标树真实生成的芯片映射头若提供同名定义，自动让位。 */
#ifndef MT6357_LDO_VSRAM_PROC_OP_EN
#define MT6357_LDO_VSRAM_PROC_OP_EN 0
#endif
#ifndef MT6357_LDO_VSRAM_OTHERS_OP_EN
#define MT6357_LDO_VSRAM_OTHERS_OP_EN 0
#endif
#ifndef MT6357_LDO_VFE28_OP_EN
#define MT6357_LDO_VFE28_OP_EN 0
#endif
#ifndef MT6357_LDO_VXO22_OP_EN
#define MT6357_LDO_VXO22_OP_EN 0
#endif
#ifndef MT6357_LDO_VRF18_OP_EN
#define MT6357_LDO_VRF18_OP_EN 0
#endif
#ifndef MT6357_LDO_VRF12_OP_EN
#define MT6357_LDO_VRF12_OP_EN 0
#endif
#ifndef MT6357_LDO_VEFUSE_OP_EN
#define MT6357_LDO_VEFUSE_OP_EN 0
#endif
#ifndef MT6357_LDO_VCN33_OP_EN
#define MT6357_LDO_VCN33_OP_EN 0
#endif
#ifndef MT6357_LDO_VCN28_OP_EN
#define MT6357_LDO_VCN28_OP_EN 0
#endif
#ifndef VSRAM_OTHERS
#define VSRAM_OTHERS 0
#endif
#ifndef VFE28
#define VFE28 0
#endif
#ifndef VXO22
#define VXO22 0
#endif
#ifndef VRF18
#define VRF18 0
#endif
#ifndef VRF12
#define VRF12 0
#endif
#ifndef VEFUSE
#define VEFUSE 0
#endif
#ifndef VCN33_0
#define VCN33_0 0
#endif
#ifndef VCN33_1
#define VCN33_1 1
#endif
#ifndef VCN28
#define VCN28 0
#endif

/* ---- 9b. PMIC LDO 寄存器 id / LDO 名占位（第二批，pmic_lp_api.c 引用）
 * 同样来自 MTK 私有寄存器生成头，任何公开树均无源码；#ifndef 守卫占位为 0，
 * 目标树真实生成的芯片映射头若提供同名定义，自动让位。 */
#ifndef MT6357_LDO_VCN18_OP_EN
#define MT6357_LDO_VCN18_OP_EN 0
#endif
#ifndef VCN18
#define VCN18 0
#endif
#ifndef MT6357_LDO_VCAMA_OP_EN
#define MT6357_LDO_VCAMA_OP_EN 0
#endif
#ifndef VCAMA
#define VCAMA 0
#endif
#ifndef MT6357_LDO_VCAMD_OP_EN
#define MT6357_LDO_VCAMD_OP_EN 0
#endif
#ifndef VCAMD
#define VCAMD 0
#endif
#ifndef MT6357_LDO_VCAMIO_OP_EN
#define MT6357_LDO_VCAMIO_OP_EN 0
#endif
#ifndef VCAMIO
#define VCAMIO 0
#endif
#ifndef MT6357_LDO_VLDO28_OP_EN
#define MT6357_LDO_VLDO28_OP_EN 0
#endif
#ifndef VLDO28_0
#define VLDO28_0 0
#endif
#ifndef VLDO28_1
#define VLDO28_1 1
#endif
#ifndef MT6357_LDO_VAUX18_OP_EN
#define MT6357_LDO_VAUX18_OP_EN 0
#endif
#ifndef VAUX18
#define VAUX18 0
#endif
#ifndef MT6357_LDO_VAUD28_OP_EN
#define MT6357_LDO_VAUD28_OP_EN 0
#endif
#ifndef VAUD28
#define VAUD28 0
#endif
#ifndef MT6357_LDO_VIO28_OP_EN
#define MT6357_LDO_VIO28_OP_EN 0
#endif
#ifndef VIO28
#define VIO28 0
#endif
#ifndef MT6357_LDO_VIO18_OP_EN
#define MT6357_LDO_VIO18_OP_EN 0
#endif
#ifndef VIO18
#define VIO18 0
#endif

/* ---- 9c. PMIC LDO 寄存器 id / LDO 名占位（第三批，pmic_lp_api.c 引用）
 * VDRAM / VMC / VMCH / VEMC / VSIM1 / VSIM2 / VIBR / VUSB33 / TREF 系列。
 * 同样来自 MTK 私有寄存器生成头，任何公开树均无源码；#ifndef 守卫占位为 0，
 * 目标树真实生成的芯片映射头若提供同名定义，自动让位。 */
#ifndef MT6357_LDO_VDRAM_OP_EN
#define MT6357_LDO_VDRAM_OP_EN 0
#endif
#ifndef VDRAM
#define VDRAM 0
#endif
#ifndef MT6357_LDO_VMC_OP_EN
#define MT6357_LDO_VMC_OP_EN 0
#endif
#ifndef VMC
#define VMC 0
#endif
#ifndef MT6357_LDO_VMCH_OP_EN
#define MT6357_LDO_VMCH_OP_EN 0
#endif
#ifndef VMCH
#define VMCH 0
#endif
#ifndef MT6357_LDO_VEMC_OP_EN
#define MT6357_LDO_VEMC_OP_EN 0
#endif
#ifndef VEMC
#define VEMC 0
#endif
#ifndef MT6357_LDO_VSIM1_OP_EN
#define MT6357_LDO_VSIM1_OP_EN 0
#endif
#ifndef VSIM1
#define VSIM1 0
#endif
#ifndef MT6357_LDO_VSIM2_OP_EN
#define MT6357_LDO_VSIM2_OP_EN 0
#endif
#ifndef VSIM2
#define VSIM2 0
#endif
#ifndef MT6357_LDO_VIBR_OP_EN
#define MT6357_LDO_VIBR_OP_EN 0
#endif
#ifndef VIBR
#define VIBR 0
#endif
#ifndef MT6357_LDO_VUSB33_OP_EN
#define MT6357_LDO_VUSB33_OP_EN 0
#endif
#ifndef VUSB33_0
#define VUSB33_0 0
#endif
#ifndef VUSB33_1
#define VUSB33_1 1
#endif
#ifndef MT6357_LDO_TREF_OP_EN
#define MT6357_LDO_TREF_OP_EN 0
#endif
#ifndef TREF
#define TREF 0
#endif

/* ---- 10. 7.3 已删除 / 改名的内核符号兼容 -------------------------------- */
/* 7.3 主线 timer.h 不再提供 from_timer（上游 5.x 曾有），本包补回标准实现。 */
#ifndef from_timer
#define from_timer(var, callback_timer, timer_fieldname) \
	container_of(callback_timer, typeof(*var), timer_fieldname)
#endif

/* 7.3 主线 string.h 不再声明 strncpy；提供兼容声明以通过
 * -Wimplicit-function-declaration（-Werror）。重复 extern 声明在 C 中合法。 */
#ifndef _MTK_COMPAT_STRNCPY_DECL
#define _MTK_COMPAT_STRNCPY_DECL
extern char *strncpy(char *__restrict __dest, const char *__restrict __src,
		     __kernel_size_t __n);
#endif

/* 7.3 主线 handle_mm_fault 不再返回 VM_FAULT_WRITE（改为内部计算），构建期占位为 0。 */
#ifndef VM_FAULT_WRITE
#define VM_FAULT_WRITE 0
#endif

/* SMC 调用 id，真实定义来自目标树 mtk_sip_svc.h；构建期占位。 */
#ifndef MTK_SIP_CLKBUF_CONTROL
#define MTK_SIP_CLKBUF_CONTROL 0xC2000517
#endif

/* MTK IOMMU 页表位宽配置，真实值来自目标树 .config（通常为 34）；构建期占位。
 * 注意：必须给出数值，因伪代码里以 (CONFIG_MTK_IOMMU_PGTABLE_EXT - 32) 参与移位。 */
#ifndef CONFIG_MTK_IOMMU_PGTABLE_EXT
#define CONFIG_MTK_IOMMU_PGTABLE_EXT 34
#endif

/* M4U 私有页计数宏（构建期占位，等价于标准向上取整）。 */
#ifndef M4U_GET_PAGE_NUM
#define M4U_GET_PAGE_NUM(va, size) \
	((((unsigned long)(size)) + PAGE_SIZE - 1) / PAGE_SIZE)
#endif

/* 7.3 主线已彻底移除 compat_alloc_user_space（in-compat uaccess 旧接口）。
 * 这里仅提供声明 + solution 层 NULL 弱兜底，保证伪 m4u 的 32/64 位 compat
 * ioctl 路径在语法上完整（真实 32 位用户态支持需在目标树重新实现）。 */
#ifndef _MTK_COMPAT_ALLOC_USER_SPACE_DECL
#define _MTK_COMPAT_ALLOC_USER_SPACE_DECL
extern void *compat_alloc_user_space(unsigned long len);
#endif

/* 旧版 IOMMU 域分配标志，7.3 主线已删除；构建期占位为 0（无副作用）。 */
#ifndef IOMMU_ALLOC
#define IOMMU_ALLOC 0
#endif
#ifndef IOMMU_DEALLOC
#define IOMMU_DEALLOC 0
#endif

/* 7.3 主线 get_user_pages 已删除 FOLL_POPULATE / FOLL_MLOCK（并入默认行为）；
 * 构建期占位为 0，保留旧代码 OR 组合的可读性。 */
#ifndef FOLL_POPULATE
#define FOLL_POPULATE 0
#endif
#ifndef FOLL_MLOCK
#define FOLL_MLOCK 0
#endif

/* ---- 11. 平台 / HW 函数前向声明（无主线等价物；solution/platform-bridge.c 兜底）
 * 这些函数真实实现来自目标树已修平台基线层或板级代码；本包提供 __weak 安全兜底，
 * 使路径 B 驱动在缺真实实现时仍能通过语法/链接，强符号优先。 */
struct mt_fh_hal_driver;
extern struct mt_fh_hal_driver *mt_get_fh_hal_drv(void);
extern int arch_get_nr_clusters(void);
extern char *iommu_get_port_name(int port);
extern int mtk_iommu_get_port_id(void *dev);
extern void report_custom_config_port(const char *a, const char *b, int c);
extern int mtk_iommu_get_larb_port_count(int larb);
extern int mtk_iommu_dump_sec_larb(int larb, int larb_port);
extern struct vm_area_struct *find_extend_vma(struct mm_struct *mm,
					     unsigned long addr);

/* ---- 12. helio-dvfsrc / PMIC DVFS 平台私有符号 --------------------------
 * helio-dvfsrc-opp.c 与 helio-dvfsrc-opp-mt67xx.h 引用了一批「仅在具体
 * CONFIG_MACH_* 下才由平台头提供」的常量 / 类型 / 函数。为让路径 B 在不定义
 * 任何 CONFIG_MACH_* 的探针环境（以及任意目标芯片）下也能自包含编译，这里
 * 无条件提供占位。目标树若已通过 helio-dvfsrc-opp.h 的 #if 分支提供同名定义，
 * 因本段均带哨兵守卫，不会重复定义。 */

/* PM_QOS_*_OPP_DEFAULT_VALUE：4.14 的 MTK pm_qos 扩展常量，用作 *_OPP_UNREQ
 * 哨兵。主线无等价物；语义等价于 PM_QOS_DEFAULT_VALUE(-1) 的「无请求」占位。 */
#ifndef PM_QOS_DDR_OPP_DEFAULT_VALUE
#define PM_QOS_DDR_OPP_DEFAULT_VALUE  (-1)
#endif
#ifndef PM_QOS_VCORE_OPP_DEFAULT_VALUE
#define PM_QOS_VCORE_OPP_DEFAULT_VALUE (-1)
#endif
#ifndef PM_QOS_VCORE_DVFS_FORCE_OPP_DEFAULT_VALUE
#define PM_QOS_VCORE_DVFS_FORCE_OPP_DEFAULT_VALUE (-1)
#endif

/* struct opp_profile：平台头仅在 CONFIG_MACH_MT6768 等具体芯片下定义；探针
 * 环境无这些宏，导致 helio-dvfsrc-opp.c:20 的数组元素类型不完整。无条件占位。 */
#ifndef _MTK_OPP_PROFILE_DEFINED
#define _MTK_OPP_PROFILE_DEFINED
struct opp_profile {
	int vcore_uv;
	int ddr_khz;
};
#endif

/* VCORE 电压区间常量（PMIC 步进，构建期占位；数值取自典型 MT6357 映射）。 */
#ifndef VCORE_BASE_UV
#define VCORE_BASE_UV 500000
#endif
#ifndef VCORE_STEP_UV
#define VCORE_STEP_UV 6250
#endif

/* DVFS 平台函数前向声明（真实实现来自 SPM / PMIC 平台代码；弱兜底见下方）。 */
extern int spm_get_dvfs_level(void);
extern bool is_qos_enabled(void);
extern void spm_dvfs_pwrap_cmd(int cmd, int val);
extern int get_pwrap_cmd(int vcore_opp);

/* ---- 13. 内核私有缓存与映射符号（arm64 7.3 主线无公开声明）-------------
 * 伪 M4U 驱动直接引用了一批「ARM32 遗产 / 内核私有」符号。M4U 的缓存同步枚举
 * 与 DMA/CACHE 描述结构已由本包随伪 M4U 驱动提供的本地 pseudo_m4u.h 给出
 * （见 baseline/impl-adapted/.../pseudo_m4u/pseudo_m4u.h），此处不再重复定义，
 * 仅保留 7.3 主线头文件未导出的内核私有函数前向声明，由
 * solution/platform-bridge.c 提供 __weak 占位。 */

/* dmac_map_area / dmac_unmap_area：ARM32 缓存维护别名，arm64 主线无声明。 */
extern void dmac_map_area(const void *addr, size_t size, int dir);
extern void dmac_unmap_area(const void *addr, size_t size, int dir);
/* __dma_flush_area：arm64 内核私有 cache.S 符号，未在任何头导出。 */
extern void __dma_flush_area(const void *addr, size_t size);
/* map_vm_area / unmap_kernel_range：7.3 mm/vmalloc.c 内部符号，头未声明。 */
extern int map_vm_area(struct vm_struct *area, pgprot_t prot,
		       struct page **pages);
extern void unmap_kernel_range(unsigned long addr, unsigned long size);
/* __dma_map_area / __dma_unmap_area：arm64 内核私有 cache.S 符号，未在任何头
 * 导出；伪 M4U 按 4.14 ARM 风格以 (addr, size, dir) 三参调用，提供弱兜底。 */
extern void __dma_map_area(const void *addr, size_t size, int dir);
extern void __dma_unmap_area(const void *addr, size_t size, int dir);
/* outer_clean_all：ARM32 外存缓存全清别名，arm64 主线无声明（outer_flush_all
 * 见上方平台函数段）。伪 M4U 驱动在 M4U_CACHE_CLEAN_ALL 分支调用，提供弱兜底。 */
extern void outer_clean_all(void);

/* register_syscore_ops：7.3 已删的旧 API，本包在 solution/platform-bridge.c
 * 提供兼容替身（内部包成新的 struct syscore + register_syscore）。前向声明
 * 供路径 B 的 clkbuf 等驱动调用；struct syscore_ops 由主线 syscore_ops.h 提供。 */
struct syscore_ops;
extern void register_syscore_ops(const struct syscore_ops *ops);

/* ---- 14. MTK IOMMU 平台私有函数前向声明 -------------------------------- */
extern int mtk_iommu_get_boundary_id(struct device *dev);
extern int mtk_iommu_switch_acp(struct device *dev, unsigned long iova,
				unsigned long size, int is_acp);
extern void mtk_iommu_iova_to_pa(struct device *dev, unsigned long iova,
				 unsigned long *pa);
extern void mtk_iommu_dump_iova_space(unsigned long target);
extern int mtk_iommu_get_iova_space(struct device *dev, unsigned long *base,
				    unsigned long *max, int *owner, void *priv);
extern void report_custom_iommu_leakage(const char *port_name,
					unsigned int size);
extern void mtk_iommu_trace_log(int event, dma_addr_t dma_addr, size_t size,
				unsigned int flags);
/* mtk_iommu_put_iova_space：与 get_iova_space 配套释放；伪 m4u_v2 在
 * pseudo_put_iova_space / __pseudo_dump_iova_reserved_region 路径引用，签名为
 * (struct device *, struct list_head *)。 */
extern int mtk_iommu_put_iova_space(struct device *dev,
				    struct list_head *list);
/* mtk_dump_* / __mtk_dump_reg_for_hang_issue：IOMMU 调试寄存器 dump 助手；
 * 7.3 主线无等价导出，伪 m4u_v2 在 hang 诊断路径引用，提供 void 弱兜底。 */
extern void mtk_dump_main_tlb(int m4u_id, int mm_id, struct seq_file *s);
extern void mtk_dump_pfh_tlb(int m4u_id, struct seq_file *s);
extern void mtk_dump_victim_tlb(int m4u_id, struct seq_file *s);
extern void __mtk_dump_reg_for_hang_issue(int m4u_id, struct seq_file *s);

/* ARM_MAPPING_ERROR：旧 ARM IOMMU 的映射失败哨兵；7.3 主线改用 DMA_MAPPING_ERROR。
 * 构建期占位为全 1（与历史 ARM_MAPPING_ERROR 语义一致，避免与合法 PA=0 撞车）。 */
#ifndef ARM_MAPPING_ERROR
#define ARM_MAPPING_ERROR (~0ULL)
#endif

#endif /* _MTK_PLATFORM_BRIDGE_H */
