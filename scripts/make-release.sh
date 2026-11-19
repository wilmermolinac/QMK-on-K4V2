#!/usr/bin/env bash
# Builds every variant and packs the files for a GitHub Release.
# Uso: scripts/make-release.sh v1.0.0
# Resultado en build/release/<version>/:
#   - los 4 .bin, los JSON de VIA y SHA256SUMS.txt
#   - K4V2-QMK-<version>-windows.zip: kit para Windows (flash.cmd, backup-stock.cmd,
#     restore-stock.cmd, firmware/ y via/)
#   - CHECKSUMS.md: tabla de checksums del bootloader para las notas del Release
set -euo pipefail
source "$(dirname "$0")/common.sh"

VER="${1:-}"
[[ "$VER" =~ ^v[0-9]+\.[0-9]+\.[0-9]+$ ]] || die "Uso: scripts/make-release.sh vX.Y.Z"
OUT="$BUILD/release/$VER"
KIT="$BUILD/release/kit-$VER/K4V2-QMK-$VER-windows"

rm -f "$BUILD"/out/*.bin
"$ROOT/scripts/build.sh" default
"$ROOT/scripts/build.sh" via

rm -rf "$OUT" "$BUILD/release/kit-$VER"
mkdir -p "$OUT" "$KIT/firmware" "$KIT/via" "$KIT/scripts/windows"

for f in "$BUILD"/out/keychron_k4_rgb_v2_*.bin; do
    cp "$f" "$OUT/$(basename "$f" .bin)_$VER.bin"
done
cp "$ROOT"/via/*.json "$OUT/"

# Windows kit (CRLF for the Windows text files).
crlf() { perl -pe 's/\r?\n/\r\n/' "$1" > "$2"; }
cp "$OUT"/*.bin "$KIT/firmware/"
cp "$ROOT"/via/*.json "$KIT/via/"
cp "$ROOT/scripts/common.sh" "$KIT/scripts/"
for f in "$ROOT"/scripts/windows/*.ps1; do crlf "$f" "$KIT/scripts/windows/$(basename "$f")"; done
for f in "$ROOT"/scripts/windows/*.cmd; do crlf "$f" "$KIT/$(basename "$f")"; done
crlf "$ROOT/scripts/windows/README-WINDOWS.txt" "$KIT/README-WINDOWS.txt"
(cd "$BUILD/release/kit-$VER" && rm -f "$OUT/K4V2-QMK-$VER-windows.zip" && zip -qr -X "$OUT/K4V2-QMK-$VER-windows.zip" "K4V2-QMK-$VER-windows")

(cd "$OUT" && shasum -a 256 *.bin *.json *.zip > SHA256SUMS.txt)
{
    echo "| File | Bootloader checksum |"
    echo "|---|---|"
    for f in "$OUT"/*.bin; do echo "| \`$(basename "$f")\` | \`$(image_checksum "$f")\` |"; done
} > "$OUT/CHECKSUMS.md"

echo
echo "Release en $OUT (commit $(git -C "$ROOT" rev-parse --short HEAD)):"
ls -1 "$OUT"
