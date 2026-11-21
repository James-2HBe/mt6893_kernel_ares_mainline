# 用 ares-r-oss 补齐 MT6893 平台基线层

> 本轮目标（用户）：「参考 xiaomi-oss 的 `ares-r-oss` 内核源码进行移植 WiFi 驱动，
> 把基线补上」。
>
> 结论：**驱动源码继续用 `seriaTvT/kernel_mt6893`，平台基线层用
> `MiCode/Xiaomi_Kernel_OpenSource@ares-r-oss` 补齐**——两者正好互补。
> 本包因此从「只带 connectivity 私有栈」变成「自带平台头 + 平台构建清单 +
> 链接期符号清单」的**自包含编译期基线**；平台驱动的实现与导出符号仍需目标树提供。

---

## 1. 为什么是 ares-r-oss，为什么只能补「基线」

| 来源 | 性质 | 有什么 | 缺什么 |
|---|---|---|---|
| `seriaTvT/kernel_mt6893 @ chopin_kvm` | 社区裁剪树 | **真 WiFi/connectivity 私有栈**（1570 文件，含 conninfra/WMT/STP/wlan/gen4m/MT6635） | MTK 平台层头与实现（`mt-plat` / `base/power` / `pmic` / `emi` / `devfreq` / `clk` / `eccci` / `mddp`） |
| `MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss` | 小米官方开源完整内核 | **完整 MTK 平台层**（73519 文件） | `connectivity/` 只有 **6 个空壳 adapter**，不含真驱动，不能作驱动源 |

两棵树不是重复，而是互补：上一轮从 seriaTvT 搬驱动，本轮从 ares-r-oss 搬平台层。
`README.md` §1 里「小米官方开源仓不能作源」的结论仍然成立——**它不能作驱动源，
但能作平台基线源**，这两件事此前被混在一起了。

另一个事实：ares-r-oss 是 **4.14** 内核，其平台层源码放进 7.3 同样需要
4.14→7.3 的 API 适配。因此本轮只导出**头文件与构建文件**（编译期与接线依据），
不代做平台驱动实现的适配，见 §7。

---

## 2. 来源与保真校验

| 项 | 值 |
|---|---|
| 仓库 | `MiCode/Xiaomi_Kernel_OpenSource` |
| 分支 | `ares-r-oss` |
| 固定 commit | `49c02c2de185e3e93951fdad6c4dd9caf453523a` |
| 文件清单 | 73519（`git clone --filter=blob:none --no-checkout --depth=1` + `git ls-tree -r`） |
| 导出通道 | `raw.githubusercontent.com/<commit>/<path>`（低并发长连接 + 重试） |
| 完整性 | 逐文件按 git object hash（`sha1("blob <len>\0" + data)`）与 `git ls-tree` 的 blob sha 比对，36/36 一致 |

> **为什么不用 `git cat-file --batch` 惰性拉取**：promisor remote 批量取 blob 时
> 复现了 `GnuTLS recv error (-110): The TLS connection was non-properly terminated`，
> 导致 batch 中途 exit 128、部分文件读不到。改用固定 commit 的 raw blob +
> sha 校验（上一轮验证过的可靠通道）后 182/182 构建文件、36/36 头文件全部一次成功。

逐文件 sha 见 `tools/baseline-manifest.tsv`；完整映射/歧义/未定位清单见
`reports/BASELINE-EXPORT.md`。

---

## 3. 分类原则（决定「补什么」的边界）

静态扫描 `adapted/` 得到 86 个「不在私有栈、也不在 7.3 主线树」的 include，
按性质分三类，**只有第一类从 ares-r-oss 导出**：

| 类别 | 数量 | 处理 |
|---|---|---|
| **PLATFORM** MTK 平台私有头 | 57 | 从 ares-r-oss 保真导出到 `baseline/`，递归解析依赖（BFS 3 轮），最终 36 个文件 |
| **LEGACY** 4.14 主线头 | 10 | **不导出**。`linux/of_gpio.h`、`linux/wakelock.h` 等在 7.3 已删，把 4.14 原文件放进主线 `linux/` 命名空间会与 7.3 类型定义冲突。清单与迁移方向见 `compat/PATH-ALIASES.md` §2 |
| **INTERNAL** 私有栈内部头 | 19（+ BFS 展开后 42） | **不从该仓库补**。ares-r-oss 的 `connectivity/` 是空壳，seriaTvT 裁剪树也没导出；多为条件编译分支或非 MT6893 平台 variant |

递归 BFS 还带出了 4.14 老路径别名（`soc/mediatek/emi.h`）与 dt-bindings 变体头，
处理方式见 §4.2 与 `compat/PATH-ALIASES.md` §3。

---

## 4. 本轮交付物

### 4.1 平台头文件（`baseline/`，36 个，保真）

