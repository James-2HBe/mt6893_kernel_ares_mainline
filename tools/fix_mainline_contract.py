#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# SPDX-License-Identifier: GPL-2.0
"""
fix_mainline_contract.py — 把组装后的 ares 板级 dts 修复到 Linux 主线 7.3.3
设备树契约，并消除「多数据源拼接」产生的资源冲突。

背景
====
v3 组装器把官方 ares-r-oss、官方触控配置、lineage 三份板级 dts 全量拼接，
保留了全部官方节点数据，但也带入了 4.14 厂商契约与同源重复定义：

  1. 14 处 status = "ok"        —— 主线 bindings 只允许 "okay"/"disabled"
  2. 11 处 4-cell interrupts    —— 主线 pio 是 #interrupt-cells = <2>
  3. 3 个触控节点同时启用       —— focaltech(spi6)/gt9886(i2c0)/gt9896s(spi4)
                                    共用 GPIO20/21 与 IRQ21，真机必然冲突
  4. AW8697 触觉同时挂 i2c7/i2c9 —— 共用 GPIO129/22，真机必然冲突
  5. irtx pwm_ch = <0> 覆盖      —— 两个官方仓库均为 <3>（官方真值）
  6. tphy compatible 用 mt6785  —— 主线 tphy.yaml 只认 mediatek,mt6893-tphy
  7. otg_vbus min != max        —— 主线 fixed-regulator.yaml 要求两者相等
  8. fixedregulator@0/1 带 @    —— 带单元地址却无 reg 属性，schema 报缺 reg
  9. 2 处 chopin 字样注释        —— 清理为中性表述
 10. compatible 引号内逗号后带空格 —— 原厂 "awinic, aw2028_i2c" 笔误，
                                     DT spec 要求 vendor,model 紧连无空格

对于 3/4 这类「硬件批次」冲突，不能武断删节点（会丢官方数据），改用
**cpp 宏在编译期二选一**：主线 Kbuild 本就会对 dts 做 cpp 预处理，改一行
宏即可切换批次，dtbs_check / 内核看到的永远只有一个设备占用 IRQ 与 GPIO。
默认值取「有主线移植驱动」的组合：GT9886 + i2c7（官方 ares-r-oss）。

用法
====
  python3 tools/fix_mainline_contract.py <dts-in> <dts-out> [<patch-out>]

幂等：重复运行输出不再变化。
"""
import difflib
import re
import sys
from pathlib import Path

# ── 硬件批次选择器（插入到最后一个 #include 之后）──
MACRO_HEADER = r'''
/* ══════════════ 硬件批次选择器（主线 cpp 预处理；改这里即可切换）══════════════
 * ares 存在多个触控 IC 批次与触觉 IC 挂载总线，官方 4.14 靠 defconfig 二选一；
 * 主线 dts 没有 defconfig，故用 cpp 宏在编译期消解资源冲突：GPIO20/21 + IRQ21
 * 只能属于一个触控设备，GPIO129/22 只能属于一个触觉设备。
 *
 * 数据依据：MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss（原厂）
 *          James-2HBe/kernel_mt6893 @ chopin_kvm（chopin/ares 真机测过）
 * 默认取「有主线移植驱动」且「原厂 ares-r-oss」的组合：GT9886 + i2c7。
 * 真机判别：dmesg 看触控 IC 自报 ID / i2c 探测 0x5d 与 0x5a 落在哪条总线。
 */
#define ARES_TOUCH_GT9886   1
#define ARES_TOUCH_FT3658U  2
#define ARES_TOUCH_GT9896S  3
#define ARES_TOUCH_VARIANT  ARES_TOUCH_GT9886   /* 按真机批次改这一行 */

#define ARES_HAPTIC_I2C7    1
#define ARES_HAPTIC_I2C9    2
#define ARES_HAPTIC_BUS     ARES_HAPTIC_I2C7    /* 原厂 ares-r-oss 用 i2c7 */
'''

# compatible -> (宏名, 常量名)；生成条件 status
TOUCH_VARIANTS = {
    "goodix,gt9886":  "ARES_TOUCH_GT9886",
    "focaltech,fts":  "ARES_TOUCH_FT3658U",
    "goodix,gt9896s": "ARES_TOUCH_GT9896S",
}
HAPTIC_COMPAT = "awinic,aw8697_haptic"

