#!/usr/bin/env bash
# Respaldo del firmware ORIGINAL antes de instalar QMK.
#
# El bootloader del SN32F248B no permite leer la flash por USB, así que el
# respaldo se hace así:
#   1. Se lee del teclado (modo bootloader) la suma de verificación de su
#      firmware actual, su nivel de protección y la info del chip.
#   2. Se descargan los firmwares originales del K4 v2 (ANSI v1.07 / ISO v1.01)
#      y se verifica su SHA-256.
#   3. Se compara la suma del teclado con la de esos archivos: si coincide,
#      el archivo descargado es BIT A BIT tu firmware original.
#
# Requisitos: teclado conectado con los pines BOOT puenteados (ver guía).
# No escribe nada en el teclado.
set -euo pipefail
source "$(dirname "$0")/common.sh"
need_flasher

mkdir -p "$BACKUP"
stamp="$(date +%Y%m%d-%H%M%S)"
info="$BACKUP/device-info-$stamp.txt"

wait_for_bootloader
echo "==> Leyendo información del chip (solo lectura)"
"$SONIXFLASHER" -v "$BOOTLOADER_VIDPID" -i 2>&1 | tee "$info"
dev_sum="$(grep -i "Flash checksum" "$info" | grep -oiE "0x[0-9a-f]{4}" | head -1 | tr 'A-F' 'a-f' || true)"

echo
echo "==> Descargando firmwares originales del K4 v2"
fetch() {
    local file="$1" sha="$2"
    if [ ! -f "$BACKUP/$file" ]; then
        curl -fsSL -o "$BACKUP/$file" "$STOCK_BASE_URL/$file"
    fi
    echo "$sha  $BACKUP/$file" | shasum -a 256 -c - >/dev/null || die "SHA-256 incorrecto en $file"
    echo "   OK  $file  (checksum $(image_checksum "$BACKUP/$file"))"
}
fetch "$STOCK_ANSI_FILE" "$STOCK_ANSI_SHA256"
fetch "$STOCK_ISO_FILE" "$STOCK_ISO_SHA256"

echo
if [ -z "$dev_sum" ]; then
    echo "AVISO: el bootloader no devolvió checksum; no se pudo comparar."
elif [ "$dev_sum" = "$STOCK_ANSI_CHECKSUM" ]; then
    cp "$BACKUP/$STOCK_ANSI_FILE" "$BACKUP/MI_FIRMWARE_ORIGINAL.bin"
    echo "COINCIDE: tu teclado tiene exactamente $STOCK_ANSI_FILE"
elif [ "$dev_sum" = "$STOCK_ISO_CHECKSUM" ]; then
    cp "$BACKUP/$STOCK_ISO_FILE" "$BACKUP/MI_FIRMWARE_ORIGINAL.bin"
    echo "COINCIDE: tu teclado tiene exactamente $STOCK_ISO_FILE"
else
    # The stock firmware keeps the user's lighting settings in two flash
    # pages (0x7000 / 0x8000) inside the image, so the same firmware with
    # different saved settings differs by a few whole high bytes.
    for pair in "ANSI:$STOCK_ANSI_CHECKSUM" "ISO:$STOCK_ISO_CHECKSUM"; do
        name="${pair%%:*}"; ref="${pair#*:}"
        diff=$(( (ref - dev_sum) & 0xFFFF ))
        if [ $(( diff & 0xFF )) -eq 0 ] && { [ "$diff" -le $((0x2000)) ] || [ "$diff" -ge $((0xE000)) ]; }; then
            printf 'CASI COINCIDE con %s (diferencia 0x%04x): es muy probable que sea el mismo\n' "$name" "$diff"
            echo "firmware con otros ajustes de luces guardados. Para confirmarlo: sal del modo boot,"
            echo "restablece de fábrica (Fn + J + Z durante 3-5 s, en modo Cable), vuelve a entrar al"
            echo "modo boot y ejecuta este script otra vez."
        fi
    done
    echo "NO COINCIDE exactamente: tu teclado reporta checksum $dev_sum."
    echo "Tu versión de firmware no está en la base de datos (p. ej. otra versión o idioma)."
    echo "Opciones antes de flashear QMK:"
    echo "  - Descargar el actualizador oficial de Keychron para tu K4 v2 (Windows) y"
    echo "    guardarlo: sirve para restaurar el original más adelante."
    echo "  - Volcar la flash con un ST-Link (https://github.com/SonixQMK/sonix_dumper)."
fi
echo
echo "Respaldo guardado en: $BACKUP"
echo "Info del chip:        $info"
