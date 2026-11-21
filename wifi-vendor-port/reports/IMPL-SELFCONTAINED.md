# 路径 B 21 个驱动 — 244 个符号/签名 error 清零与包内自包含

> 本线：`ares-7.3.3/wifi-vendor-port`（vendoring 线）。
> 目标：把 vendoring 68 个平台头后、被 `fatal` 截断而完整暴露出来的 **244 个
> 符号/签名 error 全部清零**，使 21 个 4.14 私有驱动在**默认探针不挂任何外部
> 平台树**的前提下达到 `21/21 CLEAN、0 error`（真正包内自包含）。

生成时间：2026-11-15　验证工具：`tools/probe_impl_full.py`（clang
`--target=aarch64-linux-gnu -fsyntax-only` + 7.3.3 真实 KBUILD_CFLAGS）

---

## 1. 起点与终点

| 指标 | 起点 | 终点 |
|---|---|---|
| 编译单元 | 21 | 21 |
| 累计 error/fatal | **244** | **0** |
| CLEAN（0 error） | 2 | **21** |
| MISSING_HEADER | 0（上一轮已 vendoring 清零） | 0 |
| 旧 API 残留 | 2（探针粗分桶误标） | 0 |
| 平台符号/类型/其它 | 17 | 0 |

`--extra` 默认值为 `[]`（`tools/probe_impl_full.py` 第 76 行），即**验证过程零外部
目录依赖**；所有头都来自 `baseline/impl-adapted/`（68 个 vendoring 头 + 3 个芯片
头 + 1 个本地 pseudo_m4u 头）、`compat/` 与 7.3.3 内核树本身。

---

## 2. 244 个 error 的根因分类与解法

按 error 文本去重后，244 条归为三大类；实际按「根因 → 解法」分层清零：

### 2.1 复用既有桥接层（消 244 → 83）

| 动作 | 文件 | 说明 |
|---|---|---|
| 落盘 576 行桥接层 | `compat/mtk_platform_bridge.h`（新增） | 复用 `ares-7.3.3-ported` 已验证成果：`PDE_DATA`→`pde_data`、`PMICLOG`、PMIC/SPM 寄存器占位、`NF_SPM_RESOURCE`、`MTK_SIP_EMIMPU_CONTROL`、freqhopping `FH_MSG`/`FH_CMD_*`/`struct fh_pll_t`、PMIC LP 枚举、平台 HW 函数前向声明、`frame_vector` 等，全部 `#ifndef` 守卫 |
| 统一接线 | `compat/mtk_platform_compat.h` 改为 `#include "mtk_platform_bridge.h"` | 21/21 `.c` 首个 include 天然注入点 |
| 落盘弱符号桩 | `solution/platform-bridge.c`（新增，12501 字节） | 链接期 `__weak` 兜底，自身编译 0 error |
| 同步 12 个 7.3 适配版 `.c` | 见下表 | 纯 7.3 API 适配，与「挂不挂平台树」无关 |

同步的 12 个适配改动（合计 193 行，全部为 7.3 API 适配，无平台树依赖）：

- `.remove` 回调 `int` → `void`：`emimpu.c`、`mtk_ppm_main.c`、`mtk_freqhopping_drv.c`、`pseudo_m4u.c`、`pseudo_m4u_v2.c`
- timer：`init_timer`/`timer_list.data`/`void (unsigned long)` → `timer_setup`/`from_timer`/`void (struct timer_list *)`（两个 `upmu_lbat_service*.c`）
- `list_cmp_func_t` 形参加 `const`；删除私有 `list_is_first` 重定义（7.3 主线已提供）
- `mmap_sem` → `mmap_lock`；`get_vaddr_frames`/`get_user_pages`/`fixup_user_fault` 签名对齐
- `BufSize`/`MVA`/`m4u_create_sgtable` 的 `unsigned int` → `unsigned long`
- 补齐「定义前调用」的前向声明（消除 undeclared function）
- 加 `<linux/vmalloc.h>` / `<linux/platform_device.h>` / `<linux/string.h>` / `<linux/timer.h>`
- clkbuf syscore 回调签名 `(void)` → `(void *data)`（7.3 `struct syscore_ops`）

