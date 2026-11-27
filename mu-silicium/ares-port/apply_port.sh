#!/usr/bin/env bash
# 一键把 ares (MT6893) UEFI 移植应用到官方 Mu-Silicium 仓库
# 用法：在 Mu-Silicium 仓库根目录执行：
#   bash /path/to/apply_port.sh /path/to/Mu-Silicium
set -euo pipefail
PORT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO="${1:-$PWD}"
if [ ! -d "$REPO/Platforms" ] || [ ! -f "$REPO/build_uefi.py" ]; then
  echo "错误：$REPO 不像 Mu-Silicium 仓库根目录" >&2
  exit 1
fi
echo "应用 ares 移植 -> $REPO"
cp -v "$PORT_DIR/Platforms/Xiaomi/aresPkg/DeviceBuild.py"  "$REPO/Platforms/Xiaomi/" 2>/dev/null || { mkdir -p "$REPO/Platforms/Xiaomi/aresPkg"; cp -v "$PORT_DIR/Platforms/Xiaomi/aresPkg/DeviceBuild.py" "$REPO/Platforms/Xiaomi/aresPkg/"; }
mkdir -p "$REPO/Platforms/Xiaomi/aresPkg"
cp -rv "$PORT_DIR/Platforms/Xiaomi/aresPkg/." "$REPO/Platforms/Xiaomi/aresPkg/"
mkdir -p "$REPO/Silicon/MediaTek/MT6893Pkg"
cp -rv "$PORT_DIR/Silicon/MediaTek/MT6893Pkg/." "$REPO/Silicon/MediaTek/MT6893Pkg/"
mkdir -p "$REPO/Resources/Configs" "$REPO/Resources/DTBs"
cp -v "$PORT_DIR/Resources/Configs/ares.toml" "$REPO/Resources/Configs/"
cp -v "$PORT_DIR/Resources/DTBs/ares.dtb" "$REPO/Resources/DTBs/"
echo "完成。接下来："
echo "  cd $REPO"
echo "  python3 build_uefi.py -d ares"
