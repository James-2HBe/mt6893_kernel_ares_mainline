#!/usr/bin/env python3
"""
从「厂商 ares 设备树源码」+「仓库 chopin 参考板」组装出
可编译的 mt6893-xiaomi-ares.dts（面向 James-2HBe/mt6893_kernel_ares_mainline
的 7.3.3-mt6893-xiaomi-chopin 分支）。

策略：
  1. include 仓库已有的 SoC 层 mt6893.dtsi（6235 行，反编译式）
  2. 从 chopin.dts 精确提取 31 个「ares 引用、但 ares 文件自身与 SoC 层都没有定义」
     的通用节点（MT6360 PMU / charger / odm / reserved-memory / chosen 等），
     作为基础节点放进根 /
  3. 为剩余 14 个 ares 特有标签生成最小桩节点
  4. 按厂商原有顺序追加 ares 四份文件里的 &label 覆盖块（真实硬件数据）
  5. 修正 include：mt6885-pinfunc.h -> mt6893-pinfunc.h，
     去掉两个不可得的厂商 dtsi，并补 MTK_PUPD_SET_R1R0_10 兼容宏

用法：python3 assemble_ares_dts.py <素材目录> <输出.dts>
"""
import re
import sys
from pathlib import Path

# chopin.dts 中定义、需要借用的基础节点 label。
# 分两类：
#  A) ares 厂商 dts 引用、但 ares 自身与 SoC 层都没定义的通用节点（31 个）
#  B) SoC 层 mt6893.dtsi 自身引用、必须由板级提供的标签（13 个）
#     —— 实测：SoC 共引用 61 个 label，其中 317 个自定义，仅这 13 个悬空。
BORROW = [
    # A: ares 板级通用节点
    "mt6360_pmu", "mt6360_pmic", "mt_pmic_vtp_ldo_reg", "mt_pmic_vfp_ldo_reg",
    "mt_pmic_vmch_ldo_reg", "chosen", "reserved_memory", "mmc_fixed_3v3",
    "mmc_fixed_1v8_io", "tboard_thermistor6", "mtkfb", "kd_camera_hw1",
    "camera_af_hw_node", "odm", "goodix_fp", "accdet", "bat_gm30",
    "irtx_pwm", "lk_charger", "charger", "cp_qc30", "usbpd_pm", "gamekey",
    "mt6360_typec", "nfc_default", "nfc_int_active", "nfc_int_suspend",
    "nfc_enable_active", "nfc_enable_suspend",
    # B: SoC 层必需的时钟 / GIC / 固定 regulator / extcon
    "clk12m", "clk26m", "extcon_usb", "gic", "otg_vbus", "u2port",
    "ufs_fixed_vcc", "vusb33",
    # 注意：wifi_mem 不单独借，reserved_memory 节点内部已定义该 label
]

# 仍需自建的最小桩。
# 实测：ares 厂商 overlay 自身已定义了 wl2866d* / irtx_gpio_led_* / usb_c_pinctrl
# 等绝大多数 label，盲目建桩会触发 duplicate_label。这里只保留真正悬空的。
STUBS = []

# 厂商 label -> 仓库 SoC 层等价 label 的重映射。
# 厂商树里 mt6359regulator 由 PMIC dtsi 定义；仓库 SoC 层已有
# `regulators: regulators` （mediatek,mt6359-regulator），语义等价。
REMAP = {
    "mt6359regulator": "regulators",
}


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return text


def extract_node(text: str, label: str) -> str | None:
    """提取 `label: name { ... }` 的完整块，按花括号配对（忽略注释/字符串）。"""
    m = re.search(r"^([ \t]*)" + re.escape(label) + r":\s*[^\n{]*\{", text, re.M)
    if not m:
        return None
    indent = m.group(1)
    start = text.rfind("\n", 0, m.start()) + 1
    clean = strip_comments(text[start:])
    # 在原文本上按字符计数，跳过注释与字符串
    depth, i, n = 0, m.end() - 1 - start, len(text[start:])
    seg = text[start:]
    in_str = False
    while i < len(seg):
        c = seg[i]
        if in_str:
            if c == '"' and seg[i - 1] != "\\":
                in_str = False
        elif c == '"':
            in_str = True
        elif seg[i:i + 2] == "/*":
            end = seg.find("*/", i + 2)
            i = (end + 2) if end != -1 else len(seg)
            continue
        elif seg[i:i + 2] == "//":
            end = seg.find("\n", i)
            i = (end + 1) if end != -1 else len(seg)
            continue
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                # 节点块以 `};` 结尾，必须把分号一起带上，否则 dtc 语法错误
                j = i + 1
                while j < len(seg) and seg[j] in " \t":
                    j += 1
                if j < len(seg) and seg[j] == ";":
                    j += 1
                return seg[:j].rstrip()
        i += 1
    return None


