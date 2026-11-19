#!/usr/bin/env bash
# Prepara todo lo necesario en ./build (no instala nada fuera del repo, salvo
# libusb/pkgconf con Homebrew si faltan):
#   - SonixQMK (sn32_develop, commit fijado) con sus submódulos
#   - toolchain ARM oficial de QMK (GCC 15.2)
#   - entorno Python con la CLI de QMK
#   - sonixflasher (CLI oficial de SonixQMK) compilado para macOS
set -euo pipefail
source "$(dirname "$0")/common.sh"

mkdir -p "$BUILD/tools" "$BUILD/toolchain"

command -v brew >/dev/null || die "Necesitas Homebrew: https://brew.sh"
for pkg in libusb pkgconf; do
    brew list --formula "$pkg" >/dev/null 2>&1 || brew install "$pkg"
done

# 1. SonixQMK
if [ ! -d "$BUILD/sonixqmk/.git" ]; then
    echo "==> Clonando SonixQMK ($SONIXQMK_BRANCH)"
    git clone --filter=blob:none -b "$SONIXQMK_BRANCH" "$SONIXQMK_REPO" "$BUILD/sonixqmk"
fi
echo "==> Fijando SonixQMK en $SONIXQMK_COMMIT"
git -C "$BUILD/sonixqmk" fetch --quiet origin "$SONIXQMK_COMMIT" 2>/dev/null || true
git -C "$BUILD/sonixqmk" checkout --quiet "$SONIXQMK_COMMIT"
for sm in lib/chibios lib/chibios-contrib lib/printf lib/lufa; do
    git -C "$BUILD/sonixqmk" submodule update --init --depth 1 "$sm"
done

# 2. Toolchain
if [ ! -x "$BUILD"/toolchain/*/bin/arm-none-eabi-gcc ] 2>/dev/null; then
    case "$(uname -m)" in
        arm64) url="$TOOLCHAIN_URL_ARM64" ;;
        *)     url="$TOOLCHAIN_URL_X64" ;;
    esac
    echo "==> Descargando toolchain: $url"
    curl -fL -o "$BUILD/toolchain/tc.tar.zst" "$url"
    tar -xf "$BUILD/toolchain/tc.tar.zst" -C "$BUILD/toolchain"
    rm -f "$BUILD/toolchain/tc.tar.zst"
fi

# 3. QMK CLI
if [ ! -x "$BUILD/venv/bin/qmk" ]; then
    echo "==> Creando entorno Python con la CLI de QMK"
    PY="$(command -v python3.12 || command -v python3.13 || command -v python3)"
    "$PY" -m venv "$BUILD/venv"
    "$BUILD/venv/bin/pip" install --quiet qmk
fi

# 4. sonixflasher
if [ ! -x "$SONIXFLASHER" ]; then
    echo "==> Compilando sonixflasher"
    [ -d "$BUILD/tools/SonixFlasherC" ] || git clone --depth 1 https://github.com/SonixQMK/SonixFlasherC.git "$BUILD/tools/SonixFlasherC"
    make -C "$BUILD/tools/SonixFlasherC"
fi

echo
echo "Listo. Siguiente paso: scripts/backup-stock.sh (con el teclado en modo bootloader)"
