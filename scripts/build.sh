#!/usr/bin/env bash
# Copia firmware/keyboards sobre SonixQMK y compila el K4 v2 (ANSI e ISO).
# Uso: scripts/build.sh [keymap]        (por defecto: default)
# Resultado: build/out/keychron_k4_rgb_v2_{ansi,iso}_<keymap>.bin
set -euo pipefail
source "$(dirname "$0")/common.sh"

KEYMAP="${1:-default}"
QMK_HOME="$BUILD/sonixqmk"
[ -d "$QMK_HOME/lib/chibios/os" ] || die "Falta SonixQMK. Ejecuta primero: scripts/setup-macos.sh (macOS) o scripts/setup-sonixqmk.sh (QMK MSYS / Linux)"

# macOS: toolchain and QMK CLI downloaded by setup-macos.sh. QMK MSYS / Linux:
# use the ones already on PATH.
TC_BIN="$(ls -d "$BUILD"/toolchain/*/bin 2>/dev/null | head -1 || true)"
[ -n "$TC_BIN" ] && export PATH="$TC_BIN:$PATH"
[ -d "$BUILD/venv/bin" ] && export PATH="$BUILD/venv/bin:$PATH"
command -v arm-none-eabi-gcc >/dev/null || die "No se encuentra arm-none-eabi-gcc. En Windows usa la consola QMK MSYS."
export QMK_HOME

echo "==> Aplicando firmware/keyboards sobre SonixQMK"
cp -R "$ROOT/firmware/keyboards/." "$QMK_HOME/keyboards/"

mkdir -p "$BUILD/out"
for layout in ansi iso; do
    kb="keychron/k4/rgb/v2/$layout"
    echo "==> Compilando $kb:$KEYMAP"
    make -C "$QMK_HOME" -j"$(cpu_count)" "$kb:$KEYMAP" SKIP_GIT=yes
    bin="keychron_k4_rgb_v2_${layout}_${KEYMAP}.bin"
    cp "$QMK_HOME/$bin" "$BUILD/out/$bin"
done

echo
ls -l "$BUILD/out"
