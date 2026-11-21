# MT6893 原厂 WiFi 私有栈 → Linux 7.3.3 主线适配包

> **stage-5（本轮）**：按用户要求移植 **B 类（clk/iommu/devfreq）框架符号**，
> 通过 GitHub 上的开源仓库。新引入 `torvalds/linux@master`（10 文件）作主线
> 框架锚点，补导 `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`
> 的 4.14 私有实现与 MT6893 variant 链（16 文件），**26/26 逐文件 blob-sha
> 校验通过**；B 类 5 个符号全部闭环（本轮新做 4 个 + stage-4 已解决 1 个），
> 以**函数级移植 + 主线语义桥接**落地到 `solution/framework-symbols.c`
> （8 个 `__weak + EXPORT_SYMBOL_GPL` 符号）。7.3.3 头文件实测 `-c` 编译
> **0 error / 0 warning**，`nm` 核验、双配置、`ld.lld -r` 强符号覆盖、
> 编译探针 5 项全绿。详见 **`reports/CLASS-B-PORTING.md`**、
> `reports/CLASS-B-SYMBOLS.md`、`docs/GITHUB-SOURCES-CLASS-B.md`、
> `docs/CLASS-B-ADAPTATION.md`。
>
> **stage-4**：复核并**解决** stage-3 遗留的两个「未开源符号」。
> 结论推翻旧判断：`aee_rr_rec_clk` **有真实开源实现**
> （`ram_console/mtk_ram_console.c:1129`，上轮关键词漏搜该路径），已保真导出
> + 函数级移植到 `solution/platform-symbols.c`；`buck_set_mode` 的 7 次「调用」
> **全是注释**（统计脚本未剥离注释）、且是 MT6335 专用废 API（MT6893 用 MT6359P），
> 真实调用 0，从链接期清单剔除。两符号均以 `__weak + EXPORT_SYMBOL_GPL`
> 落地，经 7.3.3 头文件**实际编译 + nm 符号核验**（3 个 weak 符号、0 error）。
> 详见 **`docs/UNRESOLVED-SYMBOLS.md`**、`reports/SYMBOL-REFS-CORRECTED.md`。
>
> **stage-3**：按用户要求搜 GitHub 上更有利于移植的 ares 内核源，
> 选定 `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`（commit
> `aeb1327b…`），从它**保真导出 21 个平台符号实现 `.c`**、补齐 137 个
> MT6893 变体/依赖头、修 12 个 `EXPORT_SYMBOL_GPL`、生成 2 个无实现符号的
> 桩模板，并用 7.3.3 真实编译探针验证 **API 残留 0、源树能补的头已补尽**。
> 详见 `docs/GITHUB-SOURCES.md` 与 `docs/PLATFORM-IMPL-PORTING.md`。
>
> **stage-5 分工更新**：目标树「WiFi 基线已修过」前提下，本包现在自带
> **编译期基线 + 全部 33 个平台符号的可编译实现与导出宏**，stage-3 遗留的
> 2 个「未开源符号」在 stage-4 解决，**B 类（clk/iommu/devfreq）框架符号
> 在 stage-5 闭环**（`solution/framework-symbols.c`，不整体并入 4.14 私有
> 驱动，而是接到 7.3.3 主线已有的 `clk-mtk` / `mtk-smi` / `mtk_iommu` /
> `mtk-dvfsrc` / `mtk-pm-domains`）。固件二进制仍由 `init/` 从原厂 vendor
> 提取、不在包内；真机未验证。
>
> **stage-2**：按用户要求参考小米官方开源内核
> `MiCode/Xiaomi_Kernel_OpenSource@ares-r-oss` **补齐 MTK 平台基线层**——
> 保真导出 36 个平台头 + 182 个平台层 Makefile/Kconfig，生成 313 行统一接线、
> 33 个链接期平台导出符号清单，并修正上一轮 `-I` 接线的 3 个 bug。
> **详见 `docs/BASELINE-PORTING.md`**。
>
> **stage-1**：把原厂 4.14/GKI-5.x 的 WiFi/connectivity 私有栈直接做 API 适配到
> 7.3.3、私有 DT 节点重写成主线格式、给出 init 从原厂 vendor 提取固件的脚本。
> 详见 `docs/MAINLINE-ADAPTATION.md`。
>
> 分工前提（用户）：目标 7.3.3 内核「**WiFi 基线已修过**」。本包现在自带
> **编译期基线 + 全部平台符号的可编译实现与导出宏**；stage-3 遗留的 2 个
> 「未开源符号」已在 stage-4 复核解决（见 `solution/` 与
> `docs/UNRESOLVED-SYMBOLS.md`），空桩模板已停用；**B 类（clk/iommu/devfreq）
> 框架符号已在 stage-5 闭环**（见 `reports/CLASS-B-PORTING.md`）。
> 固件二进制由 init 从原厂 vendor 提取、不在包内。
>
> 关键结论（修正口径后的真实编译探针）：同一 120 抽样，补基线前
> **70 干净 / 23 缺头** → 补基线后 **80 干净 / 13 缺头**；
> **120/120 无「7.3 已删除旧 API」残留**，compat 头编译自测通过，
> 重写 DT 用 dtc 编译通过、数字 phandle 残留 0。