def clean_vendor(path: Path) -> str:
    """去掉厂商文件里的 #include（含末尾两个不可得的 dtsi），保留其余内容。

    ares.dts 的 `/ { model = "ARES"; compatible = "mediatek,MT6893"; }` 会覆盖
    头部设好的正确 model/compatible，单独剔除；板级身份由生成器的根 / 统一给出。
    """
    text = path.read_text()
    if path.name == "ares.dts":
        text = re.sub(r"/\s*\{[^}]*\}\s*;", "", text, flags=re.S)
    for old, new in REMAP.items():
        text = re.sub(r"&" + re.escape(old) + r"\b", "&" + new, text)
    out = []
    for line in text.splitlines():
        # 厂商 dtsi 是按 DT overlay 写的，自带 /dts-v1/ 与 /plugin/；
        # 合并成单一板级 dts 后，这些顶层标记只能由生成器在开头出现一次。
        if re.match(r"^\s*#include", line) or re.match(r"^\s*/(dts-v1|plugin|memreserve)", line):
            continue
        out.append(line)
    return "\n".join(out).strip()


def main():
    if len(sys.argv) != 3:
        print(__doc__)
        sys.exit(1)
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])

    chopin = (src / "mt6893-xiaomi-chopin.dts").read_text()

    borrowed, missing = [], []
    for l in BORROW:
        node = extract_node(chopin, l)
        if not node:
            missing.append(l)
            continue
        if l == "chosen":
            # chopin 的 chosen 带一整串出厂 bootargs（含 sku=chopin、chopin
            # 面板名、ramoops 地址），对 ares 完全错误。改成空节点，让 ares
            # 自己的 &chosen 覆盖块填 videolfb 参数；bootargs 由 bootloader 传。
            m = re.match(r"([ \t]*chosen:\s*[^\n{]*\{)", node)
            node = m.group(1) + " };"
        borrowed.append(node)

    parts = []
    parts.append("""// SPDX-License-Identifier: GPL-2.0-only
/*
 * Xiaomi Redmi K40 Gaming / POCO F3 GT (ares) - board device tree
 *
 * 面向 James-2HBe/mt6893_kernel_ares_mainline 的 7.3.3-mt6893-xiaomi-chopin 分支。
 * SoC 层来自该仓库的 mt6893.dtsi；本文件只放板级接线。
 *
 * 由 tools/assemble_ares_dts.py 生成。请勿手工编辑后直接覆盖，改素材后重新生成。
 */

/dts-v1/;

#include "mt6893.dtsi"
#include "mt6893-pinfunc.h"
#include <dt-bindings/interrupt-controller/irq.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <dt-bindings/spmi/spmi.h>

/* 厂商 mt6885-pinfunc.h 里的 MTK_PUPD_SET_R1R0_10 由主线
 * mt6893-pinfunc.h -> mt65xx.h 提供（值 102），无需自行定义。 */

/ {
	model = "Xiaomi Redmi K40 Gaming (ares)";
	compatible = "xiaomi,ares", "mediatek,mt6893";
	interrupt-parent = <&gic>;
	/* 与参考板 chopin 保持一致：SoC 层 reg 均为 <0 addr 0 size>，
	 * 不写这两行 dtc 会退回默认 2/1，产生上千条 reg_format 警告。 */
	#address-cells = <0x02>;
	#size-cells = <0x02>;
""")

    parts.append("\t/* ---- 基础节点：借自同 SoC 参考板 chopin（MT6360 / 充电 / odm 等）---- */\n")
    for node in borrowed:
        parts.append(node + "\n")

    parts.append("\t/* ---- ares 特有标签的最小桩：主线/仓库均无对应驱动 ---- */\n")
    parts.append("\tares_stubs {\n")
    for l in STUBS:
        parts.append(f"\t\t{l}: {l} {{\n\t\t\tstatus = \"disabled\";\n\t\t}};\n")
    parts.append("\t};\n")
    parts.append("};\n")

    # 厂商板级数据，按原有 include 顺序：common -> camera -> ares-mt6893 -> ares.dts
    for name in ["xiaomi-mt6893-common.dtsi", "cust_mt6893_camera.dtsi",
                 "ares-mt6893.dtsi", "ares.dts"]:
        body = clean_vendor(src / name)
        parts.append(f"\n/* ===== 板级数据：{name} ===== */\n{body}\n")

    dst.write_text("\n".join(parts))
    print(f"已生成 {dst}  ({len(dst.read_text().splitlines())} 行)")
    print(f"借鉴 chopin 节点: {len(borrowed)}/{len(BORROW)}")
    if missing:
        print(f"未能提取: {missing}")
    print(f"自建桩: {len(STUBS)}")


if __name__ == "__main__":
    main()
