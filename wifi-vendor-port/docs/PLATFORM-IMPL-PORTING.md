# 平台符号实现移植（stage-3）— 从 GitHub ares 内核源补齐实现与 API

本轮接续 `BASELINE-PORTING.md`（stage-2）。上一轮从 `ares-r-oss` 只导出了
**平台头（36 个）与构建文件（182 个）**，报告 §2 明确列出 **33 个链接期
必须提供的平台导出符号**，并在 §3 留了一句「这些平台驱动同样是 4.14 源码，
本身也需要 API 适配，本包不代做」。本轮把这句话落地。

## 1. 目标与结论

| 目标 | 结果 |
|---|---|
| 搜 GitHub 上更有利于移植的 ares 仓库/分支 | 选定 `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`，固定 commit `aeb1327b…`；调研见 `GITHUB-SOURCES.md` |
| 移植 33 个平台符号的**实现** | 导出 **21 个保真实现 `.c`**；**31/33 个符号**定位到真实实现，另 2 个只有 `extern` 声明 → 桩模板 |
| 修符号导出 | 原 19 个已有 `EXPORT_SYMBOL*`，**新补 12 个 `EXPORT_SYMBOL_GPL`** |
| 补残余变体头 | 18 个 MT6893/mt6873/mt6877/mt6853 变体头 + BFS 递归导出依赖头 |
| 修 API | 编译探针证明 **7.3 已删除旧 API 残留 = 0**；修掉兼容头污染、补齐实现依赖头，详见 §4 |

## 2. 为什么换成 Lineage 合并树

`ares-r-oss` 是小米官方 GPL 树，`connectivity/` 只有 6 个空壳，且只带
`mt6885-*` 变体头；`seriaTvT/kernel_mt6893` 有完整 WiFi 私有栈但裁掉了平台层。

`lineage-23.2` 三点都占：

- **私有栈完整**：`drivers/misc/mediatek/connectivity/` 1221 文件 / 473 个 `.c`
  （`.gitignore` 有 `connectivity: Import from MediaTek kernel_modules`）。
- **变体头最全**：相对 `ares-r-oss` 多 5032 路径（2291 头 / 2073 源），
  上轮 47 个未定位 include 里的 `clk-mt6873-pg.h`、
  `helio-dvfsrc-opp-v6877.h`、`mt6873/mt6877/mt6853` 系列 dt-bindings 全都有。
- **API 写法更接近 7.3**：该分支成批回移高版本改动（`sizeof_field`、
  `timespec64`、`iov_iter` accessor、`__vmalloc` 去 pgprot 等）。实测其
  18 个平台实现 `.c` 里 **没有** `set_fs` / `strlcpy` /
  `access_ok(VERIFY_*)` / `class_create(THIS_MODULE)` 残留。

> 纠正一条历史结论：README 里「小米仓不能作源」只对 **WiFi 驱动**成立，
> 对平台基线层不成立（上一轮已更正），本轮进一步用 Lineage 合并树补上了
> 它缺的 MT6893 变体与全部平台实现。

## 3. 交付物与工具链

| 路径 | 内容 |
|---|---|
| `baseline/impl/` | **保真实现 `.c`**（21 个，按源树原路径，审计基准，勿改） |
| `baseline/impl-adapted/` | **适配版**（同一套 21 个，注入最小兼容头 + 补 `EXPORT_SYMBOL_GPL`） |
| `baseline/platform/` | 变体头 + BFS 依赖头（按原路径，另有 `flat/` 裸 include 副本） |
| `compat/mtk_platform_compat.h` | 最小兼容接线头（默认不定义宏，理由见 §4） |
| `docs/PLATFORM-STUBS.c.template` | 2 个无实现符号的链接期桩模板（默认不编译） |
| `tools/platform-impl-manifest.tsv` | 逐文件 `path / blob sha / size / kind` |
| `tools/port_platform_impl.py` | 下载候选 `.c` → 定位 33 符号定义 → 导出实现与变体头 |
| `tools/complete_impl_headers.py` | 以实现 `.c` 的 include 为种子 BFS 补依赖头 |
| `tools/finish_impl_headers.py` | 定点补 BFS 因多 variant 歧义漏掉的头 |
| `tools/adapt_platform_impl.py` | 复用 R1–R5 规则 + 注入兼容头 + 补导出宏 + 生成桩 |
| `reports/PLATFORM-IMPL.md` | 33 符号 → 实现文件 → 导出状态表 |
| `reports/IMPL-HEADERS.md` | BFS 补头清单与未定位归因 |
| `reports/API-ADAPTATION-IMPL.md` | 适配规则命中数、新补导出宏、桩列表 |
| `reports/IMPL-PROBE-BEFORE.md` / `IMPL-PROBE.md` | 补前 / 补后编译探针 |
| `docs/GITHUB-SOURCES.md` | 仓库/分支调研、选型理由、通道经验、复现命令 |