---

## 0. 各轮工作对照

| 项 | stage-0 保真导出 | stage-1 主线适配 | **stage-2 基线补齐（本轮）** |
|---|---|---|---|
| 驱动源码 | `drivers/` 原厂保真（1559 文件） | `adapted/`：529 个 `.c` 注入 compat 头 + 签名改写 | 沿用 `adapted/`，未改驱动逻辑 |
| 平台基线 | 无（裁剪树缺平台层） | 声明「由目标树提供」 | **`baseline/`：36 个 ares-r-oss 平台头 + 182 个 Makefile/Kconfig** |
| 接线 | 只有原厂说明 | `ccflags.mk` 145 个 `-I` | **313 行统一接线**（含祖先目录、`$(BASELINE)`、`-I$(srctree)/net`），修 3 个 bug |
| 链接期 | 未量化 | 未量化 | **`reports/BASELINE-MODULES.md`：709 模块 / 179 开关 / 33 个平台导出符号** |
| 兼容头 | — | `compat/mtk_wifi_compat.h` | 增 2 个 4.14 老路径别名 + `PATH-ALIASES.md` 迁移清单 |
| 验证 | blob-sha 校验 | 编译探针（口径有 `-I` bug） | **修正口径后同条件前后对比：干净 70→80、缺头 23→13、API 0** |

**stage-3（本轮）新增一列（表格未展开）**：从 `lineage-23.2` 导出
`baseline/impl/`（21 个保真实现 `.c`）与 `baseline/impl-adapted/`（适配版），
`baseline/platform/`（137 个变体/依赖头）、`compat/mtk_platform_compat.h`、
`docs/PLATFORM-STUBS.c.template`；33 个平台符号由「仅清单」推进到
**31 个有实现与导出宏**（19 原有 + 12 新补），2 个给桩模板。
工具链：`port_platform_impl.py` → `complete_impl_headers.py` →
`finish_impl_headers.py` → `adapt_platform_impl.py` → `compile_probe.py`。
完整报告见 `reports/PLATFORM-IMPL.md` / `IMPL-HEADERS.md` /
`API-ADAPTATION-IMPL.md` / `IMPL-PROBE*.md`。

`drivers/`（原厂保真件）保持不动，作为审计基准；改动全部落在 `adapted/` 与新增目录。

---

## 1. 来源与版本（可复现，stage-0 背景）

| 项 | 值 |
|---|---|
| 原厂仓库 | `seriaTvT/kernel_mt6893`（用户最初指定的仓库） |
| 分支 | `chopin_kvm`（默认分支，chopin / ares 通用，带 KVM） |
| commit | `c651f31ec9ab362f54076d5e99ba5dd1f6582a3d` |
| 内核版本线 | 4.14（Android Q/R MTK vendor kernel），**社区已用 KERNEL_VERSION 宏适配到 5.15** |
| 导出方式 | `raw.githubusercontent.com` 按固定 commit 逐文件保真下载（1559 文件 / 48MB，blob-sha 校验 OK=1570） |

### 平台基线来源（stage-2 新增）

| 项 | 值 |
|---|---|
| 仓库 / 分支 | `MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss` |
| 固定 commit | `49c02c2de185e3e93951fdad6c4dd9caf453523a` |
| 文件清单 | 73519（`--filter=blob:none` 稀疏克隆 + `git ls-tree -r`） |
| 导出通道 | raw blob + `git ls-tree` blob sha 逐文件校验（36/36、182/182 一致） |
| 范围 | 只取 MTK 平台私有头与 Makefile/Kconfig；**不取** 4.14 的 `linux/*` 主线头 |

> 更正 stage-1 的一句结论：该仓库的 `connectivity/` 只有 6 个空壳 adapter，
> **不含真驱动、不能作驱动源**；但它是完整 MTK 平台树，**能作平台基线源**。
> 两者此前被混为一谈。详见 `docs/BASELINE-PORTING.md` §1。

