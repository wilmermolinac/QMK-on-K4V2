# QMK with working Bluetooth for the Keychron K4 V2 (RGB)

![Keychron K4 V2 running this firmware with the Zones lighting effect](docs/images/k4v2-zones.jpg)

Custom [SonixQMK](https://github.com/SonixQMK/qmk_firmware) firmware for the
**Keychron K4 V2 RGB** that adds what upstream QMK was missing on this board:
**Bluetooth**. It also comes with VIA support, a lighting effect that colors
each key group differently (the photo above), and macOS scripts to back up,
flash and restore the stock firmware.

**Leer en español:** [GUIA-ES.md](GUIA-ES.md)

> Fork of [ephb/QMK-on-K4V2](https://github.com/ephb/QMK-on-K4V2). The original
> Windows guide is kept in [docs/ORIGINAL-GUIDE.md](docs/ORIGINAL-GUIDE.md).

## What you get

| Feature | Details |
|---|---|
| **Bluetooth** | 3 profiles, pairing, battery level on the number row, low-battery warning, auto sleep |
| **Mac / Windows switch** | The side switch selects a Mac layer or a Windows layer (Option/Command, macOS F-row functions) |
| **VIA** | Remap keys and change lighting from [usevia.app](https://usevia.app), no recompiling |
| **Zones lighting** | Default effect: letters, numbers, F-row, symbols, modifiers, navigation and numpad each get their own color |
| **Safe install** | Scripts verify your stock firmware before flashing and can put it back at any time |

## Status

| | |
|---|---|
| ✅ Tested on real hardware | K4 V2 **ANSI** RGB, VIA build, macOS: wired typing, VIA, Zones lighting, Bluetooth pairing (profile 1) and typing over Bluetooth |
| ⚠️ Builds, not yet confirmed on hardware | Bluetooth profiles 2 and 3, battery gauge (Fn + B), sleep/wake, the **ISO** layout, Windows as the host |

If you try any of the untested parts, please open an issue and say how it went.

## Before you start

- **Keyboard:** K4 **V2 RGB**. The white-backlight version is not covered.
  Remove the space bar: the PCB should read `FK52332UB-G Ver10` and show two
  **BOOT** pads.
- **ANSI or ISO:** ANSI has a wide one-row Enter. ISO has a tall, upside-down "L"
  Enter and an extra key next to the left Shift (for example Spanish or UK layouts).
- **Computer:** a Mac (Apple Silicon or Intel) with [Homebrew](https://brew.sh).
  For Windows or Linux, see [Other operating systems](#other-operating-systems).

> **Risk:** flashing always carries a small risk. The SN32F248B chip has its
> bootloader in ROM, so shorting the BOOT pads always lets you flash again,
> including the stock firmware. Use a good cable, don't unplug while flashing,
> and follow the steps in order.

## 1. Set up the tools (once)

```bash
git clone https://github.com/wilmermolinac/QMK-on-K4V2.git
cd QMK-on-K4V2
scripts/setup-macos.sh
```

This downloads everything into `build/` (ignored by git): SonixQMK at the
tested commit, the official QMK ARM toolchain, the QMK CLI in a Python
virtualenv, and [SonixFlasherC](https://github.com/SonixQMK/SonixFlasherC),
the command-line flasher. It only installs `libusb` and `pkgconf` through
Homebrew if they're missing.

## 2. Enter bootloader mode

1. Set the side switch to **Cable**.
2. Unplug the USB cable.
3. Remove the space bar and short the two **BOOT** pads (tweezers or a paper clip;
   touching both pads is enough).
4. Plug the cable in while still shorting the pads, then let go after 1–2 seconds.
   The keyboard lights stay off.

**How to tell it worked:** the keyboard lights stay off and it doesn't type.
To be sure, list the connected devices:

```bash
build/tools/SonixFlasherC/sonixflasher -c
```

You should see a line with `SONIX SN32F24XB` and `0c45  7040`. If that line isn't
there, unplug and try again: the pads have to be shorted at the moment you plug in.

> Once this firmware is installed you don't need to open the keyboard again:
> **Fn + Esc** enters the bootloader.

## 3. Back up the stock firmware

Do this **before** flashing anything:

```bash
scripts/backup-stock.sh
```

It never writes to the keyboard. The bootloader can't read the flash back over
USB, so the script does this instead:

1. Reads the chip's **flash checksum** and saves it to `backup/device-info-<date>.txt`.
2. Downloads the stock K4 V2 firmware (ANSI v1.07, ISO v1.01) from the SonixQMK
   database and checks its SHA-256.
3. If the checksums match, that file is **identical** to your firmware, and it's
   saved as `backup/MI_FIRMWARE_ORIGINAL.bin`.

**If it says "NO COINCIDE" (no match):** the stock firmware stores your lighting
settings in flash, which changes the checksum. Do a factory reset with
**Fn + J + Z** (hold about 3 seconds), go back to step 2 and run the backup again.
If it still doesn't match, you have a different firmware version: stop here and
keep Keychron's official updater for the K4 V2 as your way back.

Copy the `backup/` folder somewhere safe outside the repo too.

## 4. Build the firmware

> **Don't want to build?** Download the ready-made `.bin` for your layout from
> [Releases](https://github.com/wilmermolinac/QMK-on-K4V2/releases/latest) and
> skip to step 5. You still need step 1 (or another copy of SonixFlasherC) to flash it.

Pick one:

```bash
scripts/build.sh via       # recommended: works with VIA
scripts/build.sh           # plain QMK keymap (smaller, no VIA)
```

The files end up in `build/out/`, for example
`keychron_k4_rgb_v2_ansi_via.bin` and `keychron_k4_rgb_v2_iso_via.bin`.

## 5. Flash

With the keyboard in bootloader mode (step 2, or **Fn + Esc** once this firmware
is installed), flash the file that matches **your** layout:

```bash
scripts/flash.sh build/out/keychron_k4_rgb_v2_ansi_via.bin
```

The script asks you to type `si` ("yes"), flashes and checks the checksum. When
it finishes, unplug and plug the keyboard back in. On a Mac, if the Keyboard
Setup Assistant opens, choose your layout (ANSI or ISO).

The first boot after a new build resets the saved settings (lighting, VIA
keymap) to the defaults. That's expected.

## 6. Use VIA

1. Open **[usevia.app](https://usevia.app)** in **Chrome** or **Edge** (Safari can't
   talk to USB keyboards). Use the keyboard in **Cable** mode.
2. Go to **Settings** and turn on **Show Design tab**.
3. In the **Design** tab, load [`via/k4v2_ansi_via.json`](via/k4v2_ansi_via.json)
   (or [`via/k4v2_iso_via.json`](via/k4v2_iso_via.json)). The K4 V2 isn't in
   VIA's official list, so you have to load it every time you open VIA.
4. Click **Authorize device** and pick the keyboard.

You can now use:

- **Configure:** remap keys. Layers are **0 = Windows**, **1 = Mac**, **2 = Fn**
  (shared by both). In Mac mode, Fn + F1…F12 sends whatever layer 0 has on the
  F-row. The battery key is under *Custom* as **Batt**.
- **Lighting:** effect, brightness, speed and color.

The VIA build has 3 layers instead of 4 because the chip only has 8 KB of RAM.

## 7. Keys and lighting

### Everyday keys

| Action | Keys |
|---|---|
| Switch Mac / Windows | Side switch **Mac / Win** |
| F1–F12 in Mac mode | **Fn** + F-row (without Fn: brightness, Mission Control, Launchpad, media, like stock) |
| Media in Windows mode | **Fn** + F-row |
| Bootloader (to flash) | **Fn + Esc** |
| Insert / Print Screen / Sleep | **Fn + Del / Home / PgDn** |

### Bluetooth

| Action | Keys |
|---|---|
| Use Bluetooth | Side switch on **BT** |
| Choose profile 1 / 2 / 3 | **Fn + 1 / 2 / 3** |
| Pair a new device | **Hold Fn + 1 / 2 / 3 for 3 s**: the key blinks blue. Pick the keyboard in your device's Bluetooth settings; the key turns solid blue when connected |
| Battery level | **Fn + B**: the number row fills up (green > 70 %, orange 30–70 %, red < 30 %) |
| Low battery | The profile key blinks red every 4 s |
| Power saving | Lights off after 10 min idle (40 s if not connected); any key wakes it |

### Lighting

| Change | Keys |
|---|---|
| Next effect (Shift = previous) | **Light key** (top right) |
| On / off | **Fn + light key** |
| Color | **Fn + numpad `-`** |
| Saturation | **Fn + numpad Enter** |
| Speed | **Fn + numpad `+`** |
| Brightness | Mac: **F5 / F6** · Windows: **Fn + F5 / F6** |

Hold **Shift** to go the other way. Settings are saved automatically.

### Zones effect

The default effect gives each key group a fixed color:

| Group | Color |
|---|---|
| Esc | Red |
| F1–F12 | Purple |
| Number row (`` ` `` 1–0 `-` `=`) | Orange |
| Letters A–Z | Cyan |
| Symbols (`[ ] \ ; ' , . /`, plus the extra ISO keys) | Yellow |
| Modifiers (Tab, Caps Lock, Shift, Ctrl, Win/Cmd, Alt/Opt, Fn, Enter, Backspace, Space) | White |
| Navigation (Del, Home, End, PgUp, PgDn, arrows) | Green |
| Numpad | Blue |
| Light key | Magenta |

- **Brightness** works as usual.
- **Color** (Fn + numpad `-`, or VIA's color picker) rotates the whole palette, so
  the groups stay distinct. Set the hue back to 0 (pure red in VIA) for the
  colors above.
- **Saturation** fades every group toward white.
- If you switch to another effect, keep pressing the light key to get back to
  Zones. VIA's effect list doesn't show it by name.

#### Change the groups or colors

Edit [`rgb_matrix_kb.inc`](firmware/keyboards/keychron/k4/rgb/v2/rgb_matrix_kb.inc):

- `k4v2_zone_map` is the keyboard matrix (6 rows × 19 columns). Each entry
  says which group a key belongs to, for example `_L` for letters. To make
  W, A, S, D stand out, add a new group and use it at those positions.
- `k4v2_zone_hs` holds the color of each group as `{hue, saturation}` on QMK's
  0–255 scale: 0 red, 21 orange, 43 yellow, 85 green, 128 cyan, 170 blue,
  191 purple, 213 magenta. Saturation 0 is white.

Then run `scripts/build.sh via` and flash again (step 5).

## 8. Go back to the stock firmware

Enter the bootloader (**Fn + Esc**, or short the BOOT pads) and run:

```bash
scripts/restore-stock.sh
```

It flashes `backup/MI_FIRMWARE_ORIGINAL.bin`. To force a specific version:
`scripts/restore-stock.sh ansi` or `scripts/restore-stock.sh iso`.

## Troubleshooting

- **Bluetooth doesn't pair or doesn't type:** write down what the profile key
  does (blinks blue? nothing?) and open an issue. Wired mode keeps working either way.
- **Ghost or repeated keys only over Bluetooth:** add
  `#define K4V2_MATRIX_SELECT_DELAY_US 20` to
  [`config.h`](firmware/keyboards/keychron/k4/rgb/v2/config.h), rebuild and flash.
- **Mac and Windows layers swapped:** add `#define K4V2_OS_SWITCH_DISABLE` to the
  same `config.h` and open an issue.
- **VIA says it can't find a V3 definition:** load the JSON from the `via/`
  folder in the Design tab (step 6.3).
- **Anything serious:** `scripts/restore-stock.sh`.

## Other operating systems

The scripts are for macOS, but the firmware itself is standard SonixQMK. On
Windows or Linux:

1. Set up [SonixQMK](https://github.com/SonixQMK/qmk_firmware) on the
   `sn32_develop` branch at commit `c9f3803ff1cb910a713c6202cab357268cf98fa6`
   (the one in [`scripts/common.sh`](scripts/common.sh)).
2. Copy this repo's `firmware/keyboards/` folder on top of SonixQMK's `keyboards/`.
3. Build with `make keychron/k4/rgb/v2/ansi:via` (or `iso`).
4. Flash with [SonixFlasherC](https://github.com/SonixQMK/SonixFlasherC)
   (`sonixflasher -v 0c45/7040 -f <file>.bin`) or the Sonix Flasher app
   ([original guide](docs/ORIGINAL-GUIDE.md), offset `0x00`).

Back up first: without the script, compare the checksum from `sonixflasher -i`
with the stock checksums in `scripts/common.sh`.

## How the Bluetooth works

The K4 V2's Bluetooth is a separate ITON module that talks to the SN32F248B
over SPI.

- **Transport** ([`k4v2_bt.c`](firmware/keyboards/keychron/k4/rgb/v2/k4v2_bt.c)):
  ported from the Keychron **K3 V2**
  ([SonixQMK PR #501](https://github.com/SonixQMK/qmk_firmware/pull/501)), which
  copies the stock firmware's protocol. The K3 V2 and K4 V2 share the same chip
  and SPI peripheral.
- **Keyboard logic** ([`v2.c`](firmware/keyboards/keychron/k4/rgb/v2/v2.c)):
  profiles, pairing, battery and sleep, based on the K3 V2 and on the K4 V2 work in
  [SonixQMK PR #486](https://github.com/SonixQMK/qmk_firmware/pull/486).
- It plugs into QMK's `custom` Bluetooth driver, so wired and Bluetooth follow
  the side switch.

## Repository layout

| Path | Contents |
|---|---|
| `firmware/keyboards/keychron/k4/rgb/v2/` | Files copied on top of SonixQMK: Bluetooth driver, board logic, keymaps, Zones effect |
| `via/` | VIA definitions for ANSI and ISO |
| `scripts/` | macOS setup, build, backup, flash and restore |
| `GUIA-ES.md` | Full guide in Spanish |
| `docs/ORIGINAL-GUIDE.md` | ephb's original Windows guide |

## Credits

- [ephb](https://github.com/ephb/QMK-on-K4V2): the original K4 V2 guide and port.
- [dexter93](https://github.com/dexter93) and the [SonixQMK](https://github.com/SonixQMK) team.
- mgrenonville: K3 V2 Bluetooth transport (SonixQMK PR #501).
- reubenjs: K4 V2 Bluetooth integration (SonixQMK PR #486).
- 1Conan: the original ITON driver.
- PythonDeployer: K4 V2 ANSI port.

The firmware code is GPL-2.0-or-later, like QMK.