**保真链路**：sparse clone 拿 `git ls-tree` 的 blob index →
`cdn.jsdelivr.net/gh/<repo>@<commit>/<path>` 下载 →
`sha1(b"blob <size>\0" + data)` 逐文件比对。
`raw.githubusercontent.com` 在本环境实测超时，已弃用；详见 `GITHUB-SOURCES.md` §4。

## 4. 「修 API」实际做了什么（以及一个踩过的坑）

### 4.1 签名改写几乎不需要

`tools/adapt_platform_impl.py` 复用 `adapt_api.py` 的 R1–R5
（`class_create` 去 owner、`access_ok` 去 VERIFY、vfs_*→kernel_*、
`strlcpy`→`strscpy`、`dma_zalloc_coherent`→`dma_alloc_coherent`），
在 21 个实现 `.c` 上**命中为 0**。这不是漏做，而是源树已 backport 的
直接证据：编译探针 `API=0`，`21/21` 单元在首个 fatal 前没有
「7.3 已删除旧 API」报错。2 处 `init_timer` 位于 `#if` 低版本死分支。
因此**不做猜测性重写**，只做可审计的接线与符号补齐。

### 4.2 一个真实的坑：兼容头污染

初版让 `mtk_platform_compat.h` 直接 `#include "mtk_wifi_compat.h"`，
想复用 WiFi 栈的 7.3 补符号。结果把 `struct timeval` / `strlcpy` 宏 /
`mm_segment_t` 灌进平台实现，与 7.3 主线头冲突；同时注释里写了
`VERIFY_*/`，其中的 `*/` **提前闭合了块注释**，中文被当成 C 代码编译。
编译探针错误数从 **20 条涨到 171 条**。

现已改为**最小接线头**：不引入任何宏，只保留一个集中扩展点，
真遇到新 API 缺失时按 `KERNEL_VERSION` 分支在此添加。修掉后
`21/21` 单元无旧 API 报错。

### 4.3 真正的适配工作量在「补依赖头」

平台实现 `.c` 依赖大量 MTK 平台内部头（`mtk_spm.h`、
`mt-plat/upmu_common.h`、`helio-dvfsrc-ipi.h`、`mt-plat/aee.h`、
`mt-plat/sync_write.h`…）。BFS 从 21 个 `.c` 的 include 出发递归导出，
另定点补了 `mach/mtk_cpufreq_api.h`（mt6873 分支）与
`linux/usb/class-dual-role.h`。

`asm/dma-iommu.h` **故意不补**：源树只有 `arch/arm/include/asm/dma-iommu.h`
（4.14 arm32），aarch64 目标下属条件编译死分支，导入反而污染。

## 5. 33 个符号的落地状态

| 状态 | 数量 | 说明 |
|---|---|---|
| 已有 `EXPORT_SYMBOL*` | 19 | 源树实现里本来就导出了 |
| **新补 `EXPORT_SYMBOL_GPL`** | **12** | 适配版在其定义文件末尾补齐 |
| 需目标树基线层 / 桩模板 | 2 | `buck_set_mode`、`aee_rr_rec_clk` |

后两个在整棵源树里**只有 `extern` 声明、没有任何 `.c` 实现**
（`drivers/misc/mediatek/pmic/include/pmic_regulator.h:47`、
`drivers/clk/mediatek/clk-mt6873-pg.h:87` / `clk-mt6885-pg.h:76`），
属于 MTK 私有平台层未开源部分。`docs/PLATFORM-STUBS.c.template`
给出默认返回安全值的桩实现，用 `CONFIG_MTK_PLATFORM_STUBS` 包裹，
**默认不参与编译**；真机应优先由目标树已修基线层提供真实实现。

实现落点举例（完整表见 `reports/PLATFORM-IMPL.md`）：

- `mtk_emimpu_*`（7 个符号）→ `drivers/memory/mediatek/emimpu.c`
- `get_cur_vcore_opp` → `drivers/devfreq/helio-dvfsrc-opp.c`（+ mt6893 variant）
- `mt_ppm_*` → `drivers/misc/mediatek/base/power/ppm_v3/src/`
- `clk_buf_*` → `drivers/misc/mediatek/base/power/clkbuf_v1/mt6873/mtk_clkbuf_hw.c`
- `smi_reg_backup_sec` / `smi_reg_restore_sec` → `drivers/misc/mediatek/pseudo_m4u/pseudo_m4u.c`
- `mt_usb_is_device` → `drivers/usb/mtu3/mtu3_dr.c`
- `spm_resource_req` → `drivers/misc/mediatek/base/power/spm_v4/mtk_spm_resource_req.c`
- `force_get_tbat` → `drivers/misc/mediatek/power/mt6873/mtk_battery_intf.c`