## 2. 芯片与栈的事实

- 板载 WiFi/BT/FM/GPS 是 **MT6635 combo 芯片**，挂在 **connsys** 上。
  `ares_defconfig`：`CONFIG_MTK_COMBO_CHIP_CONSYS_6893=y` +
  `CONFIG_MTK_COMBO_WIFI=y` + `CONFIG_WLAN_DRV_BUILD_IN=y`。
- WiFi 数据通路接 `cfg80211`，控制面/固件下载走 **WMT / STP / conninfra** 私有栈。
  MT6893 的 WiFi host interface 是 **AXI / conninfra**（`CONFIG_MTK_COMBO_WIFI_HIF=axi`，
  Connac2x2 SOC3_0），**不是 SDIO**；`ComboA`/`msdc1` 仅作参考。
  详见 `config/Kconfig-Makefile-notes.md`。
- 私有 `compatible`（`mediatek,mt6893-consys` / `mediatek,wifi`）主线无 binding，
  消费方是移植过来的私有驱动；纯主线下板载原生 WiFi 不可用的根因见
  `../docs/NATIVE-NETWORKING.md`（结论不变，本包是"放宽边界搬私有栈"后的素材）。

## 3. 目录结构

```
wifi-vendor-port/
├── README.md                          本文件
├── docs/
│   ├── BASELINE-PORTING.md            stage-2：ares-r-oss 补基线完整说明
│   ├── MAINLINE-ADAPTATION.md         stage-1：API 适配 / 节点重写 / 固件
│   ├── GITHUB-SOURCES.md              stage-3：GitHub 仓库/分支调研与选型
│   ├── PLATFORM-IMPL-PORTING.md       stage-3：实现移植 / API 适配 / 边界
│   ├── UNRESOLVED-SYMBOLS.md          ★stage-4：两个「未开源符号」复核与解决
│   └── PLATFORM-STUBS.c.template      stage-3 桩 → stage-4 已停用（#if 0）
├── PORTING-GUIDE.md                   依赖图 / 适配点清单（stage-0 背景）
├── drivers/                           原厂保真源码（1559 文件，审计基准，勿改）
├── adapted/                           stage-1：适配后全量源码（529 文件有差异）
├── solution/                          ★stage-4：两个符号的正式解决实现
│   ├── platform-symbols.c             __weak + EXPORT_SYMBOL_GPL 自包含实现
│   └── verify.sh                      7.3.3 实编译 + nm 核验 + 双配置验证
├── baseline/                          平台基线（stage-2 头 + stage-3 实现 + stage-4 保真副本）
│   ├── include/ ...                   stage-2：36 个 ares-r-oss 保真平台头
│   ├── build-files/                   stage-2：182 个 Makefile/Kconfig
│   ├── impl/                          stage-3：21 个保真平台符号实现 .c（勿改）
│   ├── impl-adapted/                  stage-3：21 个适配版（补 EXPORT_SYMBOL_GPL）
│   ├── platform/                      stage-3：137 个变体/依赖头（原路径 + flat/）
│   ├── reference/                     ★stage-4：ram_console 真实实现保真副本（7 文件）
│   └── (逐文件 sha: tools/*-manifest.tsv)
├── compat/
│   ├── mtk_wifi_compat.h              stage-1：API 兼容头
│   ├── mtk_platform_compat.h          ★stage-3：平台实现侧最小兼容接线头
│   ├── compat_selftest.c              stage-1：7.3.3 头文件编译自测
│   ├── soc/mediatek/emi.h             ★stage-2：4.14 老路径别名
│   ├── asm/unaligned.h                ★stage-2：4.14 老路径别名
│   └── PATH-ALIASES.md                ★stage-2：别名 + LEGACY 头迁移方向 + 剩余缺口归因
├── dts/                               wifi-private-nodes / wifi-mainline / original
├── integration/
│   ├── gen_ccflags.sh                 生成统一 -I（含祖先目录 + compat 部署 + baseline）
│   ├── ccflags.mk                     ★stage-2：313 行接线结果
│   └── baseline-ccflags.mk            baseline 独立 -I 清单
├── init/                              extract-wifi-firmware.sh / init.wifi.rc
├── config/                            defconfig 片段 + 接线说明
├── firmware/FIRMWARE-LIST.md          固件名清单（源码常量反查）
├── tools/
│   ├── adapt_api.py / compile_probe.py / dts_verify.sh / compat_compile_test.sh
│   ├── build_baseline.py              stage-2：平台头 BFS 保真导出器
│   ├── scan_baseline_modules.py       stage-2：平台模块 / 链接期符号扫描
│   ├── port_platform_impl.py          ★stage-3：下载实现 .c + 定位 33 符号
│   ├── complete_impl_headers.py       ★stage-3：BFS 补实现依赖头
│   ├── finish_impl_headers.py         ★stage-3：定点补多 variant 歧义头
│   ├── adapt_platform_impl.py         stage-3：API 适配 + 补导出宏 + 桩
│   ├── platform-impl-manifest.tsv     stage-3：逐文件 blob sha
│   ├── export_reference.py            ★stage-4：ram_console 真实实现保真导出器
│   ├── audit_symbol_refs.py           ★stage-4：剥离注释的调用计数复核
│   ├── reference-manifest.tsv         ★stage-4：保真副本逐文件 blob sha
│   ├── baseline-manifest.tsv / baseline-modules.tsv
│   └── MANIFEST.txt                   stage-0 逐文件 blob-sha 清单
└── reports/
    ├── BASELINE-EXPORT.md             stage-2：include 分类 / 映射 / 歧义 / 未定位
    ├── BASELINE-MODULES.md            stage-2：709 模块 + 33 符号（末尾含 stage-4 勘误）
    ├── COMPILE-PROBE-BEFORE.md        stage-2：补基线前（同口径对照）
    ├── COMPILE-PROBE.md               stage-2：补基线后
    ├── PLATFORM-IMPL.md               stage-3：33 符号落点（末尾含 stage-4 勘误）
    ├── IMPL-HEADERS.md                stage-3：BFS 补头 / 定点补齐 / 归因
    ├── API-ADAPTATION-IMPL.md         stage-3：适配命中 / 导出宏 / 符号状态
    ├── IMPL-PROBE-BEFORE.md           stage-3：实现补前探针
    ├── IMPL-PROBE.md                  stage-3：实现补后探针（API 残留 0）
    ├── SYMBOL-REFS-CORRECTED.md       ★stage-4：33 符号计数勘误 + 逐行证据
    ├── SOLUTION-PROBE.md              ★stage-4：solution 编译探针（CLEAN=1）
    └── API-ADAPTATION.md              stage-1：改写规则命中统计
```

