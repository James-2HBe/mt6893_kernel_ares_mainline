#!/usr/bin/env python3
"""
ares 板级设备树组装器 v2 —— 「官方数据版」

与 v1 的根本区别：
  v1 的 37 个基础节点（chosen / reserved-memory / gic / clk / odm / mtkfb /
  charger / PMIC ...）是从 **chopin 反编译 DTB** 借来的，同 SoC 但**不是 ares 真机数据**。
  v2 全部改从 **小米官方内核 MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss（4.14）**
  的真实源码提取，并补上 v1 完全漏掉的 `k6893v1_64.dts`（ares 项目的
  chosen videolfb / odm LED / mmc0 / pio 复用 / PMIC 电压 / 触控 / 面板 / 外设）。

数据源分工（用户选定的「两者交叉」）：
  基础节点   ← 官方 ares-r-oss: mt6893.dts + mt6360.dtsi
  板级外设   ← 用户上传的 lineage-23.2 三份（比官方 R 版更新，含 67W 双电荷泵 / 肩键）
              + 官方 k6893v1_64.dts 补底层配置
  SoC 层     ← 仓库已有 mt6893.dtsi（6235 行，反编译式，真实可用，**不重复移植**）

输出仍是「反编译风格」单文件 dts（与仓库 chopin 板风格一致，非 upstream binding），
可在 Linux 7.3.3 内核树上用 cpp + dtc 编译。

用法: python3 assemble_ares_dts_v2.py <素材根目录> <输出.dts>
"""
import re
import sys
from pathlib import Path

# ── 官方 ares defconfig 中影响设备树 #ifdef 的 CONFIG（真实取值）──────────
# 见 vendor-reference/official/k6893v1_64_defconfig
OFFICIAL_CONFIGS = {
    "CONFIG_MICROTRUST_TEE_SUPPORT": "1",       # =y，soter-shared-mem 保留
    "CONFIG_MTK_SEC_VIDEO_PATH_SUPPORT": "1",   # =y，SVP 保留
    "CONFIG_MTK_CAM_SECURITY_SUPPORT": "1",     # =y
    # 以下未设置 → 不定义，走 #else 分支：
    #   CONFIG_FPGA_EARLY_PORTING      → ion-carveout size 0xc000
    #   CONFIG_MTK_GMO_RAM_OPTIMIZE    → sspm_share size 0x910000
    #   CONFIG_MTK_MET_MEM_ALLOC       → 同上
}

# ── A 组：从官方 mt6893.dts 提取的基础节点（仓库 SoC 层未定义 / ares 真实数据）──
OFFICIAL_ROOT_NODES = [
    "chosen",              # 真实 bootargs（含 console=ttyS0,921600n1）
    "reserved_memory",     # 真实 ares 保留内存布局（含 wifi_mem）
    "gic",                 # gic-v3 真实 reg
    "clocks",              # clk_null / clk26m / clk12m / clk32k 容器（无 label，见下）
    "mmc_fixed_3v3",
    "mmc_fixed_1v8_io",
    "extcon_usb",
    "tboard_thermistor6",
    "mtkfb",
    "camera_af_hw_node",
    "odm",
    "goodix_fp",
    "accdet",
    "lk_charger",
    "charger",
    "cp_qc30",
    "usbpd_pm",
    "gamekey",
    "nfc_default",
    "kd_camera_hw1",
]

# clocks 容器无 label，用节点名匹配（特殊处理）
SPECIAL_NODE_NAMES = {"clocks": r"^\s*clocks\s*\{"}

# ── B 组：从官方 mt6360.dtsi 提取（仓库 SoC 层没有、官方板级 overlay 引用）──
OFFICIAL_PMIC_NODES = [
    "mt6360_pmu", "mt6360_pmic",
    "mt_pmic_vtp_ldo_reg", "mt_pmic_vfp_ldo_reg", "mt_pmic_vmch_ldo_reg",
]

# ── B2 组：官方 pinctrl 子状态（在 mt6893.dts 的 &pio 覆盖块内）+ TypeC 节点 ──
# nfc_* 是官方定义的真实引脚状态；mt6360_typec 在官方 mt6360_pd.dtsi。
OFFICIAL_PINCTRL_NODES = [
    "nfc_int_active", "nfc_int_suspend",
    "nfc_enable_active", "nfc_enable_suspend",
]
OFFICIAL_TYPEC_NODE = "mt6360_typec"  # 在 mt6360_pd.dtsi 的 / { } 内