# 4-cell EINT 的 IRQ_TYPE 宏（来自 dt-bindings/interrupt-controller/irq.h）
IRQ_MACRO = r"IRQ_TYPE_[A-Z_]+"

# compatible 字符串内部「逗号 + 空白」：DT spec 的 "vendor,model" 不允许空格。
# 限定在 compatible 属性行内，且逗号必须在引号内部；因此既不会误伤多个
# compatible 之间的 ", " 分隔（逗号在引号外），也不会动 model = "A, B" 之类。
COMPAT_LINE_RE = re.compile(r'^(\s*compatible\s*=\s*)(.*)$', re.M)
COMPAT_SPACE_RE = re.compile(r'("[\w,+-]+,)[ \t]+([\w,+-]+")')

# ── 主线格式 GPU 节点（Panfrost / arm,mali-bifrost.yaml）────────────────
# 由 fix() 第 12 步注入到成品 dts 末尾。先 /delete-node/ 删掉 SoC 层
# mt6893.dtsi 里 4.14 私有的 mali@13000000，再新建符合 ^gpu@[a-f0-9]+$
# 命名规范的节点——节点名是 schema 的硬性要求，而 &label 覆盖改不了节点名。
# 此串不含反斜杠，故可用普通三引号字符串，用 \t 保持与 dts 一致的缩进。
MAINLINE_GPU_BLOCK = """
/* ══════════════ 主线格式 GPU 节点（Panfrost / Mali-G77 Valhall）══════════════
 * 由 tools/fix_mainline_contract.py 注入，请勿手工改动；重建 dts 会自动带回。
 *
 * SoC 层 mt6893.dtsi 的 mali@13000000 是 4.14 私有格式：
 *   compatible = "mediatek,mt6893-mali"      主线 schema 无此条
 *   clocks     = <&topckgen 0x69 ...>        11 项私有 2-cell 时钟
 *   interrupts = <0 0x16c 4 ...>             原始数值，未用 GIC 宏
 * 节点名 mali@... 也不符合 arm,mali-bifrost.yaml 的 ^gpu@[a-f0-9]+$，
 * 而 &label 覆盖改不了节点名，故先 /delete-node/ 删除再整体新建。
 * 新节点满足主线契约，让 CONFIG_DRM_PANFROST 能匹配（驱动 of_match 已含
 * arm,mali-valhall-jm），并通过 dtbs_check。
 *
 * 数据依据（官方 MiCode/Xiaomi_Kernel_OpenSource @ ares-r-oss, 4.14）：
 *   - 中断 <0 0x16c 4  0 0x16b 4  0 0x16a 4>
 *         → GIC_SPI 364/363/362 + IRQ_TYPE_LEVEL_HIGH
 *         （0=GIC_SPI, 4=IRQ_TYPE_LEVEL_HIGH；本 dts GIC 为 3-cell，无需减 32；
 *           对照主线 mt8192 的 365/364/363 正好一一对应）
 *   - reg 0x13000000 + 0x4000（与 SoC dtsi 一致）
 *   - 频率/电压：drivers/misc/mediatek/base/power/include/gpufreq_v1/mt6893/
 *                mtk_gpufreq_internal_plat.h
 *
 * 已知限制（主线缺件，不是漏做）：
 *   1) clocks 暂用 clk26m（26 MHz fixed-clock）满足 schema 的 required，并让
 *      panfrost 的 devm_clk_get(dev, NULL) 能取到 clock；真实 MFG PLL / DVFS
 *      需要 clk-mt6893，主线 drivers/clk/mediatek 没有该驱动。
 *   2) mali-supply 指向厂商 PMIC 的 vgpu11 buck；该节点在私有 PMIC 树下，
 *      主线无 MT6359 regulator 驱动，真机 probe 需 regulator 框架兜底。
 *   3) 无 power-domains / IOMMU 绑定，GPU 只能完成 probe，不保证渲染可用。
 *   4) OPP 表为「绑定合规 + 可 probe」的最小集，频率/电压取自上述原厂
 *      平台头的真实限值，不是完整 DVFS 档位；完整档位依赖 1) 的时钟驱动。
 */

/delete-node/ &mali;

/ {
\tgpu_opp_table: opp-table-gpu {
\t\tcompatible = "operating-points-v2";
\t\topp-shared;

\t\t/* MT_GPUFREQ_BATT_OC_LIMIT_FREQ / LOW_BATT_VOLT_LIMIT_FREQ = 485000 KHz
\t\t * PTPOD_DISABLE_VOLT = 75000 (mV x 100) = 750 mV */
\t\topp-485000000 {
\t\t\topp-hz = /bits/ 64 <485000000>;
\t\t\topp-microvolt = <750000>;
\t\t\tclock-latency-ns = <2000>;
\t\t};

\t\t/* MT_GPUFREQ_CUST_INIT_OPP = 836000 KHz
\t\t * GPU_ACT_REF_VOLT = 80000 (mV x 100) = 800 mV */
\t\topp-836000000 {
\t\t\topp-hz = /bits/ 64 <836000000>;
\t\t\topp-microvolt = <800000>;
\t\t\tclock-latency-ns = <2000>;
\t\t};
\t};

\tgpu@13000000 {
\t\tcompatible = "mediatek,mt6893-mali", "arm,mali-valhall-jm";
\t\treg = <0 0x13000000 0 0x4000>;
\t\tinterrupts = <GIC_SPI 364 IRQ_TYPE_LEVEL_HIGH>,
\t\t\t     <GIC_SPI 363 IRQ_TYPE_LEVEL_HIGH>,
\t\t\t     <GIC_SPI 362 IRQ_TYPE_LEVEL_HIGH>;
\t\tinterrupt-names = "job", "mmu", "gpu";
\t\tclocks = <&clk26m>;
\t\tclock-names = "core";
\t\tmali-supply = <&mt_pmic_vgpu11_buck_reg>;
\t\toperating-points-v2 = <&gpu_opp_table>;
\t\tstatus = "okay";
\t};
};
"""