## 4. 适配层速览

- **compat 头**（`compat/mtk_wifi_compat.h`，仅 ≥6.1 生效）：补 `set_fs` 家族、
  `struct timeval`/`do_gettimeofday`、`VERIFY_*`、`strlcpy`、`dma_zalloc_coherent`、
  `create_proc_entry` 编译桩。已用 7.3.3 真实头文件 `-fsyntax-only` 验证。
- **adapt 脚本**（`tools/adapt_api.py`，幂等）：改 `class_create(THIS_MODULE,x)`、
  `access_ok(VERIFY_*,p,n)`、`vfs_read/write`、`strlcpy`、`dma_zalloc` 调用点，
  给每个 `.c` 注入 compat 头；排除 uboot/virtual-OS 非内核单元与同名自定义函数。
- **社区已分流、不改**：`timer_setup`/`proc_ops`/`timespec64` 由 108 文件 / 253 处
  `KERNEL_VERSION` 宏接管，7.3 走新分支，盲改会破坏兼容。
- **DT 重写**（`dts/wifi-mainline.dtsi`）：14 节点、数字 phandle 全还原 `&label`、
  裸中断号→`GIC_SPI`、reg 重排版（值不变）；dtc 编译通过、数字残留 0。
- **init 固件**（`init/`）：driver probe 之前从 vendor 提固件到 `/lib/firmware`，
  原名就位、不硬编码 eco/flavor、只读 system 走 `firmware_class.path`。

- **平台基线（stage-2）**：`baseline/` 36 个 ares-r-oss 平台头（EMI/MPU、
  DVFSRC、clkbuf/PPM/SPM、PMIC、topo_ctrl、IOMMU、dt-bindings…）+ 182 个
  平台层 Makefile/Kconfig；`compat/` 2 个纯转发别名；`integration/ccflags.mk`
  统一接线并修正 `-I` 少了祖先层、compat 部署路径、缺 `-I$(srctree)/net` 三个 bug。
- **链接期清单（stage-2）**：`reports/BASELINE-MODULES.md` 给出 33 个
  `adapted/` 真实调用的平台导出符号（`mtk_emimpu_*` / `get_cur_vcore_opp` /
  `topo_ctrl_get_nr_clusters` / `buck_set_mode` …），是全量链接的主要 undefined 来源。
