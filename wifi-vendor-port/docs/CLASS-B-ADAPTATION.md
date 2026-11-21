# B 类框架符号适配与接线指南（stage-5）

面向把本包接入目标 Linux 7.3.3 内核树的开发者。完整背景见
`reports/CLASS-B-PORTING.md`，源码依据见 `docs/GITHUB-SOURCES-CLASS-B.md`。

## 0. 三条路径怎么选

| 路径 | 适用场景 | 编什么 | 结果 |
|---|---|---|---|
| **A（推荐）** | 目标树就是干净的 7.3.3 主线（已有 `clk-mtk` / `mtk-smi` / `mtk_iommu` / `mtk-dvfsrc` / `mtk-pm-domains`） | `solution/framework-symbols.c` + `solution/platform-symbols.c`；**不编** `framework/vendor/` 下的 4.14 私有驱动 | 8 个 B 类/桥接符号 + 3 个 stage-4 符号全部可链接，无重复定义 |
| **B（备选）** | 目标树「已修基线层」确实移植了整套 4.14 私有 clk/iommu/devfreq 驱动 | 路径 A 的 **全部** + 那些 4.14 驱动（需自行完成 4.14→7.3 API 适配） | `__weak` 定义被强符号自动覆盖，`ld.lld -r` 实测无 multiple definition |
| 验证 | 只想复核本包 | `sh solution/verify-class-b.sh <KDIR>` | 5 项检查全绿 |

`framework/vendor/` 与 `framework/upstream/` 是**保真审计副本，默认不参与编译**。
把它们整体 `obj-y +=` 进主线同名目录会重复定义框架结构体与全局符号——这正是
B 类被单独列出的原因。

## 1. 最小接线（路径 A）

```bash
# 在目标树建一个独立目录，避免碰主线 drivers/clk|iommu|devfreq
mkdir -p drivers/misc/mediatek/ares_framework
cp /path/to/wifi-vendor-port/solution/framework-symbols.c   drivers/misc/mediatek/ares_framework/
cp /path/to/wifi-vendor-port/solution/platform-symbols.c    drivers/misc/mediatek/ares_framework/
cp /path/to/wifi-vendor-port/compat/mtk_framework_compat.h  drivers/misc/mediatek/ares_framework/
# framework-symbols.c 顶部 include 由 "../compat/mtk_framework_compat.h" 改为
#   #include "mtk_framework_compat.h"   （同目录）
```

`drivers/misc/mediatek/ares_framework/Makefile`：

```makefile
obj-$(CONFIG_MTK_FRAMEWORK_SYMBOLS)  += framework-symbols.o
obj-$(CONFIG_MTK_PLATFORM_SYMBOLS)   += platform-symbols.o
```

`Kconfig`：直接合入 `framework/Kconfig.fragment` 的两个配置项
（`MTK_FRAMEWORK_SYMBOLS`、`MTK_FRAMEWORK_HAVE_PG_H`），
并把 stage-4 的 `CONFIG_MTK_PLATFORM_SYMBOLS` / `CONFIG_MTK_RAM_CONSOLE`
按目标树实际值设好。详见 `framework/Kconfig.fragment` 与
`framework/Makefile.fragment`。

> `platform-symbols.c` 的 `aee_rr_rec_clk` 段受 `CONFIG_MTK_RAM_CONSOLE`
> 守卫；`ares_defconfig` 实际值为 `=y`，头里是 `extern` 声明、正需要外部实现。

## 2. 可选桥接：让私有回调真正被主线事件驱动

路径 A 的**最小接线已经能链接、能启动**（符号有定义、私有栈调用不会
undefined）。下面三个注入点是「语义完整性」增强，按需接：

### 2.1 power-gating 回调（`register_pg_callback`）

`drivers/pmdomain/mediatek/mtk-pm-domains.c` 的
`scpsys_power_off()` / `scpsys_power_on()`（`:837` / `:696`）：

```c
#include <linux/mtk_framework_compat.h>   /* 或目标树内的相对路径 */

static int scpsys_power_off(struct generic_pm_domain *genpd)
{
        ...
        mtk_pg_notify(pd->id, true);       /* 掉电前 -> before_off */
        return scpsys_power_off_internal(pd);
}

static int scpsys_power_on(struct generic_pm_domain *genpd)
{
        int ret = scpsys_power_on_internal(genpd);
        if (!ret)
                mtk_pg_notify(pd->id, false);  /* 上电后 -> after_on */
        return ret;
}
```

clk 侧若需要在 rate change 时通知，用主线
`devm_clk_notifier_register()`（`clk-mux.c:438` 的
`devm_mtk_clk_mux_notifier_register` 即其封装）注册一个 notifier，
回调里调 `mtk_pg_notify()`。异常/调试路径调 `mtk_pg_debug_dump()`。

### 2.2 SMI 寄存器备份/恢复（`smi_reg_*_sec`）

**默认无需改主线**：7.3.3 `mtk-smi.c:727-729` 已有

```c
static const struct dev_pm_ops smi_larb_pm_ops = {
        SET_RUNTIME_PM_OPS(mtk_smi_larb_suspend, mtk_smi_larb_resume, NULL)
        SET_LATE_SYSTEM_SLEEP_PM_OPS(pm_runtime_force_suspend,
                                     pm_runtime_force_resume)
};
```

