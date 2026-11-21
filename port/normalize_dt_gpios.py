#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0
"""
normalize_dt_gpios.py — 把 ares 厂商设备树里的 MTK pio 三元组标准化成
主线 devm_gpiod_get() 可直接使用的 *-gpios 属性。

关键事实（本轮核实）：
  主线 7.3.3 有 drivers/pinctrl/mediatek/pinctrl-mt6893.c，且目标仓库
  mt6893.dtsi 的 pio 节点是：
      gpio-controller;
      #gpio-cells = <0x02>;        <-- 2 cells
      #interrupt-cells = <0x02>;
  因此厂商写法 <&pio 129 0> 本来就是合法的 2-cell GPIO specifier。
  上一轮判定「pio 三元组无法解析」是错的：真正缺的只是**标准属性名**。
  厂商用 reset-gpio / irq-gpio / gpio-rst-std / key_left 等私有属性名，
  主线驱动读的是 reset-gpios / irq-gpios / key-gpios / *-gpios。

本脚本（非破坏性、幂等、去重）：
  - 对每个目标 compatible，只在其**第一次出现**的节点里，在 compatible
    行之后插入标准 *-gpios 属性（值 = 同一个 <&pio N flags>）。
  - 保留所有厂商私有属性与 4-cell interrupts 原样：
      * 私有属性供厂商兼容层使用，不改动；
      * 4-cell EINT 在主线内核里 of_irq_get() 会失败，但本轮所有驱动都
        改用 gpiod_to_irq(irq-gpios) 取 IRQ，不依赖 interrupts 属性，
        注释掉反而会破坏 dtc 对 phandle 引用的解析，故保留。
  - 缩进与 compatible 行对齐，补丁可读。

用法：
  python3 port/normalize_dt_gpios.py <dts-in> <dts-out> [<patch-out>]

实机 pin 全部来自 ares 官方/lineage 设备树，未编造：
  gt9886@5d        goodix,reset-gpio=<&pio 20>, irq-gpio=<&pio 21>
  fingerprint@0    &goodix_fp: GPIO14(reset) / GPIO15(eint), fpc,gpio_irq=<&pio 15>
  aw8697_haptic@5a reset-gpio=<&pio 129>, irq-gpio=<&pio 22>
  st21nfc@08       gpio-rst-std=<&pio 10>, gpio-irq-std=<&pio 11>
  gamekey          key_left=pio111 key_right=pio112 hall_left=pio4 hall_right=pio5
  xmusb350         rst=pio28 sda=pio205 scl=pio204 therm=pio29
"""
import difflib
import re
import sys
from pathlib import Path

# compatible -> 标准属性（相对 compatible 行的缩进额外加一个 tab）
NORMALIZE = {
    "goodix,gt9886": [
        "reset-gpios = <&pio 20 0x0>;\t/* 官方 goodix,reset-gpio = <&pio 20 0x0> */",
        "irq-gpios = <&pio 21 0x0>;\t/* 官方 goodix,irq-gpio = <&pio 21 0x0> */",
    ],
    "goodix,goodix-fp": [
        "reset-gpios = <&pio 14 0x0>;\t/* &goodix_fp pinctrl GPIO14 */",
        "irq-gpios = <&pio 15 0x0>;\t/* &goodix_fp fpc,gpio_irq = <&pio 15 0> */",
    ],
    "awinic,aw8697_haptic": [
        "reset-gpios = <&pio 129 0>;\t/* 官方 reset-gpio = <&pio 129 0> */",
        "irq-gpios = <&pio 22 0>;\t/* 官方 irq-gpio = <&pio 22 0> */",
    ],
    "mediatek,nfc": [
        "reset-gpios = <&pio 10 0x0>;\t/* 官方 gpio-rst-std = <&pio 10 0x0> */",
        "irq-gpios = <&pio 11 0x00>;\t/* 官方 gpio-irq-std = <&pio 11 0x00> */",
    ],
    "xm,gamekey": [
        "key-gpios = <&pio 111 0x00>, <&pio 112 0x00>;\t/* key_left/key_right */",
        "hall-gpios = <&pio 4 0x00>, <&pio 5 0x00>;\t/* hall_left/hall_right */",
    ],
    "xm,xmusb350-charger": [
        "reset-gpios = <&pio 28 0x00>;\t/* xm,xmusb350_rst_gpio */",
        "irq-gpios = <&pio 110 0x00>;\t/* interrupts = <110 ...> -> gpiod */",
        "sda-gpios = <&pio 205 0x00>;\t/* xm,xmusb350_sda_gpio */",
        "scl-gpios = <&pio 204 0x00>;\t/* xm,xmusb350_scl_gpio */",
        "therm-gpios = <&pio 29 0x00>;\t/* mi,connect_therm */",
    ],
}

COMPAT_RE = re.compile(r'^(\s*)compatible\s*=\s*"([^"]+)"')


def _node_already_normalized(lines, start, compat):
    """Look ahead from the compatible line until the node closes; if the first
    target property is already present, the node was normalized in a previous
    run. This makes the transform idempotent."""
    first_prop = NORMALIZE[compat][0].split("=", 1)[0].strip()
    depth = 0
    for i in range(start, min(start + 40, len(lines))):
        line = lines[i]
        if i > start and re.match(r'^\s*' + re.escape(first_prop) + r'\s*=',
                                  line):
            return True
        depth += line.count("{") - line.count("}")
        if i > start and depth < 0:
            break
    return False


def normalize(text: str):
    lines = text.splitlines(keepends=True)
    out = []
    seen = set()
    inserted = {k: 0 for k in NORMALIZE}

    for idx, line in enumerate(lines):
        m = COMPAT_RE.match(line)
        if m and m.group(2) in NORMALIZE and m.group(2) not in seen:
            compat = m.group(2)
            seen.add(compat)
            out.append(line)
            if _node_already_normalized(lines, idx, compat):
                continue
            indent = m.group(1) + "\t"
            for prop in NORMALIZE[compat]:
                out.append(indent + prop + "\n")
            inserted[compat] += 1
            continue
        out.append(line)

    return "".join(out), inserted


def main():
    if len(sys.argv) < 3:
        print("用法: normalize_dt_gpios.py <dts-in> <dts-out> [<patch-out>]")
        return 2
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    original = src.read_text()
    result, inserted = normalize(original)

    if result == original:
        print("  已是标准化状态，无需修改")
    dst.write_text(result)
    print(f"  已写出 {dst}")

    for compat, n in inserted.items():
        status = "OK  " if n == 1 else "MISS"
        print(f"  {status} {compat:28s} 插入 {n} 处（按 compatible 去重）")

    if len(sys.argv) >= 4:
        patch = Path(sys.argv[3])
        diff = difflib.unified_diff(
            original.splitlines(keepends=True),
            result.splitlines(keepends=True),
            fromfile=f"a/{src.name}", tofile=f"b/{dst.name}")
        patch.write_text("".join(diff))
        print(f"  补丁已写出 {patch}")

    # 幂等自检
    again, _ = normalize(result)
    print("  幂等自检: " + ("通过（重复运行不再变化）" if again == result else "失败"))
    return 0


if __name__ == "__main__":
    sys.exit(main())
