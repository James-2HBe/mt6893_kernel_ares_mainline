# 真实编译探针报告（Linux 7.3.3 + compat + adapted + 平台基线）

由 `tools/compile_probe.py` 生成。用主线内核真实 `KBUILD_CFLAGS` （`clang --target=aarch64-linux-gnu -fsyntax-only`）对适配后源码做语法编译。

> **本轮已启用平台基线层** `-I baseline/`：由 `tools/build_baseline.py` 从 `MiCode/Xiaomi_Kernel_OpenSource@ares-r-oss` 按固定 commit 保真导出（blob sha 校验）。用于量化「补基线」前后的差异。

| 指标 | 结果 |
|---|---|
| 抽样编译单元 | 120 / 共 530 个可编译 .c |
| 平台基线层 | 已启用 `baseline/` |
| 无「7.3 已删除旧 API」报错 | **120/120** |
| 完全干净（0 error） | 80 |
| 仅缺头文件（fatal: file not found） | 13 |
| 平台符号/类型错误（头已找到） | 27 |
| 缺头 + 平台符号合计（旧口径） | 40 |
| 仍有 API 残留 | 0 |
| 其它 | 0 |

## 剩余缺失头 Top 20

| 头 | 命中单元数 |
|---|---|
| `dt-bindings/mmc/mt6853-msdc.h` | 6 |
| `rt_config.h` | 3 |
| `linux/soc/mediatek/mtk-pm-qos.h` | 2 |
| `mach/emi_mpu.h` | 1 |
| `linux/of_gpio.h` | 1 |

## 结论

适配后源码在语法层不再出现 `set_fs` / `do_gettimeofday` / `init_timer` / `strlcpy` / `class_create(owner,..)` 等 7.3 已删除 API。

启用 ares-r-oss 平台基线层后，「找不到 MTK 平台私有头」的 fatal 数量应显著下降；剩余错误为 4.14 主线删掉的内核头 （`linux/of_gpio.h` 等，属 API 适配）、私有栈内部头、以及链接期才需要的平台导出符号——分类清单见 `reports/BASELINE-EXPORT.md`。

单独验证 compat 头自洽性：`tools/compat_compile_test.sh` （用 7.3.3 头文件 `-fsyntax-only` 编译 `compat/compat_selftest.c`，通过）。

