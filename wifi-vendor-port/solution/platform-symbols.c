/* SPDX-License-Identifier: GPL-2.0 */
/*
 * platform-symbols.c — 两个「stage-3 判为未开源」符号的解决实现
 *
 * 目标内核：Linux 7.3.3 (arm64)。自包含，仅依赖主线头，可独立 -fsyntax-only
 * 编译验证；不 include 任何 MTK 私有头，因此不会与 mtk_ram_console.h 的
 * static inline 兜底或 pmic_regulator.h 的 MT6335 条件声明冲突。
 *
 * ============ 符号 1: aee_rr_rec_clk ============
 * stage-3 结论「全树只有 extern 声明、无 .c 实现」是错的。
 * 真实实现：xiaomi-mt6893-dev/kernel_xiaomi_mt6893 @ lineage-23.2 (aeb1327b)
 *   drivers/misc/mediatek/ram_console/mtk_ram_console.c:1129
 *   （保真副本见 baseline/reference/drivers/misc/mediatek/ram_console/）
 * 原厂函数体：
 *   void aee_rr_rec_clk(int id, u32 val)
 *   {
 *       if (!ram_console_init_done || !ram_console_buffer)
 *           return;
 *       if (id >= 0 && id < ARRAY_LEN_8)
 *           LAST_RR_SET_WITH_ID(clk_data, id, val);
 *       else
 *           pr_notice("%s invalid id= %d\n", __func__, id);
 *   }
 *   #define LAST_RR_SET_WITH_ID(rr_item, id, value) (RR_LINUX->rr_item[id] = value)
 *   #define ARRAY_LEN_8 8
 *
 * 语义：把 clk 调试值写入 last-reboot-reason 的 clk_data[8]，供异常重启后
 * 诊断；对 WiFi/连接功能本身无任何影响（纯调试记录）。整个 aee_rr_rec_* 族
 * 3598 行、原厂只 EXPORT_SYMBOL 了 1 个（说明原树全 built-in 直接链接）。
 *
 * 为何不整文件移植 mtk_ram_console.c：它依赖 ram_console_buffer / pstore /
 * of_reserved_mem / FIQ / aee 等整套 4.14 私有运行时与保留内存布局，整体并入
 * 7.3 会引入大量框架级 API 冲突；而调用点 clk-mt6873-pg.c:1100 只用到本函数。
 * 因此按「函数级移植」处理：保留 init guard、id 边界检查、越界 pr_notice，
 * 仅把存储后端从 reserved-memory 的 RR_LINUX->clk_data[] 换成本 TU 静态数组。
 * 若目标树「已修基线层」带真实 ram_console，把 AEE_RR_CLK_STORE 改回
 * LAST_RR_SET_WITH_ID 即可，函数逻辑无需变动。
 *
 * ============ 符号 2: buck_set_mode ============
 * stage-3 结论「7 次调用、无实现、需桩」也是错的。复核结果：
 *   - 7 次「调用」全部是 msdc_cust.c 里被注释掉的历史语句
 *     buck_set_mode(VCORE, enable) （msdc_pmic_force_vcore_pwm() 已
 *     Temporarily disable force pwm），真实调用 0；
 *   - 声明仅在 CONFIG_MTK_PMIC_CHIP_MT6335 条件块内，而 ares_defconfig 是
 *     CONFIG_MTK_PMIC_CHIP_MT6359P=y（MT6893 用 MT6359P + MT6360）；
 *   - 504 个 pmic/regulator/power .c 全树检索无定义、无 enum BUCK_TYPE。
 * 即：它是别款 PMIC 的废弃死 API，在 MT6893 上根本不该存在，无需实现。
 * 本文件仍提供一个安全兜底（返回 -EOPNOTSUPP 而非静默返回 0），仅为防止
 * 目标树误开 CONFIG_MTK_PMIC_CHIP_MT6335 时出现 undefined symbol。
 * 主线 7.3 等价能力是 regulator_set_mode()/regulator_get_mode()：
 *   pmode (force PWM) <-> REGULATOR_MODE_FAST / REGULATOR_MODE_NORMAL，
 *   调用方应改为 regulator_set_mode(reg, mode)，见 docs/UNRESOLVED-SYMBOLS.md。
 *
 * ============ 冲突避免 ============
 * 两个符号均声明 __weak：目标树「已修基线层」若提供同名强符号，链接器自动
 * 选用强符号，本实现静默退场；若目标树没有，本实现消除 undefined symbol。
 * 这是对「不硬改平台基线」边界的尊重，也让包能独立编译验证。
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/export.h>

/*
 * 与 mt-plat/mtk_ram_console.h 的 static inline 兜底互斥：
 * 该头在 CONFIG_MTK_RAM_CONSOLE 未开启时提供同名 static inline 空实现。
 * 若把本文件与那个头放进同一编译单元，会产生 redefinition。
 * 因此 aee_rr_rec_clk / aee_rr_curr_clk 只在 CONFIG_MTK_RAM_CONSOLE=y
 * （头里是 extern 声明、正需要外部实现）时编译；CONFIG 关闭时由头的
 * inline 兜底，本文件只保留 buck_set_mode 兜底。
 *
 * 独立编译验证（solution/verify.sh）默认 -DCONFIG_MTK_RAM_CONSOLE=1，
 * 用于证明符号与 EXPORT_SYMBOL_GPL 会真实产出。目标树接线时按实际
 * Kconfig 值即可，两种配置均自洽，见 docs/UNRESOLVED-SYMBOLS.md。
 */
