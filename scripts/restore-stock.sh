#!/usr/bin/env bash
# Vuelve al firmware ORIGINAL de Keychron.
# Uso: scripts/restore-stock.sh            (usa backup/MI_FIRMWARE_ORIGINAL.bin)
#      scripts/restore-stock.sh ansi|iso   (fuerza la versión ANSI o ISO)
# El teclado debe estar en modo bootloader (pines BOOT puenteados, o Fn+Esc
# si tienes QMK instalado).
set -euo pipefail
source "$(dirname "$0")/common.sh"

case "${1:-}" in
    ansi) file="$BACKUP/$STOCK_ANSI_FILE"; sha="$STOCK_ANSI_SHA256" ;;
    iso)  file="$BACKUP/$STOCK_ISO_FILE";  sha="$STOCK_ISO_SHA256" ;;
    "")   file="$BACKUP/MI_FIRMWARE_ORIGINAL.bin"; sha="" ;;
    *)    die "Uso: scripts/restore-stock.sh [ansi|iso]" ;;
esac

[ -f "$file" ] || die "No existe $file. Ejecuta primero scripts/backup-stock.sh (o indica ansi|iso)."
if [ -n "$sha" ]; then
    echo "$sha  $file" | shasum -a 256 -c - >/dev/null || die "SHA-256 incorrecto en $file"
fi

exec "$(dirname "$0")/flash.sh" "$file"
