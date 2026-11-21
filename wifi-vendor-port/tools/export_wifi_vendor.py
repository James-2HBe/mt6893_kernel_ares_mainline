#!/usr/bin/env python3
"""复现本包的驱动源码导出（保真，不做任何内容改写）。

用途：在能联网的机器上，按固定 commit 从原厂仓库重新导出 wifi 私有栈。
依赖：requests（pip3 install requests）。
用法：python3 export_wifi_vendor.py <输出目录>

说明：
  - 源 = seriaTvT/kernel_mt6893 @ c651f31e (chopin_kvm)，4.14 vendor kernel。
  - 导出范围见 SCOPE_PREFIXES；清单来自 GitHub Git Trees API（recursive=1）。
  - 每个 worker 复用一个长连接 requests.Session：首建连较慢(~50s)，
    之后约 0.8s/文件；切勿改成「每文件新建连接」，会慢一个数量级。
  - 只搬源码/DT/配置文本，不含任何固件二进制、不含编译产物。
"""
import os, sys, time, json, threading, urllib.request
from concurrent.futures import ThreadPoolExecutor, as_completed

try:
    import requests
    from requests.adapters import HTTPAdapter
except ImportError:
    sys.exit("需要 requests: pip3 install requests")

REPO = "seriaTvT/kernel_mt6893"
SHA = "c651f31ec9ab362f54076d5e99ba5dd1f6582a3d"
RAW = f"https://raw.githubusercontent.com/{REPO}/{SHA}/"
TREE_API = f"https://api.github.com/repos/{REPO}/git/trees/{SHA}?recursive=1"
WORKERS = 6
TIMEOUT = 90

# WiFi 主链路 + 私有依赖 + SDIO host（参考）+ 板级 DT + defconfig
SCOPE_PREFIXES = (
    "drivers/misc/mediatek/connectivity/",
    "drivers/misc/mediatek/btif/",
    "drivers/misc/mediatek/conn_md/",
    "drivers/misc/mediatek/include/",
    "drivers/misc/mediatek/emi/",
    "drivers/misc/mediatek/emi_mpu/",
    "drivers/misc/mediatek/emi_bwl/",
    "drivers/misc/mediatek/devmpu/",
    "drivers/misc/mediatek/mddp/",
    "drivers/mmc/host/mediatek/ComboA/",
    "drivers/mmc/host/wmt-sdmmc.c",
    "arch/arm64/boot/dts/mediatek/mt6893",
    "arch/arm64/boot/dts/mediatek/ares",
    "arch/arm64/boot/dts/mediatek/chopin",
    "arch/arm64/boot/dts/mediatek/xiaomi-mt6893-common",
    "arch/arm64/boot/dts/mediatek/cust_mt6893_msdc",
    "arch/arm64/configs/ares_defconfig",
    "arch/arm64/configs/chopin_defconfig",
)

_local = threading.local()
def session():
    s = getattr(_local, "s", None)
    if s is None:
        s = requests.Session()
        s.mount("https://", HTTPAdapter(pool_connections=2, pool_maxsize=2, max_retries=0))
        _local.s = s
    return s


def get_paths():
    req = urllib.request.Request(TREE_API, headers={"User-Agent": "curl/8"})
    with urllib.request.urlopen(req, timeout=60) as r:
        tree = json.load(r)
    if tree.get("truncated"):
        print("警告: Trees API truncated=True，清单可能不全", file=sys.stderr)
    return [x["path"] for x in tree["tree"]
            if x["type"] == "blob" and x["path"].startswith(SCOPE_PREFIXES)]


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "wifi-vendor-export"
    os.makedirs(out, exist_ok=True)
    paths = get_paths()
    print(f"清单 {len(paths)} 文件 @ {SHA[:12]} -> {out}", flush=True)

    def fetch(p):
        dest = os.path.join(out, p)
        for i in range(3):
            try:
                r = session().get(RAW + p, timeout=TIMEOUT)
                if r.status_code == 200 and r.content:
                    os.makedirs(os.path.dirname(dest), exist_ok=True)
                    with open(dest, "wb") as f:
                        f.write(r.content)
                    return (p, len(r.content), None)
                err = f"HTTP {r.status_code}"
            except Exception as e:
                err = f"{type(e).__name__}: {e}"
                _local.s = None
            time.sleep(1 + i)
        return (p, 0, err)

    ok, failed, t0 = 0, [], time.time()
    with ThreadPoolExecutor(max_workers=WORKERS) as ex:
        futs = {ex.submit(fetch, p): p for p in paths}
        for i, fu in enumerate(as_completed(futs), 1):
            p, size, err = fu.result()
            if err: failed.append((p, err))
            else: ok += 1
            if i % 200 == 0:
                print(f"  {i}/{len(paths)} ok={ok} fail={len(failed)} {time.time()-t0:.0f}s", flush=True)
    print(f"完成 ok={ok}/{len(paths)} fail={len(failed)} {time.time()-t0:.0f}s", flush=True)
    for p, e in failed:
        print(f"  FAIL {p}\t{e}")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
