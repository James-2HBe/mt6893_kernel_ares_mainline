# ares（Redmi K40 游戏增强版 / MT6893）→ Linux 7.3.3 移植成果包

**与原厂分离版**：本目录只包含「移植 / 修复后可直接使用的驱动与文件」，
**不含任何原厂 4.14 数据原件**。原厂数据留在原工程 `/workspace/ares-7.3.3/`，
本包在编译、安装、自检三个环节都**不依赖它们**。

主交付是一份已修复主线契约的板级设备树 + 9 个按 7.3.3 API 重写并真编译过的驱动。

---

## 一、与原厂的边界

| 原厂 / 非交付内容（本包**不含**） | 原工程中的位置 | 为什么不含 |
|---|---|---|
| 41 个小米官方 `ares-r-oss` 原件（dts/dtsi/dws/私有头/defconfig） | `vendor-reference/official/` | 4.14 数据源，不参与 Kbuild，仅作追溯 |
| 29 个 `kernel_mt6893@chopin_kvm` 第二数据源 | `vendor-reference/kernel_mt6893/` | 参数交叉验证用，仍是 4.14 厂商格式 |
| 11 个官方 4.14 私有驱动源码 | `drivers/src/` | 原厂源码，**不是**移植后可用的驱动 |
| 早期 v1/v2 板级 dts/dtb（4 个文件） | 工程根 | 历史中间版本，非最终可用产物 |
| 旧版驱动与 pre-normalize dts | `port/archive/` | 历史归档 |
| 依赖原厂数据的组装/安装脚本 | `tools/assemble_ares_dts*.py`、`tools/build_official_dts.sh`、`tools/verify_dtb*.sh`、`install_official.sh` | 只在「从原厂重新组装」时才需要；本包已含最终产物 |
| 编译中间产物 | `port/*.o`、`*.mod`、`*.cmd`、`__pycache__/` | 非交付物 |

> 唯一带进来的头文件 `mt6893-pinfunc.h` 属于**主线 / 目标仓库**的 pinctrl 宏定义，
> 是编译 dts 的必需件，不是 4.14 私有数据；若你的内核树已有，安装脚本会自动跳过。

---

## 二、目录结构

| 路径 | 内容 |
|---|---|
| `mt6893-xiaomi-ares-official.dts` | **主设备树**，5187 行，含 15 个新增标准 `*-gpios` 属性（dts 中共 17 处，另 2 处为官方面板自带）+ 12 类主线契约修复 |
| `mt6893-xiaomi-ares-official.dtb` | **起机用 dtb**，222,232 字节，含 `__symbols__` |
| `mt6893-pinfunc.h` | 编译 dts 所需的 pinfunc 宏（主线件） |
| `Makefile.patch` | 可选的 `git apply` 补丁，等效于安装脚本的 Makefile 追加 |
| `ares_defconfig.fragment` | 75 个主线 Kconfig 符号配置片段（A 类驱动开关） |
| `ares_build_config.additions` | **KVM / NTSYNC 等额外强制配置项**（见第九章） |
| `ares_builtin_config.additions` | **内建版专属**：6 个父级框架 `=y`（ASoC / 无线 / WWAN / USB 串口），仅 `--builtin` 加载（见第十章） |
| `build_workflow.sh` | **自动构建 workflow**：预检 → 安装 → 强制写 defconfig → 编译 → 收集 → 打包，见第九章 |
| `auto_install.sh` | **一键自动安装总入口**：全部文件一步到位，含幂等 / dry-run / 备份 / 回滚 / 安装后自检 |
| `ci/ares-build.yml` | GitHub Actions 版 workflow 模板（可选） |
| `install.sh` | 细粒度入口：只装 dts + pinfunc 头 + dts Makefile 条目 |
| `verify.sh` | **独立自检脚本**（文件齐全 / 幂等 / dtc 编译 / 契约检查） |
| `port/` | **9 个移植驱动**：`.c` 源码 + `.ko` 产物 + `Kbuild` + `build.sh` + `build.log` |
| `port/compat/` | 2 个 A 类补丁：给主线 `bq25980` / `bq27xxx` 补 ares 的 compatible |
| `port/dt-bindings/` | 6 个设备树补丁（含 mali / mtu3 / xhci 三个 mt6893 binding 补丁）+ GPIO 契约说明（详见下方「补丁用法」） |
| `port/integration/` | 把 9 个驱动接进内核树的 Kconfig / Makefile / defconfig 规则 + `apply.sh` |
| `tools/` | `fix_mainline_contract.py`（dts 契约 + **GPU 节点 / 相机 EEPROM 主线化注入**）、`fix_kconfig_recursion.py`（Kconfig 递归依赖）、`force_defconfig.py`（**强制写入 defconfig**）、`make_initramfs.py`（**initramfs 生成**，见第十一章） |
| `docs/` | 11 份移植 / 修复 / 实机参数 / 覆盖审计文档 |
| `wifi-vendor-port/` | 原厂 4.14 WiFi 私有栈（1559 源码保真）+ `adapted/` 主线 API 适配 + `compat/` 兼容头 + `dts/wifi-mainline.dtsi` 重写节点 + `init/` 固件提取；**stage-2** 从 `ares-r-oss` 补 36 个 MTK 平台头 / 182 个 Makefile/Kconfig / 313 行接线 / 33 个链接期符号清单；**stage-3** 从 `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2` 保真导出 **21 个平台符号实现 `.c` + 137 个变体/依赖头**，补齐 **12 个 `EXPORT_SYMBOL_GPL`**；**stage-4（本轮）** 复核并**解决**遗留的 2 个「未开源符号」：`aee_rr_rec_clk` 找到真实实现（`ram_console/mtk_ram_console.c`）并保真导出 + 函数级移植，`buck_set_mode` 判定为 MT6335 专用废弃死 API（7 次「调用」全是注释、真实调用 0、从清单剔除）；`solution/platform-symbols.c` 以 `__weak + EXPORT_SYMBOL_GPL` 落地，经 7.3.3 实编译 + nm 核验（3 weak 符号 / 0 error）。编译探针 **API 残留 0**，干净 70→80、缺头 23→13；**独立素材，不参与本包 7.3.3 编译**，见其 `README.md` / `docs/UNRESOLVED-SYMBOLS.md` |

### 9 个驱动