# ── D 组：仅被 overlay 引用、官方源码无定义的最小桩 ──
# 这些在官方编译链里由 DWS 生成的 k6893v1_64/cust.dtsi / cust_mt6893_*.dtsi
# 提供（GitHub 上 404，属编译期产物，源码仓库不含）。这里建空桩保证 dtc 能解析，
# 并明确标注「非官方真实数据，需真机/厂商 cust.dtsi 补齐」。
#   - irtx_pwm / bat_gm30: 设备节点（红外 PWM / 电池）
#   - mmc0_pins_* / nfc_clk_req_*: pinctrl 子状态
MINIMAL_STUBS = {
    "irtx_pwm": ("device", "红外 PWM 设备（官方 cust.dtsi 提供）"),
    "bat_gm30": ("device", "电池设备（官方 cust.dtsi 提供）"),
    "mmc0_pins_default": ("pinctrl", "MMC0 default 引脚状态"),
    "mmc0_pins_uhs": ("pinctrl", "MMC0 UHS 引脚状态"),
    "nfc_clk_req_active": ("pinctrl", "NFC clkreq active 引脚状态"),
    "nfc_clk_req_suspend": ("pinctrl", "NFC clkreq suspend 引脚状态"),
}

# 官方 label → 仓库 SoC 层等价 label 的重映射（避免重复定义 MT6359 regulator 容器）
REMAP = {
    "mt6359regulator": "regulators",
}

# C 组板级 overlay 追加顺序（官方 k6893v1_64 在前提供底层，lineage 三份在后覆盖外设）
BOARD_OVERLAYS = [
    ("official/k6893v1_64.dts",            "官方 ares 项目底层配置"),
    ("official/cust_mt6893_touch_1080x2300.dtsi", "官方触控"),
    ("xiaomi-mt6893-common.dtsi",          "板级共用外设 (lineage-23.2)"),
    ("cust_mt6893_camera.dtsi",            "相机 (lineage-23.2 上传)"),
    ("ares-mt6893.dtsi",                   "ares 归属配置 (lineage-23.2 上传)"),
    ("ares.dts",                           "ares 入口 (lineage-23.2 上传)"),
]


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def find_node_span(text: str, pattern: str, start_from: int = 0):
    """返回 (start, end) 字符区间：从匹配行首到配对 '}' 后的分号（若有）。"""
    m = re.search(pattern, text[start_from:], re.M)
    if not m:
        return None
    abs_start = start_from + m.start()
    line_start = text.rfind("\n", 0, abs_start) + 1
    brace = text.find("{", abs_start)
    if brace == -1:
        return None
    depth, i, n = 0, brace, len(text)
    in_str = False
    while i < n:
        c = text[i]
        if in_str:
            if c == '"' and text[i - 1] != "\\":
                in_str = False
        elif c == '"':
            in_str = True
        elif text[i:i + 2] == "/*":
            e = text.find("*/", i + 2)
            i = (e + 2) if e != -1 else n
            continue
        elif text[i:i + 2] == "//":
            e = text.find("\n", i)
            i = (e + 1) if e != -1 else n
            continue
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                j = i + 1
                while j < n and text[j] in " \t":
                    j += 1
                if j < n and text[j] == ";":
                    j += 1
                return (line_start, j)
        i += 1
    return None


def extract_label_node(text: str, label: str):
    pat = r"^([ \t]*)" + re.escape(label) + r":\s*[^\n{]*\{"
    return find_node_span(text, pat)


def get_block(text: str, span):
    return text[span[0]:span[1]].rstrip() if span else None


