# 主线 API 适配与接线说明（4.14/GKI-5.x → Linux 7.3.3）

> 本轮工作：把上一轮**保真导出**的原厂 WiFi/connectivity 私有栈，**直接做 API 适配**
> 到 Linux 7.3.3 主线，重写私有 DT 节点，并给出 init 从原厂 vendor 提取固件的落地脚本。
> 分工前提（用户）：目标 7.3.3 内核「WiFi 基线已修过」——即已带 MTK 平台基线层
> （时钟/电源/EMI/固件通道的私有驱动与头文件）；主线适配由此处交付；固件二进制
> 由 init 从原厂 vendor 分区提取，不在本包内。

---

## 1. 一个必须先讲清的事实

原厂 `seriaTvT/kernel_mt6893 @ c651f31e (chopin_kvm)` 这棵树**不是纯 4.14**：
社区为 GKI 用 `KERNEL_VERSION` 宏做过适配，**108 个文件、253 处版本分支，最高到 5.15**。
所以：

- `timer_setup`、`proc_ops`、`timespec64`、`ktime_get_real_ts64` 等**新 API 分支已经写好**，
  7.3.3 的 `LINUX_VERSION_CODE` 下会自动走这些分支；
- 剩下的 4.14 老 API 大量位于 `#if KERNEL_VERSION(4,15,0) > LINUX_VERSION_CODE` 的
  **`#else` 死分支**（如 12 处 `init_timer`），7.3 下根本不编译；
- 真正需要适配的，是**没有被版本宏保护、直接调用的 5.15→7.3 删除/改签名项**。

> 这决定了不能"全局批量替换旧 API"——会破坏双分支兼容，也会误伤死分支。
> 本轮用真实编译器（`clang --target=aarch64-linux-gnu -fsyntax-only`）确认了活跃面。

### 量化结论（真实编译探针 `reports/COMPILE-PROBE.md`）

| 指标 | 结果 |
|---|---|
| 抽样编译单元 | 120 / 共 529 个可编译 `.c` |
| **无「7.3 已删除旧 API」报错** | **120 / 120** |
| 完全 0 error | 17 |
| 仅缺平台私有头/符号 | 103 |
| 仍有 API 残留 | **0** |
| 其它噪声 | 0 |

`compat/compat_selftest.c` 用 7.3.3 真实头文件 `-fsyntax-only` **通过**，
证明 compat 头每个补齐符号与主线类型/签名自洽（`tools/compat_compile_test.sh`）。

---

## 2. 交付物结构

```
wifi-vendor-port/
├── drivers/                 【上一轮】原厂保真源码（1559 文件，48MB，blob-sha 校验过）
├── adapted/                 【本轮】适配后全量源码（保持目录层级，仅 529 个文件有差异）
│                              · 每个 .c 顶部注入 #include "mtk_wifi_compat.h"
│                              · 签名类改动（class_create/access_ok/vfs_*/strlcpy/dma）
├── compat/
│   ├── mtk_wifi_compat.h    【本轮】API 兼容头（补内核已删除符号）
│   └── compat_selftest.c    【本轮】编译自测
├── dts/
│   ├── wifi-private-nodes.dtsi     【上一轮】原文摘录（含 dtb 反编译数字 phandle）
│   ├── wifi-mainline.dtsi          【本轮】主线格式重写（label 化、去数字 phandle）
│   └── wifi-mainline-verify.dts    【本轮】dtc 验证用 stub 顶层
├── integration/
│   ├── gen_ccflags.sh       【本轮】自动生成 kbuild -I 清单
│   ├── ccflags.mk           【本轮】生成结果（145 个 -I + 宏）
│   └── (接线说明见本文 §5)
├── init/
│   ├── extract-wifi-firmware.sh    【本轮】init 从 vendor 提取固件
│   └── init.wifi.rc                【本轮】Android init.rc 片段
├── tools/
│   ├── adapt_api.py                【本轮】幂等 API 改写器
│   ├── compile_probe.py            【本轮】真实编译探针
│   ├── compat_compile_test.sh      【本轮】compat 头自测
│   └── dts_verify.sh               【本轮】DT 重写验证
├── reports/
│   ├── API-ADAPTATION.md           【本轮】改写规则命中统计
│   └── COMPILE-PROBE.md            【本轮】编译探针报告
└── firmware/FIRMWARE-LIST.md       【上一轮】固件名清单（源码常量反查）
```

