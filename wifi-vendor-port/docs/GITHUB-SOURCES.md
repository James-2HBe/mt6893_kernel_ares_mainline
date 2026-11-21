# GitHub 上 ares / MT6893 内核源码仓库与分支调研

本轮目标：找一个比 `ares-r-oss` 更"有利于移植"的 ares 内核源，把上轮
遗留的 **33 个链接期平台符号实现** 与 **残余 MT6893 变体头** 真正补上，
并做 4.14 → 7.3.3 的 API 适配。

## 1. 搜到的仓库与分支（2026-11-14 实测）

| 仓库 | 默认分支 / 相关分支 | 规模 | 特点 | 本轮用途 |
|---|---|---|---|---|
| `MiCode/Xiaomi_Kernel_OpenSource` | `ares-r-oss` | 73519 文件 | 小米官方 GPL 树；`connectivity/` 仅 6 个空壳 adapter；只带 `mt6885-*` 变体头 | **上轮源**：36 个平台头 + 182 个构建文件 |
| `seriaTvT/kernel_mt6893` | `chopin_kvm` | 1.27 GB | 完整 MTK connectivity 私有栈，但平台层被裁剪 | **上轮源**：WiFi 私有栈 1570 文件 |
| **`xiaomi-mt6893-dev/kernel_xiaomi_mt6893`** | **`lineage-23.2`**（另有 21 / 22.2 / 23.0 / 23.2-ksu） | **75912 文件 / 1.41 GB**，2026-07 维护 | 覆盖 `pissarro(mt6877) / agate(mt6893) / ares(mt6893) / choppin(mt6891)`；`connectivity/` 完整（**473 个 `.c`**）；大量 `BACKPORT:` / `UPSTREAM:` 高版本回移 | **本轮主移植源** |
| `pissarro-dev/kernel_xiaomi_mt6877` | `lineage-22.2` | 1.39 GB | 同组织旧分支，多机型合并 | 备选 |
| `AbzRaider/android_kernel_xiaomi_mt6893` | `lineage-21` / `AresX` / `ares-dev` / `ares/inline` / `android-15` / `vcodec-4.19` | 1.27 GB | 分支最多，含实验性 4.19 分支 | 备选 |
| `xiao-ne/xiaomi_kernel_chopin` | `backport-4.14.336-A15-etc` / `A16` / `A17` | 238 MB | 明确以 "backport-4.14.336" 命名，把高版本内核改动回移到 4.14 | **API 回移写法参考** |
| `freyjawionq/kernel_chopin_mt6893` | `inc_ksu` | 1.17 GB | KernelSU 集成 | 备选 |
| `Tkpointz/android_kernel_aresin` | `lxr` | 1.23 GB | POCO F3 GT / K40 GE（D1200） | 参考 |
| `redrizard/kernel_xiaomi_mt6893` | — | — | 与 `xiaomi-mt6893-dev` 同源码的 fork | 镜像 |
| `SysAsh/ares_kernel_xiaomi_mt6893` | `aex9` | — | 老 4.14 分支 | 参考 |
| `xiaomi-mt6893-dev/rcu_mt6893` | — | 小 | "RCU Backports for MT6893" | API 回移参考 |

## 2. 为什么最终选 `lineage-23.2`

三条硬指标，均经实测：

1. **私有栈完整**。`.gitignore` 有 `connectivity: Import from MediaTek kernel_modules`，
   `drivers/misc/mediatek/connectivity/` 共 **1221 文件 / 473 个 `.c`**；
   而 `ares-r-oss` 的 `connectivity/` 只有 6 个空壳，`seriaTvT` 是裁剪树。
2. **变体头最全**。相对 `ares-r-oss` 多 **5032 个路径（2291 个头 / 2073 个源）**。
   上轮 47 个未定位 include 中的 `clk-mt6873-pg.h`、`helio-dvfsrc-opp-v6877.h`、
   `mt6873/mt6877/mt6853` 系列 `dt-bindings`（clock / memory / mmc）都在这棵树里，
   `ares-r-oss` 一个都没有。
3. **API 写法更接近 7.3**。该分支把高版本改动成批回移：`sizeof_field()`、
   `fuse: convert timespec to timespec64`、`iov_iter` accessor、
   `__vmalloc` 去 `pgprot`、`kernfs` 对象创建、bpf / cgroup / mm 等。
   实测其 18 个平台实现 `.c` 里 **没有** `set_fs` / `strlcpy` /
   `access_ok(VERIFY_*)` / `class_create(THIS_MODULE)` 残留，
   编译探针 `API=0`；这直接把"修 API"的工作量从重写签名降为
   **补依赖头 + 补 `EXPORT_SYMBOL_GPL`**。

