# Shared settings for the helper scripts (sourced, not executed).
# shellcheck shell=bash

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build"
BACKUP="$ROOT/backup"

# SonixQMK commit the firmware overlay was written and tested against.
SONIXQMK_REPO="https://github.com/SonixQMK/qmk_firmware.git"
SONIXQMK_BRANCH="sn32_develop"
SONIXQMK_COMMIT="c9f3803ff1cb910a713c6202cab357268cf98fa6"

# Official QMK toolchain (same one `qmk setup` installs).
TOOLCHAIN_URL_ARM64="https://github.com/qmk/qmk_toolchains/releases/download/v15.2.0-1/qmk_toolchains-gcc15.2.0-host_macosARM64.tar.zst"
TOOLCHAIN_URL_X64="https://github.com/qmk/qmk_toolchains/releases/download/v15.2.0-1/qmk_toolchains-gcc15.2.0-host_macosX64.tar.zst"

SONIXFLASHER="$BUILD/tools/SonixFlasherC/sonixflasher"

# SN32F248B ROM bootloader (keyboard plugged in with the BOOT pads shorted).
BOOTLOADER_VIDPID="0c45/7040"

# Stock Keychron K4 v2 firmware, from the SonixQMK Mechanical-Keyboard-Database.
# checksum = 16-bit sum of the 64 KB image, the same value the bootloader
# reports with `sonixflasher -i`.
STOCK_BASE_URL="https://raw.githubusercontent.com/SonixQMK/Mechanical-Keyboard-Database/main/stockFWs/Keychron/240B"
STOCK_ANSI_FILE="Keychron_K4_RGB_Rev2_ANSI_v1.07_240B.bin"
STOCK_ANSI_SHA256="cf3a2a0b6c5e8d76fb740715e8351dac98c9e89874623b463f1437bcf759bb76"
STOCK_ANSI_CHECKSUM="0x85d5"
STOCK_ISO_FILE="Keychron_K4_RGB_Rev2_ISO_v1.01_240B.bin"
STOCK_ISO_SHA256="b3c6dbd9e0d3bfa149564e0edf49f46a2483aa407de9b34b450f8ce20415ad52"
STOCK_ISO_CHECKSUM="0xf223"

die() { echo "ERROR: $*" >&2; exit 1; }

need_flasher() {
    [ -x "$SONIXFLASHER" ] || die "sonixflasher no está compilado. Ejecuta primero: scripts/setup-macos.sh"
}

# 16-bit little-endian word sum of a file, as the Sonix bootloader computes it.
image_checksum() {
    python3 -I -c '
import sys
d = open(sys.argv[1], "rb").read()
d += b"\xff" * (65536 - len(d)) if len(d) < 65536 else b""
s = 0
for i in range(0, len(d) - 1, 2):
    s = (s + (d[i] | d[i + 1] << 8)) & 0xFFFF
print("0x%04x" % s)
' "$1"
}

wait_for_bootloader() {
    echo "Buscando el teclado en modo bootloader ($BOOTLOADER_VIDPID)..."
    for _ in $(seq 1 30); do
        if "$SONIXFLASHER" -c 2>/dev/null | grep -qi "7040"; then
            return 0
        fi
        sleep 1
    done
    die "No se encontró el bootloader. Desconecta el teclado, puentea los pines BOOT y vuelve a conectarlo."
}