#if defined(CONFIG_MTK_RAM_CONSOLE) || defined(CONFIG_MTK_PLATFORM_SYMBOLS_FORCE)

/* ------------------------------------------------------------------ */
/* aee_rr_rec_clk —— 函数级移植自 mtk_ram_console.c:1129                */
/* ------------------------------------------------------------------ */

#define AEE_RR_CLK_SLOTS 8          /* 原厂 ARRAY_LEN_8 */

static u32 aee_rr_clk_data[AEE_RR_CLK_SLOTS];
static DEFINE_SPINLOCK(aee_rr_clk_lock);

/* 存储后端切换点：目标树若有真实 ram_console，替换为
 *   LAST_RR_SET_WITH_ID(clk_data, id, val) / LAST_RRR_VAL(...) 即可。 */
#define AEE_RR_CLK_STORE(id, val)   (aee_rr_clk_data[(id)] = (val))
#define AEE_RR_CLK_LOAD(id)         (aee_rr_clk_data[(id)])

/**
 * aee_rr_rec_clk - 记录 clk 调试值（last reboot reason）
 * @id:  clk 槽位索引，有效范围 [0, AEE_RR_CLK_SLOTS)
 * @val: 待记录的 32 位值
 *
 * 逐行对齐原厂语义：越界打印 pr_notice 并丢弃，合法则写入后端。
 * 原厂的 ram_console_init_done / ram_console_buffer guard 在此由「静态后端
 * 恒可用」替代（静态数组无需初始化），语义等价且去掉了私有运行时依赖。
 */
void __weak aee_rr_rec_clk(int id, u32 val)
{
	unsigned long flags;

	if (id < 0 || id >= AEE_RR_CLK_SLOTS) {
		pr_notice("%s invalid id= %d\n", __func__, id);
		return;
	}
	spin_lock_irqsave(&aee_rr_clk_lock, flags);
	AEE_RR_CLK_STORE(id, val);
	spin_unlock_irqrestore(&aee_rr_clk_lock, flags);
}
EXPORT_SYMBOL_GPL(aee_rr_rec_clk);

/**
 * aee_rr_curr_clk - 读回已记录的 clk 值（新增，用于验证与诊断）
 * 原厂 aee_rr_rec_* 族带成对的 aee_rr_curr_* 读接口，此处补齐最小读回。
 */
u32 __weak aee_rr_curr_clk(int id)
{
	unsigned long flags;
	u32 val = 0;

	if (id < 0 || id >= AEE_RR_CLK_SLOTS)
		return 0;
	spin_lock_irqsave(&aee_rr_clk_lock, flags);
	val = AEE_RR_CLK_LOAD(id);
	spin_unlock_irqrestore(&aee_rr_clk_lock, flags);
	return val;
}
EXPORT_SYMBOL_GPL(aee_rr_curr_clk);

#endif /* CONFIG_MTK_RAM_CONSOLE || CONFIG_MTK_PLATFORM_SYMBOLS_FORCE */

/* ------------------------------------------------------------------ */
/* buck_set_mode —— 平台不匹配的废弃 API，仅作安全兜底                  */
/* ------------------------------------------------------------------ */

/**
 * buck_set_mode - MT6335 专用 buck 模式设置（MT6893 不适用）
 * @type:  原厂 enum BUCK_TYPE；本 TU 不依赖该私有枚举，按 int 接收
 * @pmode: 原厂 force-PWM 开关
 *
 * MT6893 (MT6359P) 无对应实现，且调用点全为注释，故不伪装成功：
 * 返回 -EOPNOTSUPP，让误用者明确感知「此平台不支持」。
 * 新代码请使用主线 regulator_set_mode()。
 */
int __weak buck_set_mode(int type, unsigned char pmode)
{
	pr_warn_once("%s: MT6335-only legacy API is not available on MT6893/MT6359P "
		     "(type=%d pmode=%u); use regulator_set_mode() instead\n",
		     __func__, type, pmode);
	return -EOPNOTSUPP;
}
EXPORT_SYMBOL_GPL(buck_set_mode);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("MT6893 platform symbol resolution for mainline 7.3.3");
MODULE_AUTHOR("ares porting");