| 驱动 | 作用 | 主线 API |
|---|---|---|
| `regulator-wl2866d.c` | 相机 LDO | `regmap` + `devm_regulator_register` |
| `et7480-audioswitch.c` | 音频开关 | `regmap` |
| `leds-aw2028.c` | 呼吸灯 | `led_classdev` |
| `haptic-aw8697.c` | 触觉马达 | `input` / `ff-memless` |
| `input-gamekey.c` | 弹出式肩键 | `input` + `gpiod_to_irq` |
| `st21nfc.c` | NFC | `miscdevice` / `poll` / `fasync` |
| `xmusb350-charger.c` | 充电检测 | `power_supply` |
| `gf3658-spi.c` | 屏下指纹 | SPI + 18 个 `GF_IOC_*` ioctl |
| `touchscreen-gt9886.c` | 触控 | I2C + `0x824E` 帧协议 |

---

## 三、快速开始（一键自动安装）

```bash
# 0) 前置：干净主线 7.3.3 没有 SoC 层 mt6893.dtsi 与 clk-mt6893，
#    需先合入目标仓库 James-2HBe/mt6893_kernel_ares_mainline 已有的 SoC 层。

# 1) 先预演（只打印，不改动任何文件）
./auto_install.sh /path/to/linux-7.3.3 --dry-run

# 2) 正式安装：所有需要的文件一次性到位
./auto_install.sh /path/to/linux-7.3.3

# 可选组合：
#   --build            安装后编译 dtb + 9 个模块
#   --with-artifacts   另把 dtb / .ko / docs 放进内核树 ares-port/ 便于取用
#   --rollback         撤销全部安装，回到首次安装前状态
```

`auto_install.sh` 会自动装好以下全部内容（每步幂等，重复运行安全）：

| 步骤 | 安装内容 |
|---|---|
| 1 | 主 dts、起机 dtb、`mt6893-pinfunc.h`、dts `Makefile` 的 dtb 条目与 `-@` |
| 2 | 9 个驱动 `.c`、9 条子系统 `Makefile` 规则、`drivers/misc/Kconfig` 的 ares 菜单 |
| 3 | 2 个 A 类 compatible 补丁（`bq25980_charger.c` / `bq27xxx_battery_i2c.c`） |
| 4 | 最多 38 条 `CONFIG`（A 类 29 + B 类 9）：先跳过目标 defconfig 里已有的**同值**项（避免重复赋值警告），再以标记块追加；值不同的项按本包覆盖（如 `TYPEC =m→=y`，属预期） |
| 5 | 可选附加产物（`--with-artifacts`） |
| 6 | 10 项安装后自检，任一失败即报错并提示回滚 |

每次真实安装会在 `<内核树>/.ares-port-backup/<时间戳>/` 留快照；`--rollback` 用
**首次安装前**的快照恢复（幂等复跑不会覆盖基准）。脚本只改内核树，不改交付包。

> 新增的 CONFIG 需执行 `make LLVM=1 ARCH=arm64 defconfig` 才会进入 `.config`
> （会重新生成配置，注意备份自定义项）。脚本本身不动 `.config`，避免破坏现有编译环境。

**细粒度入口**（只需装其中一部分时用）：
- `./install.sh <内核树>` — 只装 dts / pinfunc / dts Makefile 条目
- `./port/integration/apply.sh <内核树>` — 只接入 9 个驱动
- `./verify.sh <内核树>` — 验证交付包自洽，不改动内核树

**统一带 `LLVM=1`**：单独用 gcc 编译 dts 会把 `.config` 刷成 gcc，
之后 clang 编外部模块会报 `-fconserve-stack`；`port/build.sh` 带自动恢复逻辑。

### 故障排查：`make defconfig` 报 recursive dependency

```
error: recursive dependency detected!
        symbol REGMAP_I2C is selected by REGULATOR_WL2866D
        symbol REGULATOR_WL2866D depends on REGMAP_I2C
```

**根因**：Kconfig 允许同一个 `config FOO` 在多个 Kconfig 文件里重复定义并
**合并属性**。`mt6893_kernel_ares_mainline` 一类目标树常自带一份
`config REGULATOR_WL2866D ... select REGMAP_I2C`；旧版本包追加的 ares 菜单块
又写了 `depends on I2C && REGMAP_I2C && ...`。两处合并后，该符号同时
「select REGMAP_I2C」和「depends on REGMAP_I2C」，构成环。而 `REGMAP_I2C`
在主线是**无 prompt 的 tristate**，本来就只能被 select，对它写 `depends on`
本身也是错误写法。

**本包已修复**（第 5 轮）：

1. `port/integration/Kconfig.fragment` 不再对 `REGMAP_I2C` 写任何
   `depends on` / `select`，所需基础设施由 `ares_defconfig.fragment` 的
   `CONFIG_REGMAP_I2C=y` 保证（安装脚本会注入 defconfig）。
2. `auto_install.sh` / `apply.sh` 安装时自动调用
   `tools/fix_kconfig_recursion.py`，把**目标树里残留的旧写法一并清掉**，
   并把 `--check` 纳入安装后自检。脚本只处理本包 9 个驱动符号，不改主线
   其它 Kconfig，幂等，改动前备份到 `.ares-port-backup/kconfig-fix-<时间戳>/`。

**已经用旧包装过、现在卡在这一步**：不用重装，直接在内核树里跑一次即可：

```bash
python3 /path/to/ares-7.3.3-ported/tools/fix_kconfig_recursion.py /path/to/kernel
make LLVM=1 ARCH=arm64 defconfig
```

只想最小改动（只清我们追加的 ares 菜单块）也可以：

```bash
sed -i '/menu "Xiaomi ares (MT6893) vendor peripherals"/,/^endmenu/{s/ && REGMAP_I2C//g; s/REGMAP_I2C && //g}' drivers/misc/Kconfig
```

若目标树自带的那处同名定义**自身**也写了 `depends on REGMAP_I2C + select REGMAP_I2C`，
sed 就覆盖不到，必须用上面的修复脚本。定位另一半 select 的来源：

```bash
grep -rn -A6 'config REGULATOR_WL2866D' drivers/ --include='Kconfig*'
```

> 该报错属 Kconfig 解析阶段，与 `lld` / 编译器无关；`HOSTLD scripts/kconfig/conf`
> 能成功即说明工具链链接器已就绪。

### 补丁用法