def clean_overlay(path: Path) -> str:
    """清洗官方板级 overlay：去掉顶层标记、#include、REMAP 厂商 label。"""
    text = path.read_text()
    # ares.dts 末尾的 / { model="ARES"; compatible="mediatek,MT6893"; } 会覆盖
    # 生成器设置的正确 model/compatible，单独剔除
    if path.name == "ares.dts":
        text = re.sub(r"/\s*\{[^}]*\}\s*;", "", text, flags=re.S)
    for old, new in REMAP.items():
        text = re.sub(r"&" + re.escape(old) + r"\b", "&" + new, text)
    out = []
    for line in text.splitlines():
        if re.match(r"^\s*#include", line):
            continue
        if re.match(r"^\s*/(dts-v1|plugin|memreserve)", line):
            continue
        out.append(line)
    return "\n".join(out).strip()


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    official = src / "official"

    root_dts = (official / "mt6893.dts").read_text()
    pmic_dtsi = (official / "mt6360.dtsi").read_text()
    pd_dtsi = (official / "mt6360_pd.dtsi").read_text()

    # ── 组装 A 组：官方根 dts 基础节点 ──
    a_nodes, a_missing = [], []
    for label in OFFICIAL_ROOT_NODES:
        if label in SPECIAL_NODE_NAMES:
            span = find_node_span(root_dts, SPECIAL_NODE_NAMES[label])
        else:
            span = extract_label_node(root_dts, label)
        if span:
            a_nodes.append((label, get_block(root_dts, span)))
        else:
            a_missing.append(label)

    # ── 组装 B 组：官方 PMIC 节点 ──
    b_nodes, b_missing = [], []
    for label in OFFICIAL_PMIC_NODES:
        span = extract_label_node(pmic_dtsi, label)
        if span:
            b_nodes.append((label, get_block(pmic_dtsi, span)))
        else:
            b_missing.append(label)

    # ── 组装 B2 组：官方 pinctrl 子状态 + TypeC 节点 ──
    # nfc_* 定义嵌在官方 mt6893.dts 的 &pio 覆盖块内，提取后作为 pinctrl 状态
    # 挂到仓库 SoC 层已有的 &pio；这里放进一个专用容器避免污染根 /。
    b2_nodes, b2_missing = [], []
    for label in OFFICIAL_PINCTRL_NODES:
        span = extract_label_node(root_dts, label)
        if span:
            b2_nodes.append((label, get_block(root_dts, span)))
        else:
            b2_missing.append(label)
    typec_span = extract_label_node(pd_dtsi, OFFICIAL_TYPEC_NODE)
    typec_block = get_block(pd_dtsi, typec_span) if typec_span else None

    parts = []
    # ── 头部 ──
    parts.append("""// SPDX-License-Identifier: GPL-2.0-only
/*
 * Xiaomi Redmi K40 Gaming / POCO F3 GT (ares) — 板级设备树（官方数据版 v2）
 *
 * 目标: James-2HBe/mt6893_kernel_ares_mainline @ 7.3.3-mt6893-xiaomi-chopin
 *
 * 数据来源:
 *   - 基础节点: 小米官方内核 MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss (4.14)
 *               arch/arm64/boot/dts/{mediatek/mt6893.dts, mediatek/mt6360.dtsi}
 *   - 板级外设: 用户上传 lineage-23.2 三份 + 官方 k6893v1_64.dts
 *   - SoC 层:   本仓库已有 mt6893.dtsi（不重复移植）
 *
 * 由 tools/assemble_ares_dts_v2.py 生成，请勿手工编辑后覆盖。
 */

/dts-v1/;

/* 官方 ares defconfig 的条件编译取值（保留 #ifdef 由 cpp 处理） */
""")
    for k, v in OFFICIAL_CONFIGS.items():
        parts.append(f"#define {k} {v}")
    parts.append("""
#include "mt6893.dtsi"
#include "mt6893-pinfunc.h"
#include <dt-bindings/interrupt-controller/irq.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <dt-bindings/spmi/spmi.h>

/ {
\tmodel = "Xiaomi Redmi K40 Gaming (ares)";
\tcompatible = "xiaomi,ares", "mediatek,mt6893";
\tinterrupt-parent = <&gic>;
\t/* 与官方根 dts / chopin 一致：SoC 层 reg 均为 <0 addr 0 size> */
\t#address-cells = <0x02>;
\t#size-cells = <0x02>;

\t/* ══════════ 基础节点：移植自官方 ares-r-oss mt6893.dts ══════════ */
""")
    for label, block in a_nodes:
        parts.append(f"\t/* [{label}] */\n{block}\n")
    parts.append("\t/* ══════════ PMIC 节点：移植自官方 mt6360.dtsi ══════════ */\n")
    for label, block in b_nodes:
        parts.append(f"\t/* [{label}] */\n{block}\n")

    # TypeC 节点（官方 mt6360_pd.dtsi）放根 / 内
    if typec_block:
        parts.append("\t/* [mt6360_typec] 官方 mt6360_pd.dtsi */\n" + typec_block + "\n")

    # ── E 组：同 SoC 通用 fixed-regulator / USB PHY ──
    # 仓库 SoC 层 mt6893.dtsi 的 msdc/ufshci/usb 节点通过 &ufs_fixed_vcc /
    # &u2port / &vusb33 / &otg_vbus 引用这些标签。它们由官方编译期 DWS 文件
    # (k6893v1_64/cust.dtsi，源码仓库 404) 提供，属 SoC 通用接线而非 ares
    # 板级数据。这里按主线 regulator-fixed / usb-phy binding 干净重建，
    # 不带显式 phandle（chopin 反编译版的数值 phandle 会与仓库 SoC 层冲突）。
    parts.append("""
\t/* ══════════ 同 SoC 通用电源 / USB PHY（补 SoC 层悬空引用）══════════
\t * 来源：官方 cust_mt6885_msdc.dtsi 语义 + chopin 反编译 DTB 的电气值；
\t * 按主线 binding 重建，剔除显式 phandle 数值。非 ares 板级假数据。
\t * 电压：ufs_vcc=3.0V vusb33=3.3V otg_vbus=4.43~5.82V（十六进制换算）。
\t */
\tufs_fixed_vcc: regulator-ufs-vcc {
\t\tcompatible = "regulator-fixed";
\t\tregulator-name = "vreg_ufs_vcc";
\t\tregulator-min-microvolt = <3000000>;
\t\tregulator-max-microvolt = <3000000>;
\t\tregulator-always-on;
\t\tregulator-boot-on;
\t};

\tvusb33: regulator-vusb33 {
\t\tcompatible = "regulator-fixed";
\t\tregulator-name = "vusb33";
\t\tregulator-min-microvolt = <3300000>;
\t\tregulator-max-microvolt = <3300000>;
\t\tregulator-always-on;
\t};

\totg_vbus: usb-otg-vbus-regulator {
\t\tcompatible = "regulator-fixed";
\t\tregulator-name = "usb-otg-vbus";
\t\tregulator-min-microvolt = <4425000>;
\t\tregulator-max-microvolt = <5820000>;
\t};

\ttphy: t-phy@11e40000 {
\t\tcompatible = "mediatek,mt6785-tphy", "mediatek,generic-tphy-v2";
\t\t#address-cells = <0x01>;
\t\t#size-cells = <0x01>;
\t\tranges = <0x0 0x0 0x11e40000 0x1000>;
\t\tstatus = "okay";

\t\tu2port: usb-phy@0 {
\t\t\treg = <0x0 0x1000>;
\t\t\tclocks = <&clk26m>;
\t\t\tclock-names = "ref";
\t\t\t#phy-cells = <0x01>;
\t\t\tmediatek,eye-vrt = <0x04>;
\t\t\tmediatek,eye-term = <0x04>;
\t\t\tmediatek,intr = <0x1e>;
\t\t\tmediatek,discth = <0x07>;
\t\t\tmediatek,force-session-valid;
\t\t\tstatus = "okay";
\t\t};
\t};
""")

    # ── D 组设备桩：放根 / 内（新节点无 label 可引用，只能内嵌）──
    parts.append("\t/* ══════════ 最小桩：官方 cust.dtsi 编译期产物，源码不可得 ══════════\n"
                 "\t * 以下节点仅保证 dtc 可解析，非官方真实数据，需以厂商 cust.dtsi\n"
                 "\t * 或 ares 真机 /proc/device-tree 校准后替换。\n"
                 "\t */\n")
    for label, (kind, desc) in MINIMAL_STUBS.items():
        if kind == "device":
            parts.append(f"\t/* [桩] {desc} */\n"
                         f"\t{label}: {label} {{\n\t\tstatus = \"disabled\";\n\t}};\n")

    parts.append("};\n")

    # ── B2 + D 组 pinctrl 状态：作为 &pio 覆盖块追加（仓库 SoC 层已有 pio）──
    parts.append("\n/* ══════════ &pio 引脚状态：官方真实 + 最小桩 ══════════ */\n&pio {\n")
    for label, block in b2_nodes:
        parts.append(f"\t/* [官方] {label} */\n{block}\n")
    for label, (kind, desc) in MINIMAL_STUBS.items():
        if kind == "pinctrl":
            parts.append(f"\t/* [桩] {desc} */\n"
                         f"\t{label}: {label} {{\n\t}};\n")
    parts.append("};\n")

    # ── 组装 C 组：板级 overlay（交叉数据源）──
    overlay_stats = []
    for rel, desc in BOARD_OVERLAYS:
        p = src / rel
        if not p.exists():
            overlay_stats.append((rel, "缺失"))
            continue
        body = clean_overlay(p)
        parts.append(f"\n/* ══════ 板级数据：{rel} — {desc} ══════ */\n{body}\n")
        overlay_stats.append((rel, f"{len(body.splitlines())} 行"))

    dst.write_text("\n".join(parts))

    print(f"已生成 {dst}  ({len(dst.read_text().splitlines())} 行)")
    print(f"官方基础节点: {len(a_nodes)}/{len(OFFICIAL_ROOT_NODES)}  未找到: {a_missing or '无'}")
    print(f"官方 PMIC 节点: {len(b_nodes)}/{len(OFFICIAL_PMIC_NODES)}  未找到: {b_missing or '无'}")
    print(f"官方 pinctrl 状态: {len(b2_nodes)}/{len(OFFICIAL_PINCTRL_NODES)}  未找到: {b2_missing or '无'}")
    print(f"官方 TypeC 节点: {'提取成功' if typec_block else '未找到'}")
    print(f"最小桩: {len(MINIMAL_STUBS)} 个（device + pinctrl）")
    print("板级 overlay:")
    for rel, s in overlay_stats:
        print(f"  {rel}: {s}")


if __name__ == "__main__":
    main()