### 2.2 7.3 内核头/类型差异（83 → 76，性质转换的关键一步）

| 根因 | 解法 | 文件 |
|---|---|---|
| `struct timespec` 在 7.3 内核态被 `uapi/linux/time.h` 用 `#ifndef __KERNEL__` 隐藏，battery 私有头仍用旧类型（7 处 incomplete） | 补带 `_STRUCT_TIMESPEC` 守卫的最小内核态定义 | `compat/mtk_platform_compat.h` |
| `iommu_dma_map_sg` 桩声明 5 参、4.14 调用 4 参（2 处） | 桩签名按真实调用改 4 参 | `compat/linux/dma-iommu.h` |
| clkbuf 芯片枚举（先 `XO_NUMBER`/`WCN_*` 等 19 个，补后又暴露 `CLOCK_BUFFER_DISABLE`/`CLK_BUF_OUTPUT_IMPEDANCE_*`/`CLK_BUF_CONTROLS_FOR_DESENSE_*`） | **放弃宏占位打地鼠，改 vendoring 芯片头** | 见 2.3 |
| pseudo_m4u 的 `enum M4U_DMA_DIR`/`M4U_CACHE_SYNC_ENUM` 及枚举值 | 改由本地 framework 版头提供（见 2.4） | 见 2.4 |

> 关键决策：**不定义 `CONFIG_MACH_MT6873`**。无损测试表明定义它会让
> `mtk_clkbuf_ctl.h` / `helio-dvfsrc.h` 去 `#include` 未 vendoring 的芯片专属头，
> 引入 4 个新缺头，并让 3 个芯片 `.c` 走错分支（用「引入缺头」换「消枚举」不划算）。

### 2.3 clkbuf 芯片头 vendoring（解决 57 个枚举 + 打地鼠）

- 把 3 个芯片专属头 `mtk_clkbuf_hw.h`（各 141 行、**零 `#include`、完全自包含**，
  自带 `MTK_CLK_BUF_STATUS` / `MTK_CLK_BUF_OUTPUT_IMPEDANCE` /
  `MTK_CLK_BUF_CONTROLS_FOR_DESENSE` / `enum clk_buf_id` / `enum xo_id` 等全部枚举）
  复制到 `baseline/impl-adapted/.../clkbuf_v1/{mt6873,mt6885,mt6853}/`，与 `.c` 同目录。
- 恢复 `.c` 的 `#include "mtk_clkbuf_hw.h"`：**引号 include 优先命中同目录**，
  彻底避免扁平 `-I` 字典序导致的跨芯片错配。
- 同步移除 `compat/mtk_platform_compat.h` 里那 19 个 clkbuf 枚举宏占位 —— 芯片头里
  这些是 **enum 成员**，宏占位会把 enum 声明替换成数字、直接编译失败。

### 2.4 pseudo_m4u 本地 framework 版头（解决 19 个 incomplete/undeclared）

- 复制 `framework` 版（已对齐 `m4u_create_sgtable`/`m4u_dealloc_mva_sg` 签名）的
  `pseudo_m4u.h`（387 行）到 `baseline/impl-adapted/drivers/misc/mediatek/pseudo_m4u/`
  同目录，引号 include 优先命中，胜出 `-I` 里的 `include/pseudo_m4u.h`。
- 该头提供 `struct M4U_CACHE_STRUCT` / `COMPAT_M4U_CACHE_STRUCT` /
  `M4U_DMA_STRUCT`、`M4U_DMA_MAP_AREA`/`UNMAP_AREA`/`FLUSH_BY_RANGE`、
  `enum M4U_DMA_TYPE`、`m4u_portid_valid` 等。
- 同步移除 `compat/mtk_platform_compat.h` 里补的 M4U enum（否则与本地头**重定义**，
  即最后那 11 个 error）。

### 2.5 upmu 桩签名笔误（解决 57 个 int → unsigned int*）

`baseline/impl-adapted/include/mt-plat/upmu_common.h` 第 145 行桩分支把
`pmic_config_interface` 第二参误写成 `unsigned int *val`（抄了 `pmic_read_interface`
的读指针签名），与正常分支及全部 4.14 调用（传值）不符。改为 `unsigned int val`
（与 `ares-7.3.3-ported` 的修复版一致）—— clkbuf 3 个文件各 19 个
`incompatible integer to pointer` 一次性清零。