| 补丁 | 用在哪 | 能否直接 `git apply` |
|---|---|---|
| `port/compat/*.patch` | 内核树 `drivers/power/supply/bq25980_charger.c`、`bq27xxx_battery_i2c.c` | ✅ 能（`git apply --check` 已通过） |
| `port/integration/Kconfig.fragment`、`Makefile.rules`、`defconfig.additions` | 内核树对应 Kconfig / Makefile / defconfig | ✅ 手工合并或由 `apply.sh` 完成 |
| `port/dt-bindings/gpio-normalize-official.patch`、`mainline-contract.patch` | **审计证据**：记录「组装版 → 交付版」的 15 个新增 `*-gpios` 属性与 12 类修复 diff | ❌ 源（未修复的组装 dts）不在本包，交付 dts 已是最终态 |
| `Makefile.patch` | 内核树 `arch/arm64/boot/dts/mediatek/Makefile` | ✅ 能，等效于 `install.sh` 的自动追加 |

---

## 四、硬件批次切换（重要）

原厂 ares 存在多个触控 IC 批次与触觉挂载总线，4.14 靠 defconfig 二选一；
主线 dts 没有 defconfig，故在**主 dts 头部用 cpp 宏**在编译期消解资源冲突
（GPIO20/21 + IRQ21 只能属于一个触控，GPIO129/22 只能属于一个触觉）：

```c
#define ARES_TOUCH_VARIANT  ARES_TOUCH_GT9886   /* 第 45 行：按真机批次改 */
#define ARES_HAPTIC_BUS     ARES_HAPTIC_I2C7    /* 第 49 行：i2c7 或 i2c9 */
```

可选值：`ARES_TOUCH_GT9886` / `ARES_TOUCH_FT3658U` / `ARES_TOUCH_GT9896S`，
`ARES_HAPTIC_I2C7` / `ARES_HAPTIC_I2C9`。三个触控节点与三份触觉数据**都保留在源码里**，
改宏即可切换，不需要删节点。真机判别：`dmesg` 看触控 IC 自报 ID，或用 i2c 探测
`0x5d` / `0x5a` 落在哪条总线。

---

## 五、已验证（真实执行结果）

| 项 | 结果 |
|---|---|
| 内核树 `make LLVM=1 ARCH=arm64 ...dtb` | **exit 0**，222,232 字节，`__symbols__` 存在 |
| 反编译回读 | GT9886 `okay`、另两触控 `disabled`、AW8697 恰好一个启用；4-cell IRQ 残留 **0** |
| `dtbs_check`（首次跑通） | 4087 → **4027** 行；`status` 条件失败 9→0、错 tphy 7→0、`reg` 缺失 2→0、interrupts 超限 13→10 |
| 9 个驱动回归编译 | **0 error / 0 warning**，9 个 `.ko` 全产出 |
| 修复脚本幂等 | 两个脚本对交付 dts 重复运行无变化，`verify.sh` 可复验 |
| Kconfig 递归依赖 | 在干净 7.3.3 树上**精确复现**该报错 → `fix_kconfig_recursion.py` 修复后 `make LLVM=1 ARCH=arm64 defconfig` **exit 0**；`--check` 在干净树无误报，主线其它 Kconfig 零改动 |
| `git apply --check Makefile.patch` | 通过 |
| **本轮**：`awinic, aw2028_i2c` 空格 | dts 2 处、驱动匹配串、组装链规则、dtb、文档全部同步；幂等自检通过 |
| **本轮**：GPU 节点 `dtbs_check` | `dt-validate` 中 GPU / OPP 相关诊断 **0 行**；`gpu@13000000` 通过 `arm,mali-bifrost.yaml` |
| **本轮**：USB host `dtbs_check` | 去掉注入块重编对比：1670 → 1661 条，消失 9 条全是 USB binding 错误、新增 0 条实质错误；注入节点自身 0 报错 |
| **本轮**：USB 角色切换 | host / peripheral 两个 dtb 均编译通过，dtbs_check 报错集合差异为空；默认 host dtb 加宏前后字节完全一致 |
| **本轮**：`CONFIG_DRM_PANFROST` | 9 个配置符号全部 `=y`；`System.map` 中 panfrost 符号 **161** 个；panfrost 目录实编译 10 个 `.o` |
| **本轮**：相机 EEPROM | 主线化后 `at24` / `nvmem` 接管，`System.map` 中 at24 符号 46 个；`camera_eeprom` 相关 binding 诊断 **0 行** |
| **本轮**：全量构建 | `build_workflow.sh` **9 阶段通过 / 0 失败**，Image 41 MB、dtb 218 KB、9 个 `.ko` 全产出 |
| **本轮**：配置复验 | `build_workflow.sh --config-only` 6 阶段通过；新增 12 项 gadget / 网络配置在 `.config` 中全部生效 |

12 类主线契约修复详见 `docs/ROUND3-FIXES.md`，包括：`status="ok"`→`okay`、
4-cell IRQ→2-cell（主线 `pio` 是 2-cell）、`mt6785-tphy`→`mt6893-tphy`、
otg_vbus 统一 5V、无 `reg` 的单元地址节点、AW8697 重复块非破坏性禁用等。
**本轮 GPU 与驱动覆盖审计**分别见 `docs/GPU-MAINLINE.md` 与
`docs/DRIVER-COVERAGE.md`；**网络可行性与做法**见 `docs/NATIVE-NETWORKING.md`
与 `docs/USB-NETWORKING.md`。

---

## 六、已知限制（务必读）

1. **SoC 层不在本包**：干净主线 7.3.3 没有 `mt6893.dtsi` / `clk-mt6893`，需先合入
   目标仓库已有的部分。按「已有且真实可用不重复移植」的要求，这部分未重复移植。
2. **`dtbs_check` 仍有大量既存噪声**：用含本包 binding 补丁的 schema 校验交付
   dtb 为 1661 条，全部来自原厂 dts 中无主线 binding 的私有节点（`bytes`
   类型 1519 条、vendor-prefixes 40 条、spi/i2c 私有属性等）与 SoC 层
   `mt6893.dtsi` 的历史偏差。判断本包改动是否合规要用「去掉注入块重编、
   对比报错集合」的零回归口径：USB 注入修复 9 条、新增 0 条，
   详见 `docs/USB-NETWORKING.md` §6。
3. **无真机验证**：地址 / 中断 / 电气 / 驱动 probe 全部未上电验证，编译通过 ≠ 能启动。
4. **GF3658 指纹**仍需 TEE TA 与 Android HAL 配合才能工作。
5. `chosen` 保留官方 bootargs（`console=ttyS0,921600n1`），串口需主线 `8250_mtk`
   补 MT6893 compatible 才能真正出 log。
6. 文档 `docs/` 中出现的 `vendor-reference/...` 路径指**原工程**，用于说明数据来源，
   本包不含这些文件，也不依赖它们。
7. `port/*.ko` 与 `port/build.log` 内嵌的编译路径是产出时的工程位置，仅作编译证据，
   不构成运行依赖；重新执行 `port/build.sh` 会按你当前路径刷新。