平台实现也齐全：`drivers/memory/mediatek/emimpu.c`（7 个 `mtk_emimpu_*`）、
`helio-dvfsrc-v3/helio-dvfsrc-opp-mt6893.c`、`ppm_v3/src/mtk_ppm_api.c`、
`clkbuf_v1/mt6873/mtk_clkbuf_hw.c`、`clk-mt6873-pg.c`、
`mtk_iommu_ext.c`、`pseudo_m4u/`、`mtu3/mtu3_dr.c` 等。

> 上轮 README 里"小米仓不能作源"的结论只对 **WiFi 驱动** 成立（空壳），
> 对 **平台基线层** 不成立——已在上一轮更正；本轮进一步用 Lineage 合并树
> 补上了它缺的 MT6893 变体与全部平台实现。

## 3. 固定来源与保真校验

- 仓库：`xiaomi-mt6893-dev/kernel_xiaomi_mt6893`
- 分支：`lineage-23.2`
- **commit：`aeb1327b1c73dfe71c4d0e184e5c1c419ee7291f`**
- 文件清单：`git ls-tree -r --name-only HEAD` → 75912 条
- 逐文件校验：`sha1(b"blob <size>\0" + data)` 与 `git ls-tree` 的 blob sha 比对
  （例：`drivers/memory/mediatek/emimpu.c` = `7dc3ccb9…`，26142 字节，一致）
- 清单落在 `tools/platform-impl-manifest.tsv`

## 4. 网络通道经验（本沙箱实测）

| 通道 | 结果 |
|---|---|
| `git ls-remote` 对 1.2–1.4 GB 大仓 | **超时返回空**（4 个仓库全空） |
| GitHub Trees API `?recursive=1` | **超时**（大仓 tree JSON 过大） |
| `raw.githubusercontent.com` | **ReadTimeout 31.5 s**（上轮 ares-r-oss 曾可用，本轮失效） |
| `github.com/<repo>/raw/<sha>/<path>` | 可用但慢（17 s） |
| **`cdn.jsdelivr.net/gh/<repo>@<sha>/<path>`** | **1.2 s / 字节与 blob sha 一致 ✅ 主通道** |
| `fastly.jsdelivr.net` | 1.4 s ✅ 备用 |
| `git clone --filter=blob:none --no-checkout --depth=1 --single-branch` | 成功（4m16s，75912 文件），用于拿 commit 与 blob index |
| `git cat-file --batch` 批量读 blob | 不用：promisor 惰性拉取触发 GnuTLS 断连（上一轮已证） |

`tools/port_platform_impl.py` 的 `make_fetcher()` 封装了「jsdelivr → fastly →
github raw」三级回退 + 逐文件 sha 校验，本轮 397 + 1672 + 210 + 47 次下载
全部走通（成功率 396/397 起）。

## 5. 复现

```bash
# 稀疏克隆拿清单与 blob index
git clone --filter=blob:none --no-checkout --depth=1 --single-branch \
  --branch lineage-23.2 \
  https://github.com/xiaomi-mt6893-dev/kernel_xiaomi_mt6893.git /tmp/mt6893-lin232
cd /tmp/mt6893-lin232 && git ls-tree -r --name-only HEAD > /tmp/files_lin232.txt

# 导出平台符号实现与变体头
python3.11 tools/port_platform_impl.py \
  --repo /tmp/mt6893-lin232 --files /tmp/files_lin232.txt \
  --modules reports/BASELINE-MODULES.md \
  --export-report reports/BASELINE-EXPORT.md \
  --out baseline --report reports/PLATFORM-IMPL.md \
  --manifest tools/platform-impl-manifest.tsv

# BFS 补依赖头 → API 适配 → 编译探针
python3.11 tools/complete_impl_headers.py --repo /tmp/mt6893-lin232 \
  --files /tmp/files_lin232.txt --kernel /path/to/linux-7.3.3 --out baseline
python3.11 tools/adapt_platform_impl.py
python3.11 tools/compile_probe.py --kernel /path/to/linux-7.3.3 \
  --adapted baseline/impl-adapted --compat compat --baseline auto --limit 30
```
