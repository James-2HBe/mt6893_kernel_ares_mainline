# API 适配改写报告（原厂 4.14/GKI-5.x → Linux 7.3.3）
由 `tools/adapt_api.py` 生成，幂等可复跑。源文件 `drivers/` 保持保真原件，适配结果在 `adapted/`（同目录层级）。
- 扫描源文件：**1518**
- 改动文件：**529**
- 原样跳过（uboot/virtual-OS 非内核编译单元）：**101**

## 改写规则与命中
| 规则 | 说明 | 命中文件 | 命中次数 | 依据 |
|---|---|---|---|---|
| R1 | class_create 去 owner 参数 | 26 | 29 | 7.3: struct class *class_create(const char *name) |
| R2 | access_ok 去 VERIFY 首参 | 3 | 7 | 7.3: access_ok(addr, size); 变量含义在 adapt 前已确认 |
| R3a | vfs_read -> kernel_read | 5 | 5 | 7.3: 内核缓冲场景，kernel_read(file, buf, count, pos)；排除 *image 首参 |
| R3b | vfs_write -> kernel_write | 5 | 5 | 7.3: 同上；排除 *image 首参 |
| R4 | strlcpy -> strscpy | 3 | 5 | 7.3 删除 strlcpy；返回值语义差异见报告 |
| R5 | dma_zalloc_coherent -> dma_alloc_coherent | 2 | 3 | 7.3 删除前者，后者自带零初始化 |
| R7 | 注入 compat 头引用 | 529 | 529 | 用 -I compat/ 或 ccflags -include |

## 由 compat 头补齐（不在本脚本改动）

`set_fs/get_fs/get_ds/mm_segment_t/KERNEL_DS/USER_DS`、`struct timeval`、`do_gettimeofday()`、`VERIFY_READ/VERIFY_WRITE`、`strlcpy` 兜底、`dma_zalloc_coherent` 兜底、`create_proc_entry` 编译桩。详见 `compat/mtk_wifi_compat.h`，已用 7.3.3 头文件 `-fsyntax-only` 验证。

## 社区已分流、本轮不改（7.3 走新分支）

`init_timer`→`timer_setup` + 回调 `from_timer`（12 处，11 处在 `KERNEL_VERSION(4,15,0)` 的 `#else` 死分支）、`file_operations`→`proc_ops`（85 处 fops，5.6 版本宏分流）、`timespec64`（BT/wlan 部分已适配）。这些在 7.3 的 `LINUX_VERSION_CODE` 下编译的是新分支，盲改反而会破坏版本兼容。

## 改动示例（截取新增行）

**`misc/mediatek/btif/common/btif_dma_plat.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/btif/common/btif_plat.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/btif/common/mtk_btif.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
+	p_btif_class = class_create(p_btif_dev_name);
```

**`misc/mediatek/btif/common/mtk_btif_exp.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md_dbg.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md_dev.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md_dump.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md_exp.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```

**`misc/mediatek/conn_md/conn_md_log.c`**

```c
+/* Added by adapt_api.py: 4.14/5.x -> 7.3 API compat shim */
+#include "mtk_wifi_compat.h"
```