# ── 主线格式 USB host 控制器（外接 USB 网络设备用）────────────────────
# 由 fix() 第 14 步注入到成品 dts 末尾。MT6893 自带 WiFi(connsys) 与蜂窝
# 数据(CCCI/mddriver) 是 4.14 私有栈 + 闭源固件 + Android HAL，主线 7.3.3
# 完全没有，不能移植；这里走「外接 USB 设备 + 主线标准驱动」路径。
# 不改内核源码、不碰私有栈，只把 SoC 层 usb@11201000 主线化并支持角色切换。
MAINLINE_USB_BLOCK = """
/* ══════════════ 主线格式 USB（host 外接设备 / peripheral 共享网络）══════════════
 * 由 tools/fix_mainline_contract.py 注入，请勿手工改动；重建 dts 会自动带回。
 *
 * SoC 层 mt6893.dtsi 的 usb@11201000 是 4.14 私有格式：
 *   compatible = "mediatek,mtu3"                    schema 要求 SoC 专属串 + 通用串
 *   clocks = <&topckgen 0x7b &infracfg_ao ...>     私有 2-cell，主线无 clk-mt6893
 *   phys = <&u2port &mt6360_pmu>                   第二项指向私有 PMIC 中断控制器
 *   usb-role-switch / role-switch-default-mode = "peripheral"  默认设备模式
 * 主线 mtu3 / xhci-mtk 用 devm_clk_bulk_get_optional() 取时钟（可选），
 * 而 mtu3.yaml 的 phys 要求 minItems:1，故保留 phys 但改指已主线化的 &u2port 0。
 *
 * 依据（均为板级 dts 里已合规的主线 binding）：
 *   - tphy: "mediatek,generic-tphy-v2" + u2port（#phy-cells = <1>）
 *   - clk26m（fixed-clock）、vusb33 / otg_vbus（regulator-fixed）
 *   - mediatek,mtu3.yaml / mediatek,mtk-xhci.yaml 的 compatible enum 由
 *     port/dt-bindings/usb-mtu3-mt6893.patch / usb-xhci-mt6893.patch 补入 mt6893 条目
 *
 * 角色切换（主线 cpp 预处理，编译期二选一）：
 *   0 = host（默认）：插 USB WiFi 网卡 / 4G 上网卡 / USB 以太网，需自备硬件
 *   1 = peripheral：  一根 USB 线连 PC，用 ConfigFS 起 RNDIS / ECM / NCM
 *                    gadget 共享 PC 网络，零额外硬件（但借的是 PC 的网络，
 *                    不是本机自带的 WiFi / SIM 卡）。内核 mtu3 已编为
 *                    DUAL_ROLE，两侧驱动都在，只需改下面一行重编 dtb。
 *
 * 已知限制（真机未验证）：
 *   1) clocks 用 clk26m 满足 schema 的 required；主线驱动只做 prepare/enable，
 *      真实 USB 时钟由 bootloader / 硬件默认维持，枚举是否成功需实机确认。
 *   2) peripheral 模式需用户态用 configfs 组 gadget（u_ether / libcomposite），
 *      且不提供 ADB / MTP / fastboot；两种角色不能同时使用。
 *   3) host 模式插外接设备时，固件需自行放入 /lib/firmware，用户态需
 *      wpa_supplicant / ModemManager / dhcpcd，详见 docs/USB-NETWORKING.md。
 *   4) 自带 WiFi（connsys）与蜂窝（CCCI）在任何角色下都不可用，
 *      原因与代价见 docs/NATIVE-NETWORKING.md。
 */

#define ARES_USB_ROLE_HOST        0
#define ARES_USB_ROLE_PERIPHERAL  1
#define ARES_USB_ROLE             ARES_USB_ROLE_HOST   /* 按需要改这一行 */

/ {
\tusb@11201000 {
\t\t/delete-property/ clocks;
\t\t/delete-property/ clock-names;
\t\t/delete-property/ phys;
\t\t/delete-property/ usb-role-switch;
\t\t/delete-property/ role-switch-default-mode;
\t\tcompatible = "mediatek,mt6893-mtu3", "mediatek,mtu3";
\t\tclocks = <&clk26m>;
\t\tclock-names = "sys_ck";
\t\tphys = <&u2port 0>;
#if ARES_USB_ROLE == ARES_USB_ROLE_HOST
\t\tdr_mode = "host";
#else
\t\tdr_mode = "peripheral";
#endif
\t\tstatus = "okay";

\t\tusb@0 {
\t\t\t/delete-property/ clocks;
\t\t\t/delete-property/ clock-names;
\t\t\tcompatible = "mediatek,mt6893-xhci", "mediatek,mtk-xhci";
\t\t\tclocks = <&clk26m>;
\t\t\tclock-names = "sys_ck";
\t\t\tvbus-supply = <&otg_vbus>;
\t\t\tvusb33-supply = <&vusb33>;
\t\t\tstatus = "okay";
\t\t};
\t};
};
"""