8. **GPU 只能 probe，不保证渲染可用**：`clocks = <&clk26m>` 是满足 binding 的
   保守接法，真实 MFG PLL / DVFS 需主线没有的 `clk-mt6893`；`mali-supply` 指向
   私有 PMIC 树下的 vgpu11；无 power-domains / IOMMU 绑定。详见 `docs/GPU-MAINLINE.md`。
9. **相机 EEPROM 5 个源节点 → dtb 里 4 个设备**：原厂 `i2c4` 下两个 `0x51`
   节点主线化后同名被 dtc 合并（同 I2C 地址本就不该有两颗），功能等价。
   容量默认 2KB，可在节点加 `size = <...>` 覆盖。详见 `docs/DRIVER-COVERAGE.md`。
10. **自带 WiFi 与 SIM 卡蜂窝数据不可用**：MT6635 combo 走 connsys / WMT、
    集成 modem 走 CCCI / mddriver，主线 7.3.3 都没有，还依赖闭源固件与
    Android HAL。在「不硬改」边界内只能走 USB 外接设备，或 USB gadget
    连 PC 共享网络（零额外硬件但借 PC 的网络）。
    完整证据链见 `docs/NATIVE-NETWORKING.md`。**本轮已按用户要求把原厂
    WiFi 私有驱动与私有节点保真导出，完成主线 API 适配、私有节点重写、
    init 从 vendor 提取固件脚本，并进一步参考 xiaomi-oss 的 `ares-r-oss`
    补齐 MTK 平台基线层**（`wifi-vendor-port/`，放宽「不硬改」边界后的独立素材）：
    `baseline/` 含 36 个平台头 + 182 个平台构建文件 + 33 个链接期导出符号清单，
    编译探针同口径对比干净 70→80、缺头 23→13、120/120 无旧 API 残留。
    **最新一轮（stage-3）**又从 GitHub 搜到更完整的
    `xiaomi-mt6893-dev/kernel_xiaomi_mt6893@lineage-23.2`，把那 33 个符号
    从「只有清单」推进到 **21 个保真实现 `.c` + 12 个新补 `EXPORT_SYMBOL_GPL`
    + 31 个有实现**（余 2 个源树只有 `extern` 声明，给桩模板），并补齐
    137 个 MT6893 变体/依赖头；平台实现编译探针 **API 残留 0**。
    **最新一轮（stage-4）**进一步复核那 2 个「未开源符号」并推翻旧判断：
    `aee_rr_rec_clk` 其实有真实开源实现（上轮关键词漏搜 `ram_console/` 路径），
    已保真导出并函数级移植；`buck_set_mode` 的 7 次「调用」全是注释、
    且为 MT6335 专用废 API（MT6893 用 MT6359P），真实调用 0、从清单剔除。
    两符号由 `solution/` 的 `__weak + EXPORT_SYMBOL_GPL` 实现消除链接期
    undefined，并经 7.3.3 头文件实编译与 nm 核验。
    全量链接仍需目标树「已修基线层」提供 B 类（clk/iommu）框架符号；
    不影响本包 7.3.3 编译产物，也不改变本节结论。

---

## 七、文档索引

| 文档 | 内容 |
|---|---|
| `wifi-vendor-port/docs/BASELINE-PORTING.md` | **stage-2**：ares-r-oss 补平台基线完整说明（两棵树互补、分类原则、保真校验、接线 bug 修复、同口径前后对比、边界与复现命令） |
| `wifi-vendor-port/docs/GITHUB-SOURCES.md` | **stage-3**：GitHub ares/MT6893 内核仓库与分支调研（11 个仓库、选型三条硬指标、固定 commit、CDN 通道实测、复现命令） |
| `wifi-vendor-port/docs/PLATFORM-IMPL-PORTING.md` | **stage-3**：平台符号实现移植总说明（21 个实现 `.c`、12 个新补导出宏、兼容头污染踩坑、BFS 补头、量化验证、接线步骤、边界） |
| `wifi-vendor-port/reports/PLATFORM-IMPL.md` | **stage-3**：33 个平台符号 → 实现文件 → 依据 → 导出状态表 |
| `wifi-vendor-port/reports/API-ADAPTATION-IMPL.md` | **stage-3**：R1–R5 命中数、12 个新补 `EXPORT_SYMBOL_GPL`、2 个桩模板符号 |
| `wifi-vendor-port/reports/IMPL-PROBE.md` | **stage-3**：平台实现编译探针（21 单元、API 残留 0、剩余 2 个缺头均为正确跳过的生成头/arm32 头） |
| `wifi-vendor-port/docs/UNRESOLVED-SYMBOLS.md` | **stage-4（本轮）**：两个「未开源符号」复核与解决（根因、3 条硬证据、函数级移植、接线、量化验证、复现命令） |
| `wifi-vendor-port/solution/platform-symbols.c` | **stage-4**：`aee_rr_rec_clk` 移植 + `aee_rr_curr_clk` 读回 + `buck_set_mode` 兜底，均 `__weak + EXPORT_SYMBOL_GPL` |
| `wifi-vendor-port/reports/SYMBOL-REFS-CORRECTED.md` | **stage-4**：33 符号调用计数勘误（剥离注释/声明）+ 两符号逐行证据 |
| `wifi-vendor-port/reports/SOLUTION-PROBE.md` | **stage-4**：solution 编译探针（CLEAN=1 / 0 error / API 0） |
| `wifi-vendor-port/baseline/reference/…/mtk_ram_console.c` | **stage-4**：`aee_rr_rec_clk` 真实实现保真副本（7 文件，7/7 blob-sha 一致） |
| `wifi-vendor-port/docs/PLATFORM-STUBS.c.template` | stage-3 桩模板 → **stage-4 已停用**（`#if 0`，改用 `solution/`） |
| `wifi-vendor-port/docs/MAINLINE-ADAPTATION.md` | stage-1：主线 API 适配 + 节点重写 + init 固件 完整说明（含 §9 stage-2 修订记录） |
| `wifi-vendor-port/reports/BASELINE-EXPORT.md` | **stage-2**：86 个缺失 include 三分类、57 个平台头 → ares-r-oss 路径映射、歧义与未定位清单 |
| `wifi-vendor-port/reports/BASELINE-MODULES.md` | **stage-2**：709 个平台 kbuild 目标 / 179 个 Kconfig 开关 / **33 个链接期平台导出符号** |
| `wifi-vendor-port/reports/COMPILE-PROBE-BEFORE.md` | **stage-2**：补基线前同口径对照（70 干净 / 23 缺头 / 0 API） |
| `wifi-vendor-port/reports/COMPILE-PROBE.md` | **stage-2**：补基线后编译探针（80 干净 / 13 缺头 / 0 API，120/120 无旧 API 残留）。原「17 干净 / 103 缺平台头」为 `-I` 少一层造成的错误归因，已修正 |
| `wifi-vendor-port/compat/PATH-ALIASES.md` | **stage-2**：2 个 4.14 老路径别名头 + LEGACY 主线头迁移方向 + 剩余 13 个缺头归因 |
| `wifi-vendor-port/reports/API-ADAPTATION.md` | **本轮**：adapt 改写规则命中统计（class_create/access_ok/vfs_*/strlcpy/dma） |
| `wifi-vendor-port/dts/wifi-mainline.dtsi` | **本轮**：主线格式重写的私有节点（label 化、去数字 phandle，dtc 通过） |
| `wifi-vendor-port/README.md` | 原厂 WiFi 私有栈移植/适配包总览（来源 commit、导出范围、适配层速览） |
| `wifi-vendor-port/PORTING-GUIDE.md` | 私有栈 → 主线的依赖图、适配点清单（stage-0 背景） |
| `wifi-vendor-port/firmware/FIRMWARE-LIST.md` | 从原厂 vendor 提取的固件名清单（源码常量反查，含 soc3_0 RAM/patch/WMT.cfg） |
| `docs/NATIVE-NETWORKING.md` | **本轮**：板载原生 WiFi / 蜂窝可行性评估——证据链、三条辨伪、放宽边界的代价 |
| `docs/USB-NETWORKING.md` | **本轮**：USB host 外接网络设备 + gadget 零硬件共享网络、dtbs_check 验证结果 |
| `docs/GPU-MAINLINE.md` | **本轮**：主线格式 GPU 节点（中断换算、mali-supply、OPP）、binding 补丁、PANFROST 强制配置、dtbs_check 结果与已知限制 |
| `docs/DRIVER-COVERAGE.md` | **本轮**：交付 dts 全部 compatible 的覆盖审计，逐项判定「主线已有 / 本包已补 / 仍无驱动 / 不可移植」 |
| `docs/ROUND3-FIXES.md` | **第 4 轮**：12 类主线契约修复、`kernel_mt6893` 复用评估、`dtbs_check` 前后对比 |
| `docs/OFFICIAL-FULL-DTS.md` | 第 3 轮：官方全量设备树移植方案、编译验证与限制 |
| `docs/PORTED-DRIVERS.md` | 第 1 轮：9 个驱动的移植清单与 API 映射 |
| `docs/PORT-ROUND2.md` | 第 2 轮：按实机参数解决 GT9886 / GF3658 / AW8697 的适配死结 |
| `docs/REAL-HW-PARAMS.md` | 实机 BOM / 寄存器 / 帧协议 / GPIO 映射表 |
| `docs/DRIVER-PORTING.md` | A/B/C 类驱动甄别与移植策略 |
| `docs/OFFICIAL-DRIVER-PATHS.md` | 官方私有驱动在 4.14 源码树中的路径 |