| 分组 | 代表文件 | 用途 |
|---|---|---|
| EMI / MPU | `include/memory/mediatek/emi.h`、`dramc.h` | `mtk_emimpu_*`（WiFi/conninfra 保留内存保护，调用最多） |
| DVFSRC / OPP | `drivers/devfreq/helio-dvfsrc*.h`（含 mt6893 变体） | `get_cur_vcore_opp` |
| 电源 / PMIC | `base/power/include/{mtk_clkbuf_ctl,mtk_ppm_api,mtk_spm_resource_req}.h`、`pmic/include/{pmic_regulator,pmic_api_buck,pmic_lbat_service}.h` | clkbuf、PPM、SPM、buck、LBAT |
| 性能 / IOMMU | `performance/include/topo_ctrl.h`、`drivers/iommu/mtk_iommu_ext.h` | `topo_ctrl_get_nr_clusters`、`smi_reg_*_sec` |
| 时钟 / DT bindings | `clk/mediatek/clk-mt6885-pg.h`、`include/dt-bindings/{clock,mmc,memory}/*` | 平台宏 |
| CCCI / freqhopping / 调试 | `eccci/{port_ipc,ccci_ipc_task_ID}.h`、`freqhopping/*`、`debug_latch/plat_dbg_info.h` | 私有通道头 |
| 电池 | `power/supply/mediatek/battery/mtk_battery_internal.h` 等 | 低电节流 |

### 4.2 兼容路径别名（`compat/`，非保真，纯转发）

| 文件 | 转发到 | 理由 |
|---|---|---|
| `compat/soc/mediatek/emi.h` | `<memory/mediatek/emi.h>` | 4.14 老路径，被 8 处 conninfra/wlan 平台文件引用 |
| `compat/asm/unaligned.h` | `<linux/unaligned.h>` | 7.3 不再提供 `asm/unaligned.h` |

不造空桩的 LEGACY 头（`mtk-pm-qos.h` / `of_gpio.h` / `wakelock.h` …）
的迁移方向见 `compat/PATH-ALIASES.md` §2。

### 4.3 平台构建文件与链接期清单（`baseline/build-files/`，182 个，保真）

`tools/scan_baseline_modules.py` 导出 15 个平台目录的 Makefile/Kconfig，
解析出 **709 个 kbuild 目标 / 179 个 Kconfig 开关**，并从 `baseline/` 头的
410 个函数声明与 `adapted/` 调用点求交集，得到 **33 个链接期必须提供的平台导出符号**。
调用最多的几个：

| 符号 | 调用次数 | 声明头 |
|---|---|---|
| `mtk_emimpu_set_apc` | 33 | `include/memory/mediatek/emi.h` |
| `mtk_emimpu_init_region` / `set_addr` / `set_protection` / `free_region` | 各 15 | 同上 |
| `get_cur_vcore_opp` | 13 | `drivers/devfreq/helio-dvfsrc-opp.h` |
| `topo_ctrl_get_nr_clusters` | 7 | `performance/include/topo_ctrl.h` |
| `buck_set_mode` | 7 | `pmic/include/pmic_regulator.h` |
| `mt_ppm_sysboost_*` / `spm_resource_req` / `freqhopping_config` / `clk_buf_ctrl` | 1–2 | 各平台头 |

完整表见 `reports/BASELINE-MODULES.md` §2，机器可读表 `tools/baseline-modules.tsv`。

### 4.4 接线（`integration/ccflags.mk`，313 行）

本轮同时**修正了上一轮接线的 3 个 bug**，这些 bug 会让带路径的 include 误报缺失：

1. `-I` 只加「含 `.h` 的最深目录」，少了祖先目录 →
   `#include <mt-plat/sync_write.h>`、`#include <conn_infra/conn_infra_cfg.h>`
   无法命中（头其实就在 `adapted/` 里）。现改为「含头目录 + 直到根的祖先」。
2. `-I$(src)/compat` 路径写错（实际部署位置是
   `$(src)/misc/mediatek/connectivity/compat`）；并把**整个 `compat/`** 复制进
   `adapted/`，别名头才能生效。
3. 缺原厂 Makefile 的 `-I$(srctree)/net`（供 `wireless/core.h`）与
   clang freestanding 内建头（`-nostdinc` 下 `stddef.h` 会误报）。

现在 `ccflags.mk` 一套文件同时给出 `$(src)`（私有栈 + compat）与
`$(BASELINE)`（平台基线层）两组 `-I`，加上 `-I$(srctree)/net` 与 MT6893 编译宏。

---

## 5. 量化验证（真实编译器，同一口径前后对比）

`tools/compile_probe.py`：`clang --target=aarch64-linux-gnu -fsyntax-only`，
7.3.3 真实 KBUILD_CFLAGS，抽样 120 个适配后 `.c`（共 530 个可编译单元），
`--baseline off` 与 `--baseline auto` 唯一差别就是是否 `-I baseline/`。

