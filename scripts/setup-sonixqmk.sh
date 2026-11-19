#!/usr/bin/env bash
# Para Windows (consola QMK MSYS) o Linux con QMK ya instalado: descarga
# SonixQMK en ./build/sonixqmk (commit fijado) con sus submódulos.
# El compilador ARM y make los pone QMK MSYS (o `qmk setup` en Linux).
# Después: scripts/build.sh via
set -euo pipefail
source "$(dirname "$0")/common.sh"

command -v git >/dev/null || die "Falta git."
command -v arm-none-eabi-gcc >/dev/null || die "Falta arm-none-eabi-gcc. En Windows abre la consola QMK MSYS (https://msys.qmk.fm); en Linux instala QMK (https://docs.qmk.fm/newbs_getting_started)."

mkdir -p "$BUILD"
setup_sonixqmk

echo
echo "Listo. Siguiente paso: scripts/build.sh via"
echo "Para flashear en Windows usa scripts\\windows\\flash.cmd (o el kit de Windows del Release)."