---

## 八、把本包一次性上传到 GitHub

`push_to_github.sh` 一条命令把整个目录（58 个文件 / 3.8 MB）**一次性**推成一个
commit。默认走 **GitHub REST Git Data API**：每个文件一个短连接请求、最后合成
一个 tree + 一个 commit，全程没有大流量长连接——正好规避大仓库 `git clone` 常见的
`RPC failed / GnuTLS recv error / early EOF` 断流。

### 前置：一个 Personal Access Token

> 本环境未绑定 GitHub 授权（`gh` 未登录、无 `GITHUB_TOKEN`），因此**无法代传**，
> 需要你在自己的机器或已授权的会话里执行。token 只通过环境变量传入，
> 不写进文件、不出现在命令历史、不打印。

1. GitHub → Settings → Developer settings → Personal access tokens
2. classic token 勾 `repo`（私有仓库）或 `public_repo`（仅公开）；
   fine-grained token 给 `Contents=read/write` + `Metadata=read`，
   若要让脚本自动建仓再加 `Administration=read/write`

### 三条命令（默认 API 通道，推荐）

```bash
cd ares-7.3.3-ported
export GH_TOKEN=<你的PAT>                 # 注意等号后不要写进任何文件
./push_to_github.sh                       # 自动建 <你的用户名>/ares-7.3.3-ported
```

先预演（不联网、不改动远端）：`./push_to_github.sh --dry-run`

### 常用变体

```bash
./push_to_github.sh yourname/myrepo --private     # 指定仓库名 + 私有
./push_to_github.sh --branch port-7.3.3           # 指定分支
./push_to_github.sh --path kernel/ares            # 传到已有仓库的子目录
./push_to_github.sh --git                         # 改用传统 git push（单次 push + 3 次断流重试）
GH_API=https://ghe.example.com/api/v3 ./push_to_github.sh   # GitHub Enterprise
```

脚本可**重复运行**：仓库已存在时基于当前分支做增量 commit（带 parent），不会
重复建仓、不会清空历史；6 个 `.sh` 与 `fix_kconfig_recursion.py` 的可执行位（`100755`）会保留。
也可用 `gh auth login` 代替 `GH_TOKEN`（脚本会自动取 `gh auth token`）。

### 如果用 gh CLI（最简，但依赖一次长连接 push）

```bash
gh auth login
cd ares-7.3.3-ported
gh repo create ares-7.3.3-ported --public --source=. --push
```

目录只有 3.8 MB，这条一次就能完成；若你的网络像大仓库克隆那样反复 TLS 断流，
改用上面的 API 通道更稳。

---

## 九、自动构建 workflow（强制 defconfig + KVM / NTSYNC）

`build_workflow.sh` 一条命令跑完全流程；`tools/force_defconfig.py` 负责把配置
**强制**写进 `arch/arm64/configs/defconfig`。

### 与 auto_install.sh 的差别

| | `auto_install.sh` | `build_workflow.sh` |
|---|---|---|
| defconfig | 跳过同值项后追加标记块（温和） | 删除所有同名旧赋值与历史块，写入唯一一份（**强制**） |
| 编译 | 不编译（除非 `--build`） | 默认编 dtb + 9 个 `.ko` + `Image`；`--builtin` 时改为全部内建、不编 `.ko` |
| 产物 | 留在内核树 | 收集到独立目录并打包 |