def norm_compat_spaces(joined):
    """逐行规范化 compatible 属性里引号内部的逗号后空格，返回 (文本, 处数)。"""
    total = 0

    def repl(m):
        nonlocal total
        head, body = m.group(1), m.group(2)
        new_body, n = COMPAT_SPACE_RE.subn(r'\1\2', body)
        total += n
        return head + new_body

    return COMPAT_LINE_RE.sub(repl, joined), total

STATS = []


def note(tag, n, detail=""):
    STATS.append((tag, n, detail))


def mask_comments(lines):
    """把注释内容替换为空格，仅用于括号配对计数，原文不变。"""
    out, in_block = [], False
    for line in lines:
        buf, i, n = [], 0, len(line)
        while i < n:
            if in_block:
                j = line.find("*/", i)
                if j < 0:
                    buf.append(" " * (n - i)); break
                buf.append(" " * (j + 2 - i)); i = j + 2; in_block = False
            else:
                j, k = line.find("/*", i), line.find("//", i)
                if j >= 0 and (k < 0 or j < k):
                    buf.append(line[i:j]); i = j + 2; in_block = True
                elif k >= 0:
                    buf.append(line[i:k]); buf.append(" " * (n - k)); break
                else:
                    buf.append(line[i:]); break
        out.append("".join(buf))
    return out


def node_span(masked, compat_idx):
    """给定 compatible 所在行，返回其所属节点的 (start, end) 行索引（含）。"""
    start = compat_idx
    while start >= 0:
        s = masked[start]
        if "{" in s and "}" not in s:
            break
        start -= 1
    if start < 0:
        return None
    depth, end = 0, start
    for i in range(start, len(masked)):
        depth += masked[i].count("{") - masked[i].count("}")
        if depth <= 0:
            end = i; break
    return start, end


def parent_label(masked, start):
    """回溯最近的 &label { 行，返回 label 字符串。"""
    for i in range(start - 1, -1, -1):
        m = re.match(r"^&(\S+)\s*\{", masked[i])
        if m:
            return m.group(1)
        if re.match(r"^\s*/\s*\{", masked[i]):
            return "/"
    return "?"


def indent_of(line):
    return re.match(r"^(\s*)", line).group(1)


