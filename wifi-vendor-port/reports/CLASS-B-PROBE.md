# 真实编译探针报告（Linux 7.3.3 + compat + adapted + 平台基线）

由 `tools/compile_probe.py` 生成。用主线内核真实 `KBUILD_CFLAGS` （`clang --target=aarch64-linux-gnu -fsyntax-only`）对适配后源码做语法编译。

> **未启用平台基线层**（`--baseline off`）：干净主线树没有 MTK 平台层，单文件编译卡在「找不到私有头」，此口径复现补齐前状态。

| 指标 | 结果 |
|---|---|
| 抽样编译单元 | 2 / 共 2 个可编译 .c |
| 平台基线层 | 未启用 |
| 无「7.3 已删除旧 API」报错 | **2/2** |
| 完全干净（0 error） | 2 |
| 仅缺头文件（fatal: file not found） | 0 |
| 平台符号/类型错误（头已找到） | 0 |
| 缺头 + 平台符号合计（旧口径） | 0 |
| 仍有 API 残留 | 0 |
| 其它 | 0 |

## 结论

适配后源码在语法层不再出现 `set_fs` / `do_gettimeofday` / `init_timer` / `strlcpy` / `class_create(owner,..)` 等 7.3 已删除 API。

未启用基线时累计 error 为 0 —— 该编译单元不依赖任何 MTK 平台私有头/符号（例如自包含的符号实现），无需平台基线层。

单独验证 compat 头自洽性：`tools/compat_compile_test.sh` （用 7.3.3 头文件 `-fsyntax-only` 编译 `compat/compat_selftest.c`，通过）。

> 本探针对象为 solution/ 下两个自包含实现（platform-symbols.c / framework-symbols.c），仅依赖主线 7.3.3 头与 compat/mtk_framework_compat.h，故在 --baseline off 口径下仍应 0 error；8 个 B 类符号与 EXPORT_SYMBOL_GPL 由上方 nm 步骤核验。

