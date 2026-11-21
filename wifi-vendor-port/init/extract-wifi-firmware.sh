#!/system/bin/sh
# SPDX-License-Identifier: GPL-2.0
# extract-wifi-firmware.sh — init 阶段从原厂 vendor 分区提取 WiFi/connectivity
# 固件到 /lib/firmware，供主线 request_firmware() 加载。
#
# 由 init.rc 在 wlan/conninfra 驱动加载【之前】调用：
#   on early-init / on init
#     exec u:r:vendor_modprobe:s0 root root -- /vendor/bin/extract-wifi-firmware.sh
#
# 也可以手工在 root shell 跑一次验证。
#
# 设计要点（见 ../firmware/FIRMWARE-LIST.md）：
#  - 不硬编码具体 eco/flavor 组合：把 vendor firmware 目录里所有 WiFi/combo
#    固件按【原名】整包软链或拷贝到 /lib/firmware，让驱动按它自己的候选名
#    （WIFI_RAM_CODE_soc3_0_1_1.bin / soc3_0_*_hdr.bin / WMT.cfg ...）去命中。
#  - 原厂固件路径常见 /vendor/firmware、/vendor/etc/firmware、/etc/firmware；
#    Android 还可能在 /vendor/firmware_mnt/image（动态分区）。全部纳入搜索。
#  - 若 /lib/firmware 不可写（只读 system），改用 /vendor/lib/firmware 并
#    通过内核参数 firmware_class.path 告知；两者都尝试。
#  - 保持文件【原名】，不裁剪 _hdr.bin 头，不改权限为可执行。

TAG="wifi-fw"
VENDOR_DIRS="
/vendor/firmware
/vendor/etc/firmware
/vendor/firmware_mnt/image
/etc/firmware
/firmware
/system/vendor/firmware
"
# 只搬这些前缀/关键文件，避免把整包 vendor 的无关大文件全量复制
FW_PATTERNS="
WIFI_RAM_CODE
WIFI_RAM_CODE2
soc3_0_
mt66xx_patch
WMT.cfg
WMT_SOC.cfg
bt.cfg
BT_RAM_CODE
btPowerTable
"

log() { echo "$TAG: $*"; }

# 目标目录：优先 /lib/firmware，不可写退到 /vendor/lib/firmware
pick_dest() {
    for d in /lib/firmware /vendor/lib/firmware /data/local/tmp/firmware; do
        if [ -d "$d" ] || mkdir -p "$d" 2>/dev/null; then
            if [ -w "$d" ]; then echo "$d"; return 0; fi
        fi
    done
    return 1
}

DEST="$(pick_dest)" || { log "ERROR: 无可写固件目录"; exit 1; }
log "目标目录: $DEST"

copied=0
for vd in $VENDOR_DIRS; do
    [ -d "$vd" ] || continue
    log "扫描: $vd"
    for pat in $FW_PATTERNS; do
        # 用 find 精确匹配前缀，避免 ls/wildcard 在无匹配时报错
        find "$vd" -maxdepth 3 -type f -name "${pat}*" 2>/dev/null | while read src; do
            base="$(basename "$src")"
            dst="$DEST/$base"
            if [ -e "$dst" ]; then
                continue
            fi
            # 优先软链（省空间、零拷贝）；失败再 cp
            if ln -s "$src" "$dst" 2>/dev/null; then
                log "link  $base  <- $src"
            elif cp -f "$src" "$dst" 2>/dev/null; then
                chmod 0644 "$dst" 2>/dev/null
                log "copy  $base  <- $src"
            else
                log "WARN  无法放置 $base"
            fi
        done
    done
done

# WMT 侧有些固件在 vendor 里叫别的名字，兜底：把 vendor firmware 根目录下
# 所有 .bin/.cfg/.dat 且体积合理(<16MB)的文件也纳入（仍保持原名）
for vd in $VENDOR_DIRS; do
    [ -d "$vd" ] || continue
    find "$vd" -maxdepth 1 -type f \
        \( -name '*.bin' -o -name '*.cfg' -o -name '*.dat' \) \
        -size -16M 2>/dev/null | while read src; do
        base="$(basename "$src")"
        [ -e "$DEST/$base" ] && continue
        ln -s "$src" "$DEST/$base" 2>/dev/null && log "link  $base (兜底)"
    done
done

log "已就位文件数: $(find "$DEST" -maxdepth 1 \( -type f -o -type l \) 2>/dev/null | wc -l)"

# 若目标不是 /lib/firmware，写一条提示：需要在 cmdline 加
#   firmware_class.path=/vendor/lib/firmware
case "$DEST" in
    /lib/firmware) : ;;
    *) log "注意: 非默认路径，请在内核 cmdline 加 firmware_class.path=$DEST" ;;
esac

# 触发 uevent，让已等 firmware 的设备重试（若驱动先于本脚本 probe 过）
echo add > /sys/firmware/uevent 2>/dev/null || true
# Android 通过 wmt 字符设备下发 NVRAM 校准数据的动作属于 vendor HAL，
# 不在本脚本范围（需该机原厂 NVRAM 分区数据）。

log "完成。驱动加载后以 dmesg | grep -i 'kalFirmwareImageMapping\|WMT\|trying file' 核对命中文件名"
exit 0
