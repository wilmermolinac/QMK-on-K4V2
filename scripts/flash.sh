#!/usr/bin/env bash
# Flashea un .bin al K4 v2 (el teclado debe estar en modo bootloader).
# Uso: scripts/flash.sh build/out/keychron_k4_rgb_v2_ansi_default.bin
set -euo pipefail
source "$(dirname "$0")/common.sh"
need_flasher

file="${1:-}"
[ -n "$file" ] && [ -f "$file" ] || die "Uso: scripts/flash.sh <archivo.bin>"

echo "Archivo: $file  (checksum $(image_checksum "$file"))"
read -r -p "¿Flashear este archivo en el teclado? Escribe 'si' para continuar: " ok
[ "$ok" = "si" ] || die "Cancelado."

wait_for_bootloader
"$SONIXFLASHER" -v "$BOOTLOADER_VIDPID" -f "$file"
echo
echo "Hecho. Desconecta y vuelve a conectar el teclado (sin puentear BOOT)."
