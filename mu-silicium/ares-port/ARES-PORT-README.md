# Mu-Silicium UEFI 移植到 ares (Redmi K40 Gaming / POCO F3 GT, MT6893)

把 [Project-Silicium/Mu-Silicium](https://github.com/Project-Silicium/Mu-Silicium) 的 Project Mu UEFI 固件移植到小米 ares（天玑 1200 / MT6893），目标是在手机上通过 UEFI 启动 Windows ARM64 或 Linux。

## 移植范围与现状

Mu-Silicium 原仓库支持 MT6735 / MT6768 / MT6789 三颗联发科 SoC，**没有 MT6893，也没有 ares 设备**。本次移植新增：

| 组件 | 路径 | 来源 / 依据 |
|---|---|---|
| ares 设备包 | `Platforms/Xiaomi/aresPkg/` | 基于 `stonePkg`（MT6789 / POCO M5）模板 |
| MT6893 Silicon 层 | `Silicon/MediaTek/MT6893Pkg/` | 基于 `MT6789Pkg`，寄存器按 MT6893 校正 |
| ares 配置 | `Resources/Configs/ares.toml` | 原厂 boot.img header v2 参数 |
| ares DTB | `Resources/DTBs/ares.dtb` | 已移植的 7.3.3 树 `mt6893-xiaomi-ares-official.dtb` |

## 关键硬件参数（已核对原厂 4.19 / 7.3.3 树）

| 项目 | 值 | 来源 |
|---|---|---|
| SoC | MT6893（Dimensity 1200），4×A78 + 4×A55 | `mt6893.dtsi` |
| PMIC | MT6359（pwrap@0x10026000） | `mt6893.dtsi` |
| GIC | GICD@0x0C000000 / GICR@0x0C040000（GICv3） | 原厂 dts |
| topckgen / infracfg / apmixed | 0x10000000 / 0x10001000 / 0x1000C000 | `mt6893.dtsi` |
| pinctrl | 0x10005000，220 个 GPIO | `mt6893.dtsi` gpio-ranges |
| MSDC | 0x11230000 / 0x11240000 | `mt6893.dtsi` |
| WDT | 0x10007000 | `mt6893.dtsi` |
| 屏幕 | 1080×2340，32bpp | ares `atag,videolfb-*` |
| framebuffer | 0x7E020000，size 0x1BE0000 | ares `atag,videolfb-fb_base_l` |
| 内存基址 | 0x40000000 | 原厂 boot header |
| kernel 加载 | 0x40080000 | 原厂 boot.img `kernel_addr` |
| tags / ramdisk | 0x47C80000 / 0x51100000 | 原厂 boot.img header |
| 主存储 | **UFS**（非 eMMC） | ares dts `ufs_mphy@11fa0000` |

## 如何应用与编译

本交付包是**代码级自包含移植包**（27 个文件），需应用到官方 Mu-Silicium 仓库：

```bash
# 1) 克隆官方仓库（约 5GB，含子模块）
git clone https://github.com/Project-Silicium/Mu-Silicium.git --recursive

# 2) 应用本移植包（一键脚本）
bash /workspace/mu-silicium-ares-port/apply_port.sh /path/to/Mu-Silicium

# 3) 安装编译依赖 + 拉取子模块
cd /path/to/Mu-Silicium
./setup_env.sh -p apt

# 4) 编译 ares
python3 build_uefi.py -d ares
# 产物：Mu-ares-0.img（UEFI FD 作为 kernel 打进 boot.img）
```

> **GitHub**：移植包已推送到 `James-2HBe/mt6893_kernel_ares_mainline` 的新分支 **`ares-uefi-port`**（commit `2112a094`），位于 `mu-silicium/ares-port/` 子目录，不影响原有分支。

### 本次验证状态

- 已完成代码级移植：23 个必需文件齐全、7 个 FILE_GUID 唯一、DSC include 链完整、所有 INF 源文件引用存在、ares.toml 可解析、DTB magic 有效 —— **静态校验 12/12 通过**。
- **未在本机完成实际 EDK2 编译**：当前环境网络持续超时（约 40KB/s 且频繁 connection reset），无法拉取编译所需的约 5GB 子模块（mu_basecore / mu_plus / mu_oem_sample / openssl / Silicium-ACPI / Device-Binaries）。编译步骤已备好，需在具备网络条件的环境执行。

## 已知限制（重要，务必阅读）

1. **存储**：ares 主存储是 UFS，而 Mu-Silicium 的 MediaTek 后端只有 MSDC（eMMC/SD）驱动。因此 UEFI 可启动到图形/文本界面，但**从 UFS 加载 Windows/Linux 启动项需要额外的 UFS 驱动**，本次移植未包含。这是 ares 与 stone(MT6789，eMMC) 的本质差异。

2. **时钟表**：`MT6893Pkg/Library/ClockImplLib/ClockImplLib.c` 基于 MT6789 时钟表改造（MTK topckgen/APMIXED 布局通用）。MT6893 的 PLL 频率/偏移与 MT6789 不完全一致，真机若时钟异常需按 `clk-mt6893` 的 `mt6885-clk.h`（279 个 clock enum）进一步校正。

3. **按键**：音量上/电源键走 PMIC（MT6359 pwrkey/homekey，接口与模板兼容），音量下走 GPIO 66（沿用模板占位，真机可按实际 GPIO 调整）。

4. **验证状态**：本次完成代码级移植并尝试编译。UEFI 能否在真机点亮、能否引导 OS 需真机验证；编译通过 ≠ 真机可用。

## 与 Linux 内核移植的关系

本目录是 UEFI 固件移植（Project Mu / EDK2），与之前完成的 Linux 7.3.3 内核移植（`/workspace/linux-7.3.3`）是**两套独立的代码栈**，互不依赖。DTB 从内核树复用，其余独立。