---

## 3. API 适配：compat 头 + adapt 脚本分工

### 3.1 compat 头补的符号（内核已彻底删除）

`compat/mtk_wifi_compat.h`，仅 `LINUX_VERSION_CODE >= 6.1` 生效。

| 原厂用法 | 7.3.3 现状 | compat 处理 |
|---|---|---|
| `set_fs(get_ds())` / `get_fs()` / `mm_segment_t` / `KERNEL_DS` | 5.18 起全删（arm64 无 addr_limit） | 类型 + 无副作用空壳宏（探针确认 `mm_segment_t` 在 7.3 未声明） |
| `struct timeval` / `do_gettimeofday(&tv)` | timeval 仅剩前向声明（incomplete）；函数删除 | 补全 `struct timeval` 定义 + inline 用 `ktime_get_real_ts64`（`tv_usec = nsec/1000`） |
| `VERIFY_READ` / `VERIFY_WRITE` | 删除，`access_ok(p,n)` 两参数 | 定义空值兜底；调用点由 adapt 脚本改 |
| `strlcpy` | 删除 | `→ strscpy`（源码侧也改，头里兜底） |
| `dma_zalloc_coherent` | 删除 | `→ dma_alloc_coherent` |
| `create_proc_entry` | 3.x 就删了（私有栈 3 处，调试用） | 编译桩返回 NULL；成员赋值由 adapt 标注 |

### 3.2 adapt 脚本改的调用点（`tools/adapt_api.py`，幂等可复跑）

| 规则 | 改写 | 命中文件 / 次数 | 依据 |
|---|---|---|---|
| R1 | `class_create(THIS_MODULE, x)` → `class_create(x)` | 26 / 29 | 7.3: `class_create(const char *name)` |
| R2 | `access_ok(VERIFY_*, p, n)` → `access_ok(p, n)` | 3 / 7 | 7.3 两参数 |
| R3 | `vfs_read/vfs_write(kernbuf)` → `kernel_read/kernel_write` | 7 / 10 | 7.3 无 addr_limit，内核缓冲走 kernel_* |
| R4 | `strlcpy` → `strscpy` | 3 / 5 | 见上 |
| R5 | `dma_zalloc_coherent` → `dma_alloc_coherent` | 2 / 3 | 见上 |
| R7 | 每个 `.c` 注入 compat 头引用 | 529 | 或用 `ccflags -include` |

**误改防护**：`vfs_read(*image, file, 0, *code_len)` 这类 **bootloader 自定义同名函数**
（参数完全不同）会被 R3 负向断言 `(?!\*)` 排除；`uboot_driver/` 等 20 个非内核编译
单元整体跳过，保持逐字节原样。

**不改的部分**（社区已用版本宏分流，7.3 走新分支，盲改会破坏兼容）：
`init_timer`→`timer_setup`（12 处，低版本死分支）、`file_operations`→`proc_ops`（5.6 分流）、
`timespec64` 迁移（BT/wlan 已适配）。

---

## 4. 私有 DT 节点重写（`dts/wifi-mainline.dtsi`）

上一轮 `wifi-private-nodes.dtsi` 是 **dtb 反编译摘录**，含大量 `<0xNN>` 数字 phandle，
不能直接 include。本轮重写为可 include 的主线格式：

