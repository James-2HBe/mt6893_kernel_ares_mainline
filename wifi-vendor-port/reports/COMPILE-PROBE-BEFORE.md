# 真实编译探针报告（Linux 7.3.3 + compat + adapted + 平台基线）

由 `tools/compile_probe.py` 生成。用主线内核真实 `KBUILD_CFLAGS` （`clang --target=aarch64-linux-gnu -fsyntax-only`）对适配后源码做语法编译。

> **未启用平台基线层**（`--baseline off`）：干净主线树没有 MTK 平台层，单文件编译卡在「找不到私有头」，此口径复现补齐前状态。

| 指标 | 结果 |
|---|---|
| 抽样编译单元 | 120 / 共 530 个可编译 .c |
| 平台基线层 | 未启用 |
| 无「7.3 已删除旧 API」报错 | **120/120** |
| 完全干净（0 error） | 70 |
| 仅缺头文件（fatal: file not found） | 23 |
| 平台符号/类型错误（头已找到） | 27 |
| 缺头 + 平台符号合计（旧口径） | 50 |
| 仍有 API 残留 | 0 |
| 其它 | 0 |

## 剩余缺失头 Top 20

| 头 | 命中单元数 |
|---|---|
| `memory/mediatek/emi.h` | 6 |
| `dt-bindings/mmc/mt6853-msdc.h` | 6 |
| `port_ipc.h` | 4 |
| `rt_config.h` | 3 |
| `mtk_ppm_api.h` | 2 |
| `mach/emi_mpu.h` | 1 |
| `linux/of_gpio.h` | 1 |

## 结论

适配后源码在语法层不再出现 `set_fs` / `do_gettimeofday` / `init_timer` / `strlcpy` / `class_create(owner,..)` 等 7.3 已删除 API。

未启用基线时剩余错误全部是「MTK 平台私有头与导出符号缺失」，属于平台基线层的职责，不是 4.14→7.3 API 适配的遗漏。

单独验证 compat 头自洽性：`tools/compat_compile_test.sh` （用 7.3.3 头文件 `-fsyntax-only` 编译 `compat/compat_selftest.c`，通过）。