def set_conditional_status(lines, masked, compat, macro, const, occurrence):
    """把某 compatible 节点的 status 改为 cpp 条件化（只在指定出现次数上做）。"""
    hits = [i for i, l in enumerate(lines)
            if re.search(r'compatible\s*=\s*"' + re.escape(compat) + r'"', l)]
    if not hits:
        return 0
    done = 0
    # 从后往前改，避免行号漂移
    for idx in reversed(hits[:occurrence] if occurrence else hits):
        span = node_span(masked, idx)
        if not span:
            continue
        s, e = span
        # 幂等：节点内已有本轮生成的条件块则跳过
        if any(re.search(r"#if\s+" + macro + r"\b", lines[i])
               for i in range(s, e + 1)):
            continue
        # 删除块内已有 status 行
        rm = [i for i in range(s, e + 1)
              if re.match(r"^\s*status\s*=", masked[i])]
        ind = indent_of(lines[idx]) + "\t"
        block = [
            ind + "#if %s == %s\n" % (macro, const),
            ind + '\tstatus = "okay";\n',
            ind + "#else\n",
            ind + '\tstatus = "disabled";\n',
            ind + "#endif\n",
        ]
        for i in reversed(rm):
            del lines[i]
        e2 = e - len(rm)
        # 在 compatible 行之后插入（idx 可能因删除而变化）
        shift = sum(1 for i in rm if i < idx)
        at = idx - shift + 1
        lines[at:at] = block
        done += 1
    return done


def set_haptic_status(lines, masked):
    """AW8697：按所在总线（&i2c7 / &i2c9）条件化 status。"""
    hits = [i for i, l in enumerate(lines)
            if re.search(r'compatible\s*=\s*"' + re.escape(HAPTIC_COMPAT) + r'"', l)]
    done = 0
    for idx in reversed(hits):
        span = node_span(masked, idx)
        if not span:
            continue
        s, e = span
        # 幂等：节点内已有本轮生成的条件块则跳过
        if any(re.search(r"#if\s+ARES_HAPTIC_BUS\b", lines[i])
               for i in range(s, e + 1)):
            continue
        plabel = parent_label(masked, s)
        const = "ARES_HAPTIC_I2C9" if plabel == "i2c9" else "ARES_HAPTIC_I2C7"
        rm = [i for i in range(s, e + 1)
              if re.match(r"^\s*status\s*=", masked[i])]
        ind = indent_of(lines[idx]) + "\t"
        block = [
            ind + "#if ARES_HAPTIC_BUS == %s\n" % const,
            ind + '\tstatus = "okay";\n',
            ind + "#else\n",
            ind + '\tstatus = "disabled";\n',
            ind + "#endif\n",
        ]
        for i in reversed(rm):
            del lines[i]
        shift = sum(1 for i in rm if i < idx)
        at = idx - shift + 1
        lines[at:at] = block
        done += 1
    return done


def find_bus_blocks(masked, raw, child_name):
    """找所有含指定子节点的 &label { ... }; 顶层块，返回 (label, start, end)。"""
    out = []
    for i, l in enumerate(masked):
        m = re.match(r"^&(\S+)\s*\{", l)
        if not m:
            continue
        depth, end = 0, i
        for j in range(i, len(masked)):
            depth += masked[j].count("{") - masked[j].count("}")
            if depth <= 0:
                end = j; break
        if any(child_name in x for x in raw[i:end + 1]):
            out.append((m.group(1), i, end))
    return out


def dedup_haptic(lines):
    """同一 &i2cX 总线上重复叠加的 AW8697 块只保留第一份。

    第一份来自官方 ares-r-oss，且已被 normalize_dt_gpios.py 补上主线标准的
    reset-gpios / irq-gpios；后续块只有厂商私有 reset-gpio / irq-gpio，且
    vib_effect_max 等校准值与官方不同（170 vs 197）。dtc 会把同名子节点静默
    合并、后者覆盖前者，行为不可预期。这里用 cpp '#if 0' 禁用冗余块（数据仍
    保留在源码，删除包裹即可恢复），不做破坏性删除。
    """
    masked = mask_comments(lines)
    blocks = find_bus_blocks(masked, lines, "aw8697_haptic@5a")
    seen, disabled = set(), []
    for label, s, e in blocks:
        if label in seen:
            disabled.append((label, s, e))
        else:
            seen.add(label)
    # 从后往前插入，避免行号漂移
    for label, s, e in reversed(disabled):
        hdr = ("\n/* [去重] 本块是 %s 上 AW8697 的重复叠加定义（另一份见上文，\n"
               " * 带主线标准 *-gpios 契约）；默认不参与编译。数据保留供审计，\n"
               " * 需要改用本批次校准值时把下面的 #if 0 改成 #if 1 即可。 */\n"
               "#if 0\n") % label
        lines[e + 1:e + 1] = ["#endif\n"]
        lines[s:s] = [hdr]
    return len(disabled)


