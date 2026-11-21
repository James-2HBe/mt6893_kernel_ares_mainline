#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
生成「主线驱动补 ares compatible」的补丁，**不修改内核树**。

用法: python3 make_compat_patches.py [内核源码目录]
输出: port/compat/*.patch
用法（应用）: cd <目标仓库> && git apply <补丁>

原则：A 类驱动（主线已有且真实可用）不重复移植，只在 of_match 里补
ares 设备树用的 compatible 别名，让主线驱动能 probe。
"""
import difflib
import pathlib
import sys

K = pathlib.Path(sys.argv[1] if len(sys.argv) > 1
                 else "/workspace/linux-7.3.3")
OUT = pathlib.Path(__file__).resolve().parent / "compat"
OUT.mkdir(parents=True, exist_ok=True)

# (补丁文件名, 相对路径, 锚点行, 要插入的行, 说明)
PATCHES = [
    (
        "bq25980-add-ares-compatible.patch",
        "drivers/power/supply/bq25980_charger.c",
        '\t{ .compatible = "ti,bq25980", .data = (void *)BQ25980 },',
        [
            '\t{ .compatible = "ti,bq2597x-master", .data = (void *)BQ25980 },',
            '\t{ .compatible = "ti,bq2597x-slave", .data = (void *)BQ25980 },',
        ],
        "67W 双电荷泵：ares 板级 dts 用 ti,bq2597x-master / "
        "ti,bq2597x-slave，\n主线 bq25980 属同族，补两条 compatible 即可复用"
        "（无需搬 MTK 私有 charger 框架）。\n注意 bq2597x 与 bq25980 寄存器"
        "差异需真机验证。",
    ),
    (
        "bq27xxx-add-ares-compatible.patch",
        "drivers/power/supply/bq27xxx_battery_i2c.c",
        '\t{ .compatible = "ti,bq27z561" },',
        [
            '\t{ .compatible = "ti,bq27z561_i2c0" },',
            '\t{ .compatible = "ti,bq27z561_i2c9" },',
        ],
        "电量计：主线已有 ti,bq27z561 / BQ27Z561 芯片定义，\nares 板级 dts "
        "用了带总线后缀的 ti,bq27z561_i2c0 / _i2c9，补两条别名即可复用。",
    ),
]


def main():
    print(f"内核树: {K}")
    print(f"输出  : {OUT}")
    for name, rel, anchor, inserts, note in PATCHES:
        src = K / rel
        if not src.exists():
            print(f"  [跳过] {name}: 源文件不存在 {rel}")
            continue
        text = src.read_text(encoding="utf-8")
        if anchor not in text:
            print(f"  [跳过] {name}: 找不到锚点（内核版本不同？）")
            continue
        if inserts[0] in text:
            print(f"  [跳过] {name}: 补丁已应用过")
            continue

        new = text.replace(anchor, anchor + "\n" + "\n".join(inserts), 1)
        body = "".join(difflib.unified_diff(
            text.splitlines(keepends=True),
            new.splitlines(keepends=True),
            fromfile="a/" + rel,
            tofile="b/" + rel,
            n=3,
        ))
        header = ("# " + note.replace("\n", "\n# ")
                  + "\n# 由 make_compat_patches.py 生成；源内核树不被修改。\n"
                  + "# 用法: git apply <本补丁>\n\n")
        (OUT / name).write_text(header + body, encoding="utf-8")
        print(f"  [生成] {name}  (+{len(inserts)} 条 compatible)")

    print("\n提示：这两个补丁属于 A 类复用，不是新驱动。")


if __name__ == "__main__":
    main()
