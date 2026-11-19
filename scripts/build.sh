#!/usr/bin/env bash
# Copia firmware/keyboards sobre SonixQMK y compila el K4 v2 (ANSI e ISO).
# Uso: scripts/build.sh [keymap]        (por defecto: default)
# Resultado: build/out/keychron_k4_rgb_v2_{ansi,iso}_<keymap>.bin
set -euo pipefail
source "$(dirname "$0")/common.sh"

KEYMAP="${1:-default}"
QMK_HOME="$BUILD/sonixqmk"
[ -d "$QMK_HOME/lib/chibios/os" ] || die "Falta SonixQMK. Ejecuta primero: scripts/setup-macos.sh"

TC_BIN="$(ls -d "$BUILD"/toolchain/*/bin | head -1)"
export PATH="$TC_BIN:$BUILD/venv/bin:$PATH"
export QMK_HOME

echo "==> Aplicando firmware/keyboards sobre SonixQMK"
rsync -a "$ROOT/firmware/keyboards/" "$QMK_HOME/keyboards/"

mkdir -p "$BUILD/out"
for layout in ansi iso; do
    kb="keychron/k4/rgb/v2/$layout"
    echo "==> Compilando $kb:$KEYMAP"
    make -C "$QMK_HOME" -j"$(sysctl -n hw.ncpu)" "$kb:$KEYMAP" SKIP_GIT=yes
    bin="keychron_k4_rgb_v2_${layout}_${KEYMAP}.bin"
    cp "$QMK_HOME/$bin" "$BUILD/out/$bin"
done

echo
ls -l "$BUILD/out"