def fix(text):
    lines = text.splitlines(keepends=True)

    # ── 0. 同一总线 AW8697 重复块去重（cpp #if 0 非破坏性禁用）──
    if "#if 0\n" not in lines:
        n = dedup_haptic(lines)
        note("AW8697 重复总线块去重", n)

    # ── 1. status "ok" -> "okay" ──
    n = 0
    for i, l in enumerate(lines):
        new = re.sub(r'(status\s*=\s*)"ok"(\s*;)', r'\1"okay"\2', l)
        if new != l:
            lines[i] = new; n += 1
    note("status ok -> okay", n)

    # ── 2. 插入宏头（最后一个 #include 之后）──
    n = 0
    if "ARES_TOUCH_VARIANT" not in text:
        last = max((i for i, l in enumerate(lines)
                    if l.startswith("#include")), default=-1)
        at = last + 1
        lines[at:at] = ["\n" + MACRO_HEADER + "\n"]
        n = 1
    note("cpp 批次选择器", n)

    # ── 3. 触控互斥 + 4. 触觉互斥 ──
    for compat, const in TOUCH_VARIANTS.items():
        masked = mask_comments(lines)
        n = set_conditional_status(lines, masked, compat,
                                   "ARES_TOUCH_VARIANT", const, occurrence=0)
        note("触控条件化 %s" % compat, n)
    masked = mask_comments(lines)
    n = set_haptic_status(lines, masked)
    note("AW8697 按总线条件化", n)

    # ── 5. 4-cell EINT -> 2-cell（主线 pio 是 #interrupt-cells = <2>）──
    # 厂商格式 <N IRQ_TYPE_xxx N 0> / <N IRQ_TYPE_xxx D 0> 的 3、4 cell 是
    # MTK EINT debounce 私有字段，主线不识别；前 2 cell 才是 <gpio, flags>。
    # 只匹配「第 1 cell 是数字、第 2 cell 是 IRQ_TYPE 宏」的 4-cell 写法，
    # 不会误伤 GIC 的 <GIC_SPI N IRQ_TYPE_xxx> 3-cell 或多组 2-cell 写法。
    pat = re.compile(
        r'(interrupts\s*=\s*<)(\d+)(\s+)(' + IRQ_MACRO + r')(\s+)\d+\s+\d+(>;)')
    joined = "".join(lines)
    joined, n = pat.subn(r'\1\2\3\4\6', joined)
    lines = joined.splitlines(keepends=True)
    note("4-cell -> 2-cell interrupts", n)

    # ── 6. tphy compatible ──
    joined = "".join(lines)
    joined, n = re.subn(r'"mediatek,mt6785-tphy"',
                        '"mediatek,mt6893-tphy"', joined)
    note("tphy compatible -> mt6893", n)

    # ── 7. otg_vbus min/max 统一为 5V ──
    joined, n = re.subn(
        r'(otg_vbus:[\s\S]{0,400}?regulator-min-microvolt\s*=\s*<)4425000(>;'
        r'[\s\S]{0,200}?regulator-max-microvolt\s*=\s*<)5820000(>;)',
        r'\g<1>5000000\g<2>5000000\3', joined, count=1)
    note("otg_vbus 电压统一 5V", n)

    # ── 8. fixedregulator@N -> regulator-N（去掉无 reg 的单元地址）──
    joined, n = re.subn(r'(\b\w+:\s*)fixedregulator@(\d+)\s*\{',
                        r'\1regulator-\2 {', joined)
    note("fixedregulator@N 去单元地址", n)

    # ── 9. irtx_pwm pwm_ch 官方真值 3 ──
    joined, n = re.subn(r'(&irtx_pwm\s*\{[\s\S]{0,300}?pwm_ch\s*=\s*<)0(>;)',
                        r'\g<1>3\2', joined)
    note("irtx pwm_ch -> 官方 3", n)

    # ── 10. chopin 注释清理 ──
    joined, n = re.subn(r'与官方根 dts / chopin 一致',
                        '与官方根 dts 一致', joined)
    joined, n2 = re.subn(
        r'来源：官方 cust_mt6885_msdc.dtsi 语义 \+ chopin 反编译 DTB 的电气值；',
        '来源：官方 cust_mt6885_msdc.dtsi 语义 + 同 SoC 官方 PMIC 电气值；', joined)
    note("chopin 注释清理", n + n2)

    # ── 11. compatible 引号内逗号后的空格 ──
    # 原厂 "awinic, aw2028_i2c" 是笔误：DT spec 的 "vendor,model" 两段之间
    # 不能有空白，dtbs_check 会把整个字符串当成含空格的非法值。
    # 只处理引号内部，故多个 compatible 之间的 ", " 分隔符不受影响。
    joined, n = norm_compat_spaces(joined)
    note("compatible 逗号空格规范化", n)

    # ── 12. 注入主线格式 GPU 节点（Panfrost / arm,mali-bifrost.yaml）──
    # 放在最后：前面所有规范化都已跑完，注入内容不会再被改写。
    # 幂等：以 gpu_opp_table: 标签为哨兵，二次运行原样返回。
    n = 0
    if "gpu_opp_table:" not in joined:
        joined = joined.rstrip() + "\n\n" + MAINLINE_GPU_BLOCK
        n = 1
    note("主线格式 GPU 节点注入", n)

    # ── 13. 相机 EEPROM 对齐主线 at24 binding（不移植重复驱动）──
    # 原厂 5 个节点是标准 I2C EEPROM（reg 0x50/51/52, status=okay），
    # 但节点名 camera_eepromN@.. 与 compatible mediatek,camera_eeprom
    # 都不满足主线 eeprom/at24.yaml（要求 ^eeprom@..$ + 厂商白名单）。
    # 直接主线化：节点名 -> eeprom@ADDR、compatible -> atmel,24c16，
    # 由主线 at24.c 的 of_match 接管并暴露 nvmem 设备；不改内核源码、
    # 不加 compat 补丁，dtbs_check 也完全通过。
    # 容量以 24c16 的 2KB 为保守默认，可在节点加 size = <...> 覆盖
    # （at24.c 第 619-665 行支持 size/pagesize/address-width/num-addresses）。
    # 原厂串保留在行内注释里可追溯；幂等：改写后两个模式都不再命中。
    joined, n1 = re.subn(
        r'(\w+:)\s*camera_eeprom\d*@([0-9a-fA-F]+)\s*\{',
        r'\1 eeprom@\2 {', joined)
    joined, n2 = re.subn(
        r'^([ \t]*)compatible\s*=\s*"mediatek,camera_eeprom"\s*;',
        r'\1/* 原厂 mediatek,camera_eeprom（相机 OTP）；容量可用 size 覆盖 */\n'
        r'\1compatible = "atmel,24c16";',
        joined, flags=re.M)
    note("camera_eeprom 对齐 at24 binding", n2)

    # ── 14. USB host 主线化（外接 USB 网络设备路径）──
    # 幂等：以 mt6893 专属 compatible 为哨兵，二次运行原样返回。
    n = 0
    if "mediatek,mt6893-mtu3" not in joined:
        joined = joined.rstrip() + "\n\n" + MAINLINE_USB_BLOCK
        n = 1
    note("主线格式 USB host 节点注入", n)

    # 第 5 步之后的改动全部在 joined 上，必须返回 joined 而非旧的 lines，
    # 否则 tphy / otg_vbus / regulator / irtx / 注释五项会静默丢失。
    return joined


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    original = src.read_text()
    result = fix(original)
    dst.write_text(result)

    print("=== 主线契约修复审计 ===")
    for tag, n, _ in STATS:
        print("  %-38s %3d 处" % (tag, n))

    again = fix(result)
    print("  幂等自检: " + ("通过" if again == result else "失败"))

    if len(sys.argv) >= 4:
        p = Path(sys.argv[3])
        d = difflib.unified_diff(original.splitlines(keepends=True),
                                 result.splitlines(keepends=True),
                                 fromfile="a/" + src.name,
                                 tofile="b/" + dst.name)
        p.write_text("".join(d))
        print("  补丁: %s" % p)
    return 0


if __name__ == "__main__":
    sys.exit(main())