- **14 个节点**全部保留：consys / wifi / bt / btif / msdc1 / msdc1_top / 3 个保留内存 / 4 个 EMI 控制器；
- **所有数字 phandle 还原为 `&label`**（映射表写在文件末尾，逐条审计）：
  `<0xed>`→`&consys_reserve_memory`、`<0x108>`→`&main_pmic`、
  `<0x11e/11f/127>`→三个 vcn33 LDO、节点自身 `0x198/199/37` 删除由 dtc 重分配；
- 裸中断号 → `GIC_SPI n IRQ_TYPE_LEVEL_HIGH`（wifi 375/378、bt 376/377），
  `clocks = <&scpsys 1>`；
- `reg` 按 `<0 ADDR 0 SIZE>` 分组重排（**值不变**）；
- 补 `#include` 守卫与"外部 label 依赖清单"（`&scpsys`/`&main_pmic`/`&infracfg_ao` 等，
  由目标树基线层提供）。

验证（`tools/dts_verify.sh`）：cpp + dtc **编译通过**（4361 字节），
反编译后**数字 phandle 残留 0**。3 条 `unique_unit_address` warning 是
`consys/wifi/bt` 共用 `0x18000000` —— 原厂 dtb 真实结构（三个虚拟设备映射到同一
consys 寄存器区），非错误。

> compatible 保持 MTK 私有串（`mediatek,mt6893-consys` / `mediatek,wifi`），
> 因为消费方是**移植过来的私有驱动**；主线无对应 binding，`dtbs_check` 报
> "no schema" 属预期，见 `../docs/NATIVE-NETWORKING.md`。

---

## 5. 接线到目标树（用户「已修基线」的 7.3.3 内核）

> 目标是「移植到主线」，所以接线是给用户的**可复现指引**，不是改我这棵干净树
> （干净树没有 MTK 平台基线层，硬编只会得到链接期一堆未定义平台符号）。

1. **放源码**：把 `adapted/` 作为 `drivers/misc/mediatek/connectivity/` 的主体，
   `compat/` 放到 `connectivity/compat/`。
2. **接 Kconfig/Makefile**：沿用原厂 `connectivity/Kconfig`（`MTK_COMBO` /
   `MTK_COMBO_CHIP_CONSYS_6893` / `MTK_COMBO_WIFI` / `WLAN_DRV_BUILD_IN`），
   把 `integration/ccflags.mk` 追加到 `connectivity/Makefile` 的 `ccflags-y`。
   `ccflags.mk` 由 `integration/gen_ccflags.sh` 自动生成（stage-2 起为 313 行：
   私有栈 `-I`（含祖先目录）+ compat + `$(BASELINE)` 平台基线 `-I` +
   `-I$(srctree)/net` + MT6893 编译宏）。
3. **平台层头（stage-2 已补，不再需要手工启用）**：`baseline/` 已从
   `ares-r-oss` 保真导出 36 个平台头 + 182 个 Makefile/Kconfig；
   设置 `$(BASELINE)` 指向其部署位置即可，详见 `docs/BASELINE-PORTING.md`。
4. **defconfig**：保留 `config/wifi-vendor-defconfig.fragment` 里的开关；主线已有的
   `CFG80211=y` / `FW_LOADER=y` 保持不变。
5. **DT**：板级 dts `#include "wifi-mainline.dtsi"`，按需 `&consys { status="okay"; }`。
6. **编译验证**：`make LLVM=1 ARCH=arm64` —— 全量链接仍需
   `reports/BASELINE-MODULES.md` §2 列出的 33 个平台导出符号有实现，
   本包在语法层已证明无 7.3 API 残留（补基线前后 120/120）。

---

## 6. init 从原厂 vendor 提取固件

`init/extract-wifi-firmware.sh` + `init/init.wifi.rc`：

- `on init` 阶段（驱动 probe **之前**）从 `/vendor/firmware`、`/vendor/firmware_mnt/image`
  等常见路径扫描，把 `WIFI_RAM_CODE_soc3_0*`、`soc3_0_*_hdr.bin`、`WMT.cfg`、
  `WMT_SOC.cfg`、`mt66xx_patch_hdr.bin` 等按**原名**软链/拷贝到 `/lib/firmware`；