系统休眠时自动完成等价的 larb 寄存器保存/恢复。本包
`smi_reg_backup_sec()` / `smi_reg_restore_sec()` 返回 `0`，表示「成功，
由主线 PM 覆盖」。4.14 的 `KREE_TeeServiceCall(M4U_TZCMD_REG_BACKUP)` TEE
通道在 7.3 不存在，且重复备份会与主线 regmap 冲突，**不要**再实现一遍。

若确需显式串联（例如某个 suspend 顺序要求私有栈先收到通知）：

```c
/* 在目标树 init 里注入后端 */
static int ares_smi_pm_backend(bool backup)
{
        struct device *dev = /* 找到 mediatek,mt6893-smi-larb 设备 */;
        return backup ? pm_runtime_force_suspend(dev)
                      : pm_runtime_force_resume(dev);
}
mtk_framework_set_smi_backend(ares_smi_pm_backend);
```

### 2.3 VCORE OPP（`get_cur_vcore_opp`）

`drivers/soc/mediatek/mtk-dvfsrc.c` 的 OPP 切换路径
（`dvfsrc_get_current_opp()`，`:188`）同步当前值：

```c
mtk_framework_set_vcore_opp(dvfsrc_get_current_opp(dvfsrc)->vcore_opp);
/* QoS 未使能 / 未请求时： */
mtk_framework_set_vcore_opp(VCORE_OPP_UNREQ);
```

未同步时 `get_cur_vcore_opp()` 返回 `VCORE_OPP_UNREQ (-1)`，**保留 4.14 的
哨兵语义**：私有栈会走「未请求」分支，不会误判成 OPP 0。

## 3. 头文件两种形态

`compat/mtk_framework_compat.h` 默认用最小 ABI 兼容定义
（`enum subsys_id` 底层为 int，函数指针调用约定一致，链接期 C 符号不带类型
信息）。若目标树确实把原厂 `drivers/clk/mediatek/clk-mt6873-pg.h` 放进了
include path，定义 `MTK_FRAMEWORK_HAVE_PG_H`（见 `Kconfig.fragment`）即改用
真实的 `enum subsys_id` / `struct pg_callbacks`。

**两种配置都由 `solution/verify-class-b.sh` 步骤 3 实测 0 error。**

## 4. 与既有交付物的关系（不要重复定义）

| 交付物 | 提供哪些符号 | 本轮关系 |
|---|---|---|
| `solution/platform-symbols.c`（stage-4） | `aee_rr_rec_clk`、`aee_rr_curr_clk`、`buck_set_mode` | **同时编入**；`framework-symbols.c` 不重复定义 `aee_rr_rec_clk` |
| `solution/framework-symbols.c`（本轮） | `register_pg_callback`、`smi_reg_backup_sec`、`smi_reg_restore_sec`、`get_cur_vcore_opp` + 4 个桥接入口 | 主实现 |
| `baseline/impl-adapted/`（stage-3，21 个 `.c`） | 31 个 A 类符号；其中 `clk-mt6873-pg.c` / `pseudo_m4u*.c` / `helio-dvfsrc-opp*.c` 也定义本轮 4 个符号 | 路径 A **不编这 3 个**（主线框架已覆盖）；路径 B 编入时，其强符号覆盖 `__weak` |
| `docs/PLATFORM-STUBS.c.template` | 已 `#if 0` 停用 | 不要启用 |

## 5. 接线检查清单

- [ ] `framework-symbols.c` 与 `platform-symbols.c` 都进了 `obj-y`，且只此一份
- [ ] `CONFIG_MTK_FRAMEWORK_SYMBOLS=y`；`CONFIG_MTK_RAM_CONSOLE` 取目标树实际值
- [ ] 路径 A：确认 `framework/vendor/` 下 3 个 4.14 私有驱动**没有**被编入
- [ ] 路径 B：确认 4.14 驱动已完成 4.14→7.3 API 适配，且无强/强重复定义
- [ ] `sh solution/verify-class-b.sh <KDIR>` 五项全绿
- [ ] 内核链接后 `nm vmlinux | grep -E ' (T|W) (register_pg_callback|smi_reg_.*_sec|get_cur_vcore_opp)$'` 能看到定义
- [ ] `module.symvers` / `System.map` 里 4 个符号可见（供 `=m` 的私有栈模块解析）

## 6. 边界（未变）

- 本包交付**源码 / 头 / 脚本 / 配置 / 报告**，不含闭源固件二进制；
  固件仍由 `init/` 从原厂 vendor 提取。
- **真机未验证**：全部是编译期语法验证、`nm` 符号核验与源码级适配。
- 本包是独立素材目录，**未编入**既有 7.3.3 交付内核的 Image/dtb，
  不改变 `../verify.sh` 原有的 dtc / GPU / USB 口径。
- Collabora 的 MT6893 主线 clk 补丁（`clk-mt6893-*.c`）尚未合入任何公开
  GitHub 分支，本包未擅自引入；见 `docs/GITHUB-SOURCES-CLASS-B.md` §3。