- **不做桩的 LEGACY 头**：`of_gpio` / `wakelock` / `earlysuspend` /
  `mtk-pm-qos` 等不搬 4.14 原文件，迁移方向见 `compat/PATH-ALIASES.md` §2。

完整规则表、命中数、映射表、复现命令见 `docs/MAINLINE-ADAPTATION.md`；
基线补齐全过程见 `docs/BASELINE-PORTING.md`。

## 5. 与既有 7.3.3 交付的关系（重要）

- 本包是**独立素材目录**（含本轮新增的 `baseline/`），**未编译进** `../` 那套
  7.3.3 交付内核的 Image/dtb；现有构建产物不受影响。`../verify.sh` 只校验本包
  存在性与自包含完整性，不改变原有的 dtc/GPU/USB 编译口径。
- 「不硬改」边界下"板载原生 WiFi 不可用"的结论（`../docs/NATIVE-NETWORKING.md`）
  **没有变**；本包是用户放宽边界（搬私有栈 + 做主线适配）后的产物。

## 6. 仍需你（用户/目标树）负责的部分

1. **平台驱动实现与导出符号**：本包已补**编译期基线**（头 + 构建清单 + 接线），
   并补齐**链接期实现**：`baseline/impl[ -adapted]/` 有 21 个保真平台符号实现
   （12 个补了 `EXPORT_SYMBOL_GPL`）、`baseline/platform/` 有 137 个依赖头；
   stage-3 遗留的 2 个「未开源符号」已在 stage-4 复核解决
   （`solution/platform-symbols.c`：`aee_rr_rec_clk` 找到并移植真实实现，
   `buck_set_mode` 判定为废弃死 API 并剔除，见 `docs/UNRESOLVED-SYMBOLS.md`）。
   `reports/BASELINE-MODULES.md` §2 的 33 个平台符号中，`buck_set_mode` 已
   勘误剔除（真实调用 0），有效 32 个。stage-5 又补上 **B 类
   （clk/iommu/devfreq）框架符号** 的 7.3.3 可编译实现
   （`solution/framework-symbols.c` + `framework/` 保真源码），
   按 `docs/CLASS-B-ADAPTATION.md` 接线即可。**A 类里那批 4.14 平台驱动
   本身仍需 4.14→7.3 的 API 适配与全量链接**（路径 B 才编它们）。
2. **LEGACY 头是真 API 适配**：`of_gpio` / `wakelock` / `earlysuspend` /
   `mtk-pm-qos` / `hie` 需按 `compat/PATH-ALIASES.md` §2 改写调用点，
   或由目标树「已修好的 WiFi 基线」直接提供头与实现。
3. **接线落地**：把 `adapted/` 与 `baseline/` 挂进目标树 Kconfig/Makefile，
   直接用 `integration/ccflags.mk`（设置 `$(src)` / `$(BASELINE)`），
   板级 dts `#include "wifi-mainline.dtsi"`。不再需要手工启用注释掉的 `-I`。
4. **固件二进制 + NVRAM**：init 从该机原厂 vendor 提取，版本与 ROM 一致；
   NVRAM 射频校准数据必须来自本机；vendor HAL 通过 `/dev/wmtWifi` 注入。
5. **用户态**：主线 `wpa_supplicant`(nl80211) + vendor HAL。

> 真机未验证。全部是源码 / 头文件 / 构建清单 / 脚本的适配与**编译期语法验证**，
> 没有在 MT6893 实机启动过；全量链接与运行仍需平台驱动实现。

## 7. 本轮（stage-5）复现

```bash
cd wifi-vendor-port
# 【可选·审计复现】重新保真导出两个 GitHub 仓库的 26 个文件（需稀疏克隆 + 联网）
python3.11 tools/export_framework.py --root framework \
    --manifest tools/framework-manifest.tsv --vendor-repo /tmp/mt6893-lin232
# 一键验证：编译 / nm / 双配置 / weak 覆盖 / 编译探针（仅需 7.3.3 内核树，可离线）
sh solution/verify-class-b.sh /path/to/linux-7.3.3
# stage-4 的三个符号（互不影响）
sh solution/verify.sh /path/to/linux-7.3.3
# 顶层全量校验（含 10 项 stage-4 检查）
cd .. && ./verify.sh
```

完整证据链与接线说明见 `reports/CLASS-B-PORTING.md`、
`docs/GITHUB-SOURCES-CLASS-B.md`、`docs/CLASS-B-ADAPTATION.md`；
stage-4 证据链见 `docs/UNRESOLVED-SYMBOLS.md`。