- 不硬编码 eco/flavor 组合（那些由真机 vendor 实际文件 + 驱动运行时决定），
  整包原名就位让驱动按自己的候选名命中；
- 只读 system 时退化到 `/vendor/lib/firmware` + cmdline `firmware_class.path=`；
- 触发 `/sys/firmware/uevent` 让早于脚本 probe 的设备重试。

固件名完整依据见 `firmware/FIRMWARE-LIST.md`（源码常量反查，非猜测）。

---

## 7. 边界与未验证项（务必先读）

1. **真机未验证**：全部是源码/DT/脚本/配置文本的适配与编译期语法验证，
   没有在 MT6893 实机上启动过。
2. **全量链接需要平台驱动实现**：stage-2 已补**编译期基线**
   （`baseline/` 36 个平台头 + 182 个构建文件 + 统一接线），但
   `conninfra`/`wmt`/`emi`/`eccci`/`mddp` 依赖的 33 个平台私有导出符号
   （时钟/电源/EMI/固件通道，见 `reports/BASELINE-MODULES.md` §2）仍需目标树
   「已修基线」提供实现与 `EXPORT_SYMBOL_GPL`；本包不含这些平台驱动的实现，
   且它们本身也是 4.14 源码、需另做 API 适配。
3. **固件闭源**：二进制不在包内，由 init 从该机原厂 vendor 提取；版本需与
   原厂 ROM 一致（ECO/flavor/IP set 强绑定），NVRAM 校准数据必须来自本机。
4. **用户态**：主线 `wpa_supplicant`(nl80211) + vendor HAL 的 WMT 字符节点
   (`/dev/wmtWifi`) 注入 NVRAM，二者不在内核移植范围。
5. **不覆盖既有结论**：板载原生 WiFi 在「纯主线、不带私有栈」上不可用
   （`docs/NATIVE-NETWORKING.md`）；本包是"放宽边界搬私有栈 + 做主线适配"后的素材，
   **未编译进本交付的 7.3.3 Image/dtb**，两者是独立的。

---

## 8. 复现命令

```bash
cd wifi-vendor-port
# 1) compat 头自洽（7.3.3 头文件 -fsyntax-only）
./tools/compat_compile_test.sh /path/to/linux-7.3.3
# 2) DT 重写验证（cpp + dtc）
./tools/dts_verify.sh /path/to/linux-7.3.3 dts
# 3) 重新生成适配源码（幂等）
python3 tools/adapt_api.py --src drivers --dst adapted --clean \
        --report reports/API-ADAPTATION.md
# 4) 同一口径的补基线前 / 后编译探针
python3 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
        --adapted adapted --compat compat --baseline off --limit 120 \
        --report reports/COMPILE-PROBE-BEFORE.md
python3 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
        --adapted adapted --compat compat --baseline auto --limit 120 \
        --report reports/COMPILE-PROBE.md
# 5) 重新生成接线 ccflags（含 baseline 与 compat 部署）
./integration/gen_ccflags.sh adapted compat integration/ccflags.mk baseline
```

> stage-2 的 `build_baseline.py` / `scan_baseline_modules.py` 完整复现命令见
> `docs/BASELINE-PORTING.md` §6。

---

## 9. stage-2 对本文的修订记录

| 位置 | 修订 |
|---|---|
| §1 量化结论 | 原「17 干净 / 103 缺平台头」为**错误归因**：`-I` 少一层导致私有栈内部头误报。修正口径后为 70 干净 / 23 缺头 / 27 平台符号；补基线后 80 / 13 / 27。**120/120 无 7.3 已删除旧 API 残留**结论不变 |
| §5 接线 | `ccflags.mk` 145 行 → 313 行；平台层头不再依赖目标树手工启用，改由 `baseline/` 提供 |
| §7 边界 | 平台基线从「完全由目标树提供」改为「本包补编译期基线，实现与导出符号仍由目标树提供」 |
