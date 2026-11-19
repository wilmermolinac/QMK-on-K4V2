Keychron K4 V2 - QMK with Bluetooth - Windows kit
=================================================

Full guide (English): https://github.com/wilmermolinac/QMK-on-K4V2#readme
Guia completa (espanol): https://github.com/wilmermolinac/QMK-on-K4V2/blob/main/GUIA-ES.md

1. Enter bootloader mode: set the side switch to Cable, unplug the keyboard,
   remove the space bar, short the two BOOT pads (tweezers or a paper clip)
   and plug the cable in while shorting them. The lights stay off.
   (With this firmware already installed: Fn + Esc.)

2. BACK UP FIRST: double-click backup-stock.cmd.
   It never writes to the keyboard. "MATCH" means backup\MI_FIRMWARE_ORIGINAL.bin
   is your stock firmware. If it says NO MATCH / ALMOST A MATCH, read the
   message (factory reset with Fn + J + Z, then try again).

3. Flash: enter bootloader mode again and drag the .bin for YOUR layout from
   the firmware folder onto flash.cmd (or double-click flash.cmd and pick it):
     ANSI (wide one-row Enter) -> keychron_k4_rgb_v2_ansi_via_*.bin
     ISO  (tall "L" Enter)     -> keychron_k4_rgb_v2_iso_via_*.bin
   Type yes. When it's done, unplug and plug the keyboard back in.

4. VIA: open https://usevia.app in Chrome or Edge, Settings -> Show Design tab,
   load via\k4v2_ansi_via.json (or the ISO one) in the Design tab, then
   Authorize device.

Go back to the stock firmware: bootloader mode, then double-click
restore-stock.cmd.

The first run downloads the official SonixFlasherC for Windows
(github.com/SonixQMK/SonixFlasherC) and checks its SHA-256.

If the bootloader is never found, see "Windows driver" in the guide.