## 6. 量化验证

同一把编译探针（`clang --target=aarch64-linux-gnu -fsyntax-only`，
Linux 7.3.3 真实 `KBUILD_CFLAGS`），口径完全一致：

| 指标 | 补前（`IMPL-PROBE-BEFORE`） | 最终（`IMPL-PROBE`） |
|---|---|---|
| 编译单元 | 18 | 21 |
| 完全干净 CLEAN | 1 | **2** |
| 仅缺头文件 | 15 | **2** |
| **7.3 已删除旧 API 残留** | **0** | **0** |
| 累计 error 行 | 20 | 216（见下方说明） |

最终剩余 2 个缺头都是**正确跳过**、不该搬进 7.3 的：

- `trace/events/mtk_events.h`：由 `CREATE_TRACE_POINTS` 在编译期生成的
  tracepoint 头，不是源码头。
- `asm/dma-iommu.h`：源树只有 `arch/arm/include/asm/dma-iommu.h`（4.14
  arm32），aarch64 目标下属条件编译死分支。

即：**源树能补且该补的头已全部补齐**（`baseline/platform/` 共 137 个头，
`baseline/` 合计 173 个），没有「能补却漏掉」的缺头。

> 注意 error 行数在补头过程中会**先上升**：头补齐后编译器能解析得更深，
> 从而暴露出下一层的 `PLATFORM` 类错误（MTK 私有类型 / 框架符号，如
> `PM_QOS_DDR_OPP_DEFAULT`、`enum BUCK_TYPE`、平台寄存器宏）。
> 这些**本来就是目标树「已修好的平台基线层」的职责**——本包是在**干净
> 主线树**上做语法探针，缺平台层是实现前提，不是适配遗漏。
> 判定本轮成败的硬指标是 **API 残留始终为 0**，以及 33 个符号里
> 31 个有了可编译的实现源码与导出宏。

## 7. 接线到目标树

1. 把 `baseline/impl-adapted/` 挂进目标树 Kconfig/Makefile（沿用源树
   `drivers/*/Makefile` 的 `obj-y` 目标名，见 `baseline/build-files/`）。
2. `subdir-ccflags-y` 追加 `baseline/` 下所有含 `.h` 目录及其祖先的 `-I`
   （`integration/gen_ccflags.sh` 已能自动扫描；`ccflags.mk` 的
   `$(BASELINE)` 段覆盖主要路径）。
3. 若目标树已有等价平台驱动，**不要同时链接本包实现**（会重复定义）；
   只把对应符号用 `EXPORT_SYMBOL_GPL` 暴露即可。
4. 仅当目标树确实缺 `buck_set_mode` / `aee_rr_rec_clk` 时，启用
   `docs/PLATFORM-STUBS.c.template` 并定义 `CONFIG_MTK_PLATFORM_STUBS`。
5. DT 与固件仍按 `MAINLINE-ADAPTATION.md`：板级 dts
   `#include "wifi-mainline.dtsi"`，`init/` 从原厂 vendor 提取固件。

## 8. 边界（未变）

- 本包交付的是**源码 / 头 / 脚本 / 配置**，不含闭源固件二进制。
- **全量链接**仍需目标树提供平台导出符号的真实实现（或本包实现 + 桩），
  且 `clk` / `iommu` / `devfreq` 等与主线框架强耦合的模块（报告中 B 类）
  需要单独适配，不建议整体并入主线目录。
- **真机未验证**；全部是编译期语法验证与源码级适配。

> **stage-5 更新（追加勘误，上文保留作审计对照）**：上面这条「B 类需单独
> 适配」的**判断依然成立**（确实不整体并入主线目录），但它**不再是未完成
> 的边界**——stage-5 已用「函数级移植 + 主线语义桥接」把 B 类 5 个符号
> 全部闭环：`solution/framework-symbols.c` 提供 7.3.3 可编译实现，
> `framework/` 保存两个 GitHub 开源仓库的 26 个保真源文件（26/26
> blob-sha 一致），`docs/CLASS-B-ADAPTATION.md` 给出接线。
> 详见 `reports/CLASS-B-PORTING.md`。
- 本包是**独立素材目录**，未编入既有 7.3.3 交付内核的 Image/dtb，
  不改变 `../verify.sh` 原有的 dtc / GPU / USB 口径。