「强制」= 每个 CONFIG 符号在 defconfig 里**恰好出现一次、值唯一**：既不会触发
`override: reassigning to symbol X`，也不会被目标树原有的 `=m` / 未设置状态悄悄降级。

### 快速开始

```bash
cd ares-7.3.3-ported
chmod +x build_workflow.sh

# 先预演（不改动任何文件）
./build_workflow.sh /path/to/linux-7.3.3 --dry-run

# 只验证配置，不编译
./build_workflow.sh /path/to/linux-7.3.3 --config-only

# 完整构建（默认 -j nproc）
./build_workflow.sh /path/to/linux-7.3.3 \
  --output /path/to/ares-build-output \
  --package /path/to/ares-build-artifacts.zip
```

常用开关：`--builtin`（**内建版**，见第十章）、`--initramfs`（**早加载模块做进
initramfs**，见第十一章）、`--skip-install`（已装过）、
`--no-image` / `--no-dtb` / `--no-modules`、`--all-modules`（全量 modules）、
`--with-vmlinux`、`--strict`（遇错即停）、`--package-only`（只重新打包）、`-j N`。

### 强制写入的配置项

来源优先级（后者覆盖前者的同名项）：`ares_defconfig.fragment`（A 类 29 项）→
`port/integration/defconfig.additions`（B 类 9 项）→ `ares_build_config.additions`：

```text
CONFIG_VIRTUALIZATION=y    # KVM 前提
CONFIG_KVM=y               # ARM64 KVM（主线 defconfig 已有，这里强制固化）
CONFIG_NTSYNC=y            # Wine/Proton 的 /dev/ntsync，7.3.3 位于 drivers/misc/ntsync.c
CONFIG_INPUT_FF_MEMLESS=y  # haptic-aw8697 的依赖，必须内建（见第十一章）
```

> `CONFIG_NTSYNC=y` 让内核镜像自带，不依赖 rootfs 里放 `.ko`；改成 `=m` 后重跑即可。
> 真机能否进 EL2 由 bootloader 决定，内核侧只需上面两个 KVM 符号。
> `CONFIG_INPUT_FF_MEMLESS=y` 是为 initramfs 补的依赖修正：`HAPTIC_AW8697`
> 的 Kconfig 有 `select INPUT_FF_MEMLESS`，模块版下会被拉成 `=m`，而 workflow
> 不编全量 modules，`insmod haptic-aw8697.ko` 会缺符号；固化 `=y` 后三种形态都安全。

临时追加/覆盖：`--extra CONFIG_NTSYNC=m`。查看完整清单：
`python3 tools/force_defconfig.py <内核树> --list`；校验：`--check`（不符返回 1）。
写入前自动备份到 `.ares-port-backup/force-defconfig-<时间戳>/`。

### 阶段与产物

| 阶段 | 动作 | 日志 |
|---|---|---|
| preflight | 检查 clang / ld.lld / flex / bison / dtc 等，缺件给 apt 提示 | 屏幕 |
| install | `auto_install.sh`（幂等） | `logs/install.log` |
| force-config | 强制写 defconfig + `--check` 校验 | `logs/force-config.log` |
| defconfig | `make LLVM=1 ARCH=arm64 defconfig` + 二次强制 + `.config` 核对 | `logs/defconfig.log`、`config-verify.txt` |
| build-dtb | `mediatek/mt6893-xiaomi-ares-official.dtb` | `logs/build-dtb.log` |
| build-modules | 9 个移植驱动 `.ko`（Kbuild 真编译） | `logs/build-modules.log` |
| build-image | `make ... Image` | `logs/build-image.log` |
| collect / report / package | 产物归集、`BUILD-REPORT.md`、zip | `logs/*.log` |

产物目录含 `Image`、`mt6893-xiaomi-ares-official.dtb`、`System.map`、`.config`、
`defconfig.forced`、`modules/*.ko`、各阶段日志与 `BUILD-REPORT.md`（阶段结果表、
配置核对、产物清单、失败明细）。

### 已知前提

- 干净主线 7.3.3 没有 SoC 层 `mt6893.dtsi` / `clk-mt6893`，**dtb 阶段会失败**；
  workflow 会如实记录，并回退带上交付包内已编译好的现成 dtb。用已含 SoC 层的
  `mt6893_kernel_ares_mainline` 树即可编译出 dtb。
- 任一阶段失败默认不中止（报告里逐项列出错误明细），便于一次跑完看到全貌。
- `ci/ares-build.yml` 是 GitHub Actions 模板：装 LLVM 工具链 → clone 内核树 →
  调用 `build_workflow.sh` → `upload-artifact` 上传报告与产物包。

## 十、内建版内核（驱动全部编进 Image，不产 `.ko`）

手机刷机若不想往 vendor ramdisk 里塞 `.ko`、也不想在 init 阶段逐个 `insmod`，
用 `--builtin` 跑一份**驱动全部随 `Image` 链接**的内核：

```bash
./build_workflow.sh /path/to/linux-7.3.3 --builtin -j "$(nproc)" \
  --output ares-builtin-output \
  --package ares-7.3.3-builtin-artifacts.zip

# 只验证配置，不编译
./build_workflow.sh /path/to/linux-7.3.3 --builtin --config-only
```

### 与模块版的差别

| | 模块版（默认） | 内建版 `--builtin` |
|---|---|---|
| 强制项数 | 88 | 94（多 6 项父级框架） |
| 清单里 38 个 `=m` 项 | 保持 `=m` | 全部提升为 `=y` |
| 9 个 B 类驱动 | 另编成 `.ko` | 随 `Image` 链接，**无 `.ko`** |
| `build-modules` 阶段 | 执行 | 跳过 |
| 产物 | 含 `modules/*.ko` | 不含 `modules/` 目录 |
| `verify-builtin` 阶段 | 不执行 | 用 `System.map` 验证 9 个驱动符号 |

38 项 = A 类 29 项（fragment 里的 `=m`：PMIC / 充电 / 音频 / USB WiFi /
WWAN / ConfigFS 网络等）+ B 类 9 个移植驱动。

### 为什么多出 6 项

主线 arm64 defconfig 把 `SOUND` / `SND` / `SND_SOC` / `RFKILL` / `GNSS` /
`USB_SERIAL` 都设为 `=m`。Kconfig 规定依赖父级 tristate 的子项取值不能超过
父级：前三个卡住 `SND_SOC_CS35L41` 功放，后三个卡住 `CFG80211` / `MAC80211` /
`MT7921U` / `RTL8XXXU` / `ATH9K_HTC` / `WWAN` / `USB_SERIAL_OPTION` 等 10 项。
子项即使强制写成 `=y`，`make defconfig` + `olddefconfig` 仍会降级回 `=m`；
而内建版不编模块、不产 `.ko`，等于这些驱动直接失效。
`ares_builtin_config.additions` **只在 `--builtin` 时加载**，把 6 个父级提升为
`=y` 解除上限，实测 10 项全部稳定为 `=y`（`System.map` 可见符号）。模块版
不加载本文件，父级保持 `=m`，行为完全不变。