---

## 3. 收敛过程（实测）

| 阶段 | 动作 | 累计 error | CLEAN |
|---|---|---|---|
| T0 | 起点 | 244 | 2 |
| T1 | 桥接层 + 12 个适配版 `.c` | 83 | 16 |
| T2 | `timespec` + dma-iommu 4 参 + 首批枚举占位 | 76 | 17 |
| T3 | 芯片头 vendoring（移除宏占位、恢复 include） | 76（枚举性质 → 签名/结构体） | 17 |
| T4 | 本地 `pseudo_m4u.h` + 修 `pmic_config_interface` | 11 | 20 |
| T5 | 移除重复的 M4U enum | **0** | **21** |

---

## 4. 最终资产清单（本轮新增/修改）

| 文件 | 变更 |
|---|---|
| `compat/mtk_platform_bridge.h` | 新增（576 行，复用 ported 桥接层） |
| `solution/platform-bridge.c` | 新增（弱符号桩，自身 0 error） |
| `compat/mtk_platform_compat.h` | 接线 `#include "mtk_platform_bridge.h"` + `struct timespec` 兼容定义 |
| `compat/linux/dma-iommu.h` | `iommu_dma_map_sg` 改 4 参 |
| `baseline/impl-adapted/drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.h` | 新增（本地 framework 版，387 行） |
| `clkbuf_v1/{mt6873,mt6885,mt6853}/mtk_clkbuf_hw.h` | 新增 3 个芯片头（各 141 行） |
| 12 个 `.c` | 同步 7.3 适配版（clkbuf 3 个保留芯片头 include） |
| `include/mt-plat/upmu_common.h` | 修 `pmic_config_interface` 桩签名 |
| `include/`（68 个头）+ 4 个 compat 桩 | 上一轮 vendoring 成果，保留 |

---

## 5. 验证结果（均已实测）

| 项目 | 结果 |
|---|---|
| 默认探针（不带任何参数、`--extra` 为空） | **21/21 CLEAN、0 error** |
| 桥接层 `solution/platform-bridge.c` 自身编译 | **0 error** |
| 默认探针是否挂外部平台树 | **否**（依赖全在 `impl-adapted/` + `compat/` + 内核树） |

复现：

```bash
cd ares-7.3.3/wifi-vendor-port
python3 tools/probe_impl_full.py
# 期望：CLEAN(0 error): 21，累计 error/fatal 行: 0
```

---

## 6. 与 `ares-7.3.3-ported` 路线的关系

两条线互补，成果同源、互不冲突：

| | `ares-7.3.3-ported`（既有交付） | `ares-7.3.3`（本线） |
|---|---|---|
| 21/21 达成方式 | 适配版 `.c` + 桥接层 + 探针默认挂**包内** `baseline/platform` 树（`--extra`） | 适配版 `.c` + 桥接层 + vendoring 头 + 芯片头 + 本地 pseudo_m4u 头 + compat 桩 |
| 是否依赖平台树 | 依赖包内 `baseline/platform`（210+ 文件） | **完全不依赖**（连包内 platform 树都不挂） |
| 21 个驱动所需头 | 从包内平台树解析 | 全部随驱动落在 `impl-adapted/` 内 |

本线把「路径 B 整体移植仍需目标树自行做 API 适配」这个遗留边界进一步收紧为：
**`impl-adapted/` 目录本身即编译自包含**，目标树无需补任何头/宏/符号。

---

## 7. 残余边界（如实说明）

- 这是**编译期语法自包含**（`-fsyntax-only`）。链接期/真机仍需真实平台实现用强
  符号覆盖 `solution/platform-bridge.c` 的 `__weak` 占位；占位枚举/寄存器 id 数值为
  编译期占位，真实目标树提供同名定义时由 `#ifndef` 守卫自动让位。
- 未做真机 Kbuild 接入、链接与启动验证 —— 属原方案职责边界，非本轮缺口。
- `tools/vendor_missing_headers.py`（上一轮 vendoring 自动搬运脚本）保留为可复现
  资产；本线的头已齐备，日常验证只需跑默认探针。
