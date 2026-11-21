# 4.14 → 7.3 头文件路径别名与 API 适配清单

本目录的 `soc/mediatek/emi.h`、`asm/unaligned.h` 是**纯路径转发别名**，
不是从 ares-r-oss 保真导出的文件；它们只解决「同一个头在 4.14 与 7.3 路径不同」，
不引入任何类型或函数声明，因此不会与主线头文件冲突。

部署时 `integration/gen_ccflags.sh` 会把整个 `compat/` 复制到
`adapted/misc/mediatek/connectivity/compat/`，并由 `-I$(src)/compat` 命中。

---

## 1. 已提供别名（安全，纯转发）

| 4.14 / 原厂写法 | 7.3 等价位置 | 处理方式 |
|---|---|---|
| `#include <soc/mediatek/emi.h>` | `<memory/mediatek/emi.h>`（ares-r-oss `include/memory/mediatek/emi.h`） | `compat/soc/mediatek/emi.h` 转发 |
| `#include <asm/unaligned.h>` | `<linux/unaligned.h>` | `compat/asm/unaligned.h` 转发 |

`soc/mediatek/emi.h` 被 `conninfra/platform/*_emi.c`、`wmt` 平台层等 8 处引用；
其声明（`mtk_emimpu_*`）正是 `reports/BASELINE-MODULES.md` §2 中调用次数最高的
平台导出符号，因此这条别名直接关系到链接期能否对上符号。

---

## 2. 明确**不**做桩的 4.14 主线头（属真 API 适配）

下面这些头在 7.3 已删除，把 ares-r-oss 的 4.14 原文件搬进主线的
`linux/` 命名空间会与 7.3 类型定义冲突；造空桩又会掩盖真实调用点差异。
因此本包只列出清单与迁移方向，**由目标树按实际配置改写调用点**。

| 头 | 被引用位置 | 7.3 迁移方向 |
|---|---|---|
| `linux/of_gpio.h` | `msdc_cust.c`、`wmt_plat_alps.c`、`connfem_dt_parser.c`、`btmtk_*` 等（19 处） | `linux/gpio/consumer.h` 的 `devm_gpiod_get*` / `gpiod_*`；7.3 已无 `of_get_named_gpio` |
| `linux/wakelock.h` | `gl_os.h`、`fm_utils.h` | `linux/pm_wakeup.h` 的 `struct wakeup_source` + `__pm_stay_awake`/`__pm_relax` |
| `linux/earlysuspend.h` | `gl_typedef.h`、`platform.c`、`wmt_dev.c` | Android 早期挂起在 7.3 无对应；改 `fb_notifier`/`pm_notifier` 或按配置裁掉 |
| `linux/hie.h` | ComboA `mtk_hw_fde.c` | 主线 inline encryption 走 `blk-crypto`/`linux/blk-crypto.h`，需重写 FDE 逻辑 |
| `linux/soc/mediatek/mtk-pm-qos.h` | `wlan/.../plat_*/plat_priv.c`、`soc3_0.c`（8 处） | `linux/pm_qos.h` 的 `cpu_latency_qos_*`，或 `dev_pm_qos_*`；注意 MTK 私有版是 per-cluster/DDR QoS，语义不等价 |
| `linux/mfd/mt6397/rtc_misc.h` | `wmt_plat_alps.c`、`wmt_detect_pwr.c` | MT6397 私有，7.3 无对应；按实际 PMIC 型号接线 |
| `linux/earlysuspend.h` 等其余 | 见 `reports/BASELINE-EXPORT.md` §4 | — |

> `mtk_pm_qos_add_request / update_request / remove_request / request_active`
> 共 4 个函数被 8 处调用；若目标树「已修好的 WiFi 基线」自带
> `mtk_pm_qos` 实现，直接提供该头即可，无需迁移；否则按上表改写。

---

## 3. 其它无法从 ares-r-oss 补齐的缺失头（原因说明）

| 类型 | 例子 | 原因 / 落点 |
|---|---|---|
| 非 MT6893 平台 variant | `dt-bindings/mmc/mt6853-msdc.h`、`mt6873-msdc.h`、`dt-bindings/memory/mt6873-larb-port.h`、`platform_mt6885.h` | ares-r-oss 只带 `mt6885-*`；这些在 `CONFIG_MACH_MT6893` 下不参与编译，属条件编译死分支 |
| 私有栈内部头 | `rt_config.h`、`conninfra_ext.h`、`gps_dl_config.h`、`wmt_task.h`、`cos_api.h` | ares-r-oss 的 `connectivity/` 只有 6 个空壳 adapter；seriaTvT 裁剪树也未导出。需从完整原厂 vendor 栈补，或由构建期生成 |
| 4.14 老 mt-plat 路径 | `mach/mt_reg_base.h`、`mach/emi_mpu.h`、`mtk_disp_notify.h`、`mtk_hibernate_dpm.h` | ares-r-oss 的 mt-plat 版本已演进，无这些老路径；按目标平台等价头替换 |
| cfg80211 内部头 | `wireless/core.h` | 7.3 仍在 `net/wireless/core.h`，靠 `-I$(srctree)/net` 命中（已加入 `integration/ccflags.mk` 与编译探针口径） |

---

## 4. 边界

别名头只保证**编译期路径可解析**；EMI/MPU、PMIC、clkbuf、dvfsrc 等平台符号的
**实现与导出**仍由目标树平台基线层提供，清单见 `reports/BASELINE-MODULES.md` §2。
真机未验证。