| 指标 | 补基线前 | 补基线后 | 变化 |
|---|---|---|---|
| 完全干净（0 error） | 70 | **80** | **+10** |
| 仅缺头文件（fatal: file not found） | 23 | **13** | **−10** |
| 平台符号/类型错误（头已找到） | 27 | 27 | — |
| 仍有 7.3 已删除旧 API 残留 | **0** | **0** | — |
| 无「7.3 已删除旧 API」报错 | **120/120** | **120/120** | — |
| 累计 error 行 | 266 | 258 | −8 |

即：**平台基线层让 10 个编译单元从「缺头」变成「完整通过」，
缺头数降 43%，且没有引入任何新的 API 错误**。

> 口径说明：上一版报告的「17 干净 / 103 缺平台头」是**错误归因**——
> 那 103 个其实是上面第 1 个 `-I` bug 造成的「私有栈内部头没命中」。
> 修正口径后是 70 干净 / 23 缺头 / 27 平台符号。
> 「120/120 无 7.3 已删除旧 API 残留」这一核心结论不受影响。

补后剩余 13 个缺头全部有明确归属（见 §7 与 `compat/PATH-ALIASES.md` §3）：
`dt-bindings/mmc/mt6853-msdc.h`(6，非 MT6893 变体)、`rt_config.h`(3，
私有栈内部)、`linux/soc/mediatek/mtk-pm-qos.h`(2)、`linux/of_gpio.h`(1)、
`mach/emi_mpu.h`(1)——**没有一个是「ares-r-oss 能补但没补」的平台头**。

---

## 6. 复现命令

```bash
cd wifi-vendor-port

# 1) 稀疏克隆 ares-r-oss（只需 tree，不取全部 blob）
git clone --filter=blob:none --no-checkout --depth=1 --single-branch \
    --branch ares-r-oss https://github.com/MiCode/Xiaomi_Kernel_OpenSource.git \
    /tmp/ares-r-oss
git -C /tmp/ares-r-oss ls-tree -r --name-only HEAD > /tmp/ares_r_oss_files.txt

# 2) 保真导出平台头文件 + 生成 baseline-ccflags.mk + 导出报告
python3.11 tools/build_baseline.py --repo /tmp/ares-r-oss \
    --files /tmp/ares_r_oss_files.txt --kernel /path/to/linux-7.3.3 \
    --adapted adapted --out baseline \
    --report reports/BASELINE-EXPORT.md \
    --ccflags integration/baseline-ccflags.mk

# 3) 导出平台层 Makefile/Kconfig + 模块/链接期符号清单
python3.11 tools/scan_baseline_modules.py --repo /tmp/ares-r-oss \
    --files /tmp/ares_r_oss_files.txt --baseline baseline --adapted adapted \
    --out baseline/build-files --report reports/BASELINE-MODULES.md \
    --tsv tools/baseline-modules.tsv

# 4) 生成统一接线（含 compat 部署、祖先 -I、baseline -I、net 与编译宏）
./integration/gen_ccflags.sh adapted compat integration/ccflags.mk baseline

# 5) 同一口径的补前 / 补后编译探针
python3.11 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
    --adapted adapted --compat compat --baseline off --limit 120 \
    --report reports/COMPILE-PROBE-BEFORE.md
python3.11 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
    --adapted adapted --compat compat --baseline auto --limit 120 \
    --report reports/COMPILE-PROBE.md
```

---

## 7. 边界与仍需你（目标树）做的事

1. **平台驱动实现与导出符号**：本包补的是**编译期基线**（头 + 构建清单 + 接线）。
   全量链接仍需 §4.3 的 33 个平台符号有实现并 `EXPORT_SYMBOL_GPL`；
   若目标树没有，按 `baseline/build-files/` 的定位把对应 `.c` 一并移植，
   而**这些 4.14 平台驱动本身也要做 4.14→7.3 的 API 适配**，本包不代做。
2. **LEGACY 头是真 API 适配**：`of_gpio` / `wakelock` / `earlysuspend` /
   `mtk-pm-qos` / `hie` 等需按 `compat/PATH-ALIASES.md` §2 改写调用点，
   或由目标树「已修好的 WiFi 基线」直接提供这些头与实现。
3. **私有栈内部头**：`rt_config.h`、`conninfra_ext.h` 等 ares-r-oss 不含，
   需从完整原厂 vendor 栈补，或确认其在 MT6893 配置下处于条件编译死分支。
4. **非 MT6893 变体**：`mt6853-*` / `mt6873-*` 等 dt-bindings 在
   `CONFIG_MACH_MT6893` 下不参与编译，不必补。
5. **固件 / NVRAM / 用户态**：固件二进制仍由 `init/` 从原厂 vendor 提取
   （不在包内），NVRAM 校准数据必须来自本机，vendor HAL 通过 `/dev/wmtWifi` 注入。
6. **真机未验证**：全部是源码 / 头文件 / 构建清单 / 编译期语法验证，
   未在 MT6893 实机启动过；本包也**未编入** `../` 那套 7.3.3 的 Image/dtb，
   两者独立，既有构建产物不受影响。