### 关于 `CONFIG_MODULES`

`--builtin` **保留** `CONFIG_MODULES=y`：它保证的是本包涉及的驱动不编成模块，
主线 defconfig 里其余上百个 `=m` 项仍原样保留（workflow 不执行
`make modules`，所以产物里没有任何 `.ko`）。

若要「整棵树彻底禁用模块支持」的纯内建内核，可在构建前把
`arch/arm64/configs/defconfig` 的 `CONFIG_MODULES=y` 改成
`# CONFIG_MODULES is not set` 再跑 `--builtin`。注意这会让主线所有 `=m` 项被
Kconfig 重新定值，`Image` 明显变大、编译更久，个别只在 `=m` 下验证过的驱动
可能需要额外调整。

### 验证产物

`builtin-verify.txt` 与 `BUILD-REPORT.md` 记录 9 个驱动在 `System.map` 中的
命中数与示例符号——**配置写成 `=y` 不等于真的编进镜像**，这一步是链接层证据；
`config-verify.txt` 应显示 0 项不符。

### 实跑结果（clang 18 / lld 18 / -j32）

在已含 SoC 层 `mt6893.dtsi` 的 7.3.3 树上完整跑通：**9 阶段通过 / 0 失败**，
耗时 1 分 4 秒（增量编译）。

| 项目 | 结果 |
|---|---|
| 强制配置项 | 94 项，`config-verify.txt` **0 项不符** |
| 9 个驱动 | `.config` 全部 `=y`，9 个 `.o` 本次真实编译 |
| 链接证据 | `System.map` 命中 9/9（`wl2866d_probe`、`gt9886_probe`、`st21nfc_probe` …） |
| `Image` | 42,588,672 字节（模块版 42,056,192，内建增量 532,480 字节） |
| `.ko` | **0 个**，产物包内无 `modules/` 目录 |
| dtb | 222,232 字节，含 `__symbols__` |

产物包 `ares-7.3.3-builtin-artifacts.zip`（17 MB）含 `Image`、dtb、
`System.map`、`.config`、`defconfig.forced`、`builtin-verify.txt`、
`config-verify.txt`、9 份阶段日志与 `BUILD-REPORT.md`。

---

## 十一、把需要早加载的模块做进 initramfs

### 为什么需要单独处理

`.ko` 如果放在 rootfs（`vendor_dlkm` / `/vendor/lib/modules`），只有 rootfs 挂载
**之后**才读得到。而供电、触控、充电这类驱动在 rootfs 挂载之前就要工作：恢复模式、
解密首屏、关机充电、Android 第一阶段 init。两条路线：

| 方案 | 做法 | 代价 |
|---|---|---|
| 编进内核 | `--builtin`，9 个驱动 `=y`（第十章） | 镜像变大、失去模块可替换性 |
| 放进 initramfs | `.ko` 随 initramfs 在 rootfs 之前解包，早期 `insmod` | 需要维护 cpio 与加载脚本 |

initramfs 是内核启动时解包到 rootfs 的 cpio 归档，早于任何分区挂载。所以
「早加载模块」= 把 `.ko` 打进这个归档，并保证早期有东西按依赖顺序 `insmod`。

### 三条接入路径

| | A 内嵌进 `Image`（本包默认） | B 外部 initrd | C Android vendor_ramdisk |
|---|---|---|---|
| 机制 | `CONFIG_INITRAMFS_SOURCE` | bootloader 传 `initrd=` | BoardConfig / `init.rc` |
| 重编内核 | **是**，cpio 链接进 `Image` | 否 | 否 |
| 谁提供 `/init` | 外部 ramdisk，或 `--initramfs-mode standalone` | 自己写 | Android init |
| 适合 | 单镜像自带、不想改 boot 打包 | 已有独立 ramdisk 分区 | AOSP 刷机 |

关键点：**A 默认与 bootloader 传来的 ramdisk 叠加**——内核先解包内置 cpio 铺好
`/lib/modules/`，随后外部 ramdisk 解包并覆盖同名文件，由它的 `/init` 接管。
`CONFIG_INITRAMFS_FORCE=y` 才会忽略外部 ramdisk；一旦开了它，内置 cpio **必须**
自带 `/init`，否则内核报 `No working init found`。

### 一条命令（推荐）

```bash
./build_workflow.sh /path/to/linux-7.3.3 --initramfs -j "$(nproc)" \
  --output ares-initramfs-output --package ares-initramfs.zip
```

workflow 在 `build-modules` 之后、`build-image` 之前插入两个阶段：

1. `initramfs` — 调 `tools/make_initramfs.py` 生成树 + cpio，再用
   `scripts/config --set-str INITRAMFS_SOURCE <描述文件>` 注入 `.config`；
   `make Image` 时内核 `usr/Makefile` 自动把 cpio 链接进 `vmlinux`。
2. `verify-initramfs` — 解析内核**实际生成的** `usr/initramfs_data.cpio`，逐个
   确认 9 个 `.ko` 与 `modules.dep/load/alias` 都在里面（配置写了不算数）。

可选：`--initramfs-mode android`（附带 `init.ares.rc` / `BoardConfig.mk.fragment`
模板）、`--initramfs-mode standalone --busybox <静态busybox>`（自带 `/init`）、
`--initramfs-force`（开 `INITRAMFS_FORCE`）。

### 单独用生成工具（不跑完整 workflow）

```bash
python3 tools/make_initramfs.py --kernel /path/to/linux-7.3.3 \
  --out ./ares-initramfs --mode modules --cpio --gzip
python3 tools/make_initramfs.py --out ./ares-initramfs --check
```

生成结果：

```text
ares-initramfs/
├── root/lib/modules/7.3.3/     9 个 .ko + modules.dep / modules.load / modules.alias
├── ares-initramfs.list         gen_init_cpio 描述文件 → CONFIG_INITRAMFS_SOURCE
├── ares-initramfs.cpio         可直接给内核吃的归档
├── ares-initramfs.cpio.gz      外部 initrd 用（CONFIG_BLK_DEV_INITRD=y + RD_GZIP）
├── modules.load                加载顺序清单
├── init.ares.rc / BoardConfig.mk.fragment   （--mode android）
└── MANIFEST.md                 依赖判定、校验结果、三种接入方式
```

工具**不依赖**系统的 `cpio` / `depmod` / `modinfo` / `busybox`：模块依赖直接从
`.ko` 的 ELF `.modinfo` 段解析，cpio 由内核自带的 `usr/gen_initramfs.sh` +
`usr/gen_init_cpio` 生成。用描述文件而不是目录，是因为它能精确建
`/dev/console` 设备节点（`nod` 指令）并控制每个文件的 mode/uid/gid。

### 加载顺序与依赖

`modules.load` 的顺序（供电先行，内核虽有 deferred probe，先加载可减少
`-EPROBE_DEFER` 重试、缩短首帧）：

```text
regulator-wl2866d → xmusb350-charger → et7480-audioswitch → touchscreen-gt9886
→ haptic-aw8697 → leds-aw2028 → input-gamekey → gf3658-spi → st21nfc
```

`modules.dep` 按 `.modinfo` 的 `depends` 拓扑排序生成，可被 `modprobe` 直接使用；
`modules.alias` 收录设备树 `of:` 别名，供后期 udev 自动匹配。工具会检查「不在本目录」
的外部依赖在 `.config` 里是 `=y` 还是 `=m`：本包唯一一处是
`haptic-aw8697.ko → ff-memless`，实测 `CONFIG_INPUT_FF_MEMLESS=y` 已由内核内建满足，
`insmod` 不会因未知符号失败；若某个依赖仍是 `=m`，工具会明确要求一并打包或改成 `=y`。

### `/init` 的衔接（最容易踩的坑）

默认的 `modules` 模式**不写 `/init`**：它只铺模块，加载动作交给外部 ramdisk。
Android 上把下面几行放进 `vendor_ramdisk` 的 `init.rc`（工具会生成完整
`init.ares.rc`）：

```text
on early-init
    insmod /lib/modules/7.3.3/regulator-wl2866d.ko
    insmod /lib/modules/7.3.3/touchscreen-gt9886.ko
    ...   # 顺序同 modules.load
```

AOSP 可用 `BOARD_VENDOR_RAMDISK_KERNEL_MODULES` +
`BOARD_VENDOR_RAMDISK_KERNEL_MODULES_LOAD` 让构建系统自动拷模块并生成
`modules.load`（模板见 `BoardConfig.mk.fragment`）。注意 Android 12+ GKI 的普通
vendor 模块放 `vendor_dlkm` 动态分区、由第二阶段 init 加载；只有 rootfs 挂载前
就要用的才放 `vendor_ramdisk`。

若是纯 Linux 启动、没有外部 ramdisk，用 `standalone` 模式：工具会写 `/init`
（挂 proc/sysfs → 按 `modules.load` 逐个 `insmod` → 解析 `root=` →
`switch_root`），并需要 `--busybox` 提供静态 busybox（自动塞进 cpio 并建好
`sh`/`mount`/`insmod`/`switch_root` 等软链）。

### 纯手工三步（不使用本包脚本时）

```bash
# 1. 目录布局（内核版本目录名要与 vermagic 一致）
mkdir -p iram/lib/modules/7.3.3 && cp port/*.ko iram/lib/modules/7.3.3/
printf '%s\n' 'regulator-wl2866d.ko:' ... > iram/lib/modules/7.3.3/modules.dep
# 2. 注入配置（指向目录即可，内核会自己扫描；但拿不到 /dev/console nod）
scripts/config --file .config --set-str INITRAMFS_SOURCE "$(pwd)/iram"
make LLVM=1 ARCH=arm64 olddefconfig
# 3. 重编镜像
make LLVM=1 ARCH=arm64 -j"$(nproc)" Image
```

指向**目录**时内核用 `gen_initramfs.sh` 的 `dir_filelist` 扫描，简单但无法创建
设备节点（靠 `CONFIG_DEVTMPFS_MOUNT=y` 自动挂 `/dev`）；指向**描述文件**才能用
`nod` 与 `slink`，也更可复现。改了产物目录后必须重新生成描述文件并重编 `Image`。

### 实跑结果（clang 18 / lld 18 / -j32，模块版 + `--initramfs`）

在已含 SoC 层 `mt6893.dtsi` 的 7.3.3 树上完整跑通：**11 阶段通过 / 0 失败**，
约 58 秒（增量编译）。

| 项目 | 结果 |
|---|---|
| 强制配置项 | 42 项（含本轮新增 `CONFIG_INPUT_FF_MEMLESS=y`） |
| `CONFIG_INITRAMFS_SOURCE` | 指向生成的 `ares-initramfs.list`；`INITRAMFS_COMPRESSION_GZIP=y` |
| 内核内嵌 cpio | `usr/initramfs_data.cpio` = 3,080,704 字节 / 20 条目 |
| 链接层验证 | 9 个 `.ko` + `modules.dep/load/alias` + `dev/console` **全部命中** |
| `Image` | 43,170,304 字节（模块版基线 42,056,192，增量 **1,114,112**） |
| 外部 initrd | `ares-initramfs.cpio.gz` = 1,098,663 字节 |
| `.ko` | `modules/` 9 个仍保留，另 9 个随 initramfs 内嵌进 `Image` |

> 3 MB 的 cpio 只让 `Image` 大 1.1 MB，是因为
> `CONFIG_INITRAMFS_COMPRESSION_GZIP=y` 会在链接进 `vmlinux` 前先 gzip 压缩。

`config-verify.txt` 有 **2 项不符**（`CONFIG_SND_SOC_CS35L41*` 显示 `MISSING`）：
干净 arm64 defconfig 未开 `CONFIG_SOUND`，ASoC 子系统被整体裁掉。这是主线配置的
既有约束、不是写入失败，与 9 个移植驱动 / dtb / initramfs 都无关；`--builtin`
形态显式提升 `SOUND/SND/SND_SOC=y`，不存在此现象。若模块版也要保留音频功放，
在 `ares_build_config.additions` 补 `CONFIG_SOUND=m`、`CONFIG_SND=m`、
`CONFIG_SND_SOC=m` 后重跑即可。

产物包 `ares-7.3.3-initramfs-artifacts.zip`（23 MB）含 `Image`、dtb、
`System.map`、`.config`、`defconfig.forced`、`initramfs/`（文件树 + cpio + gz +
描述文件 + `MANIFEST.md`）、`initramfs_data.cpio`、`initramfs-verify.txt`、
`config-verify.txt`、11 份阶段日志与 `BUILD-REPORT.md`。
