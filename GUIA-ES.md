# QMK con Bluetooth en el Keychron K4 V2 (macOS) — guía en español

Esta guía instala [SonixQMK](https://github.com/SonixQMK/qmk_firmware) en un
**Keychron K4 V2 RGB**, guarda primero el firmware original y añade soporte
**Bluetooth** (probado en un K4 V2 ANSI, ver [Estado del Bluetooth](#estado-del-bluetooth)).

> Flashear firmware tiene un riesgo pequeño. El chip del K4 V2 (SN32F248B)
> tiene el bootloader grabado en ROM, así que puentear los pines BOOT siempre
> permite volver a flashear, incluso el firmware original. Aun así: usa un
> buen cable, no desconectes durante el flasheo y sigue los pasos en orden.

## 0. Qué necesitas saber de tu teclado

| Dato | Cómo saberlo |
|---|---|
| **ANSI o ISO** | ANSI: Enter horizontal de una fila. ISO: Enter alto en forma de "L" invertida y tecla extra junto al Shift izquierdo (p. ej. teclado en español). |
| **Versión RGB** | Retroiluminación de colores. Esta guía no cubre la versión de luz blanca. |
| **PCB** | Quita la barra espaciadora: debe decir `FK52332UB-G Ver10` (la versión confirmada) y verse los pines BOOT. |

## 1. Preparar el entorno (una sola vez)

Desde la carpeta del repo:

```bash
scripts/setup-macos.sh
```

Descarga todo dentro de `build/` (ignorado por git): SonixQMK en el commit
probado, el toolchain ARM oficial de QMK, la CLI de QMK en un entorno Python y
compila `sonixflasher`, el flasheador oficial de SonixQMK para la terminal.
Solo instala `libusb` y `pkgconf` con Homebrew si te faltan.

## 2. Entrar en modo bootloader

1. Pon el interruptor lateral en **Cable**.
2. Desconecta el cable USB.
3. Quita la barra espaciadora y puentea los dos pines **BOOT** (pinzas o un clip).
4. Conecta el cable sin dejar de puentear y suelta después de 1–2 segundos.

Comprueba que se ve el bootloader (`SONIX SN32F24XB  0c45  7040`):

```bash
build/tools/SonixFlasherC/sonixflasher -c
```

> Con QMK ya instalado no hace falta abrir nada: **Fn + Esc** entra en el bootloader.

## 3. Respaldo del firmware original (antes de instalar nada)

```bash
scripts/backup-stock.sh
```

Qué hace (no escribe nada en el teclado):

1. Lee del chip su **checksum de flash**, el nivel de protección y la info del chip
   y lo guarda en `backup/device-info-<fecha>.txt`.
2. Descarga los firmwares originales del K4 V2 desde la base de datos de SonixQMK
   (ANSI v1.07 e ISO v1.01) y verifica su SHA-256.
3. Compara: si el checksum de tu teclado coincide con uno de ellos, ese archivo
   es **idéntico** a tu firmware, y se copia como `backup/MI_FIRMWARE_ORIGINAL.bin`.

**¿Por qué no se "copia" directamente del teclado?** El bootloader del
SN32F248B no permite leer la flash por USB. La única copia física es con un
ST-Link soldado al chip ([sonix_dumper](https://github.com/SonixQMK/sonix_dumper)).
La comparación por checksum demuestra que el archivo descargado es tu firmware.

**Si dice "NO COINCIDE"** (tienes otra versión de firmware), no sigas hasta tener
una forma de volver atrás: descarga y guarda el actualizador oficial de Keychron
para el K4 V2, o haz el volcado con ST-Link. Guarda la carpeta `backup/` en un
lugar seguro (fuera del repo también).

## 4. Compilar el firmware

```bash
scripts/build.sh
```

Genera `build/out/keychron_k4_rgb_v2_ansi_default.bin` y
`build/out/keychron_k4_rgb_v2_iso_default.bin`.

## 5. Flashear

Con el teclado en bootloader (paso 2), elige el archivo de **tu** distribución:

```bash
scripts/flash.sh build/out/keychron_k4_rgb_v2_ansi_default.bin
```

o, si tu teclado es ISO:

```bash
scripts/flash.sh build/out/keychron_k4_rgb_v2_iso_default.bin
```

El script pide escribir `si`, flashea y verifica el checksum. Al terminar,
desconecta y vuelve a conectar el teclado (sin puentear BOOT).

## 6. Volver al firmware original

En bootloader (Fn + Esc con QMK, o puenteando BOOT):

```bash
scripts/restore-stock.sh
```

Usa `backup/MI_FIRMWARE_ORIGINAL.bin`. Para forzar una versión concreta:
`scripts/restore-stock.sh ansi` o `scripts/restore-stock.sh iso`.

## 7. Cómo se usa

| Acción | Teclas |
|---|---|
| Cambiar entre Mac y Windows | Interruptor lateral **Mac / Win** (capas propias para cada sistema) |
| Teclas F1–F12 en modo Mac | **Fn** + fila de funciones (sin Fn: brillo, Mission Control, Launchpad, multimedia, como el original) |
| Multimedia en modo Windows | **Fn** + fila de funciones |
| Bootloader (para flashear) | **Fn + Esc** |
| Elegir perfil Bluetooth 1/2/3 | Interruptor en **BT**, luego **Fn + 1 / 2 / 3** |
| Emparejar un dispositivo nuevo | **Mantén Fn + 1 / 2 / 3 durante 3 s**: la tecla parpadea en azul; al conectar queda fija |
| Ver la batería | **Fn + B**: la fila de números se llena (verde > 70 %, naranja 30–70 %, rojo < 30 %) |
| Batería baja | La tecla del perfil parpadea en rojo cada 4 s |
| Ahorro de energía (BT) | Luces apagadas tras 10 min sin usar (40 s si no hay conexión); cualquier tecla las enciende |

### Luces (colores y efectos)

| Qué cambia | Teclas |
|---|---|
| Siguiente efecto (Shift = anterior) | **Tecla de luz** (arriba a la derecha) |
| Encender / apagar | **Fn + tecla de luz** |
| Color | **Fn + `-`** del teclado numérico |
| Saturación | **Fn + Enter** del teclado numérico |
| Velocidad | **Fn + `+`** del teclado numérico |
| Brillo | Mac: **F5 / F6** · Windows: **Fn + F5 / F6** |

Mantén **Shift** para ir en sentido contrario. Los ajustes se guardan solos.

#### Efecto "Zonas" (colores por grupo de teclas)

Es el efecto con el que arranca el firmware: cada grupo de teclas tiene su
propio color fijo, para distinguirlos de un vistazo.

| Grupo | Color |
|---|---|
| Esc | Rojo |
| F1–F12 | Morado |
| Fila de números (`` ` `` 1–0 `-` `=`) | Naranja |
| Letras A–Z | Cian |
| Símbolos (`[ ] \ ; ' , . /`, y `#` / `\` extra en ISO) | Amarillo |
| Modificadores (Tab, Bloq Mayús, Shift, Ctrl, Win/Cmd, Alt/Opt, Fn, Enter, Borrar, Espacio) | Blanco |
| Navegación (Supr, Inicio, Fin, RePág, AvPág, flechas) | Verde |
| Teclado numérico | Azul |
| Tecla de luz | Magenta |

- El **brillo** funciona igual que siempre (F5/F6, o en VIA).
- El **color** (Fn + `-` numérico, o el selector de VIA) gira toda la paleta a
  la vez: los grupos siguen siendo distintos entre sí. Para volver a los colores
  de la tabla, deja el tono en 0 (rojo puro en VIA).
- La **saturación** baja todos los grupos hacia el blanco.
- Si cambias a otro efecto con la tecla de luz, sigue pulsándola (o Shift + tecla
  de luz) hasta volver a "Zonas". La lista de efectos de VIA no conoce este
  efecto por su nombre, así que para volver a él usa la tecla de luz.
- Para cambiar grupos o colores hay que editar
  `firmware/keyboards/keychron/k4/rgb/v2/rgb_matrix_kb.inc`, recompilar y flashear.

Para emparejar con el Mac: mantén Fn + 1 tres segundos y elige el teclado
(aparece con el nombre que le da el módulo, normalmente "Keychron K4") en
Ajustes del Sistema → Bluetooth.

## 8. Configurar con VIA (colores y teclas con el ratón)

[VIA](https://usevia.app) es un programa gratuito que permite cambiar colores,
efectos y lo que hace cada tecla sin recompilar. Necesita la versión `via` del
firmware (incluye el mismo Bluetooth):

```bash
scripts/build.sh via
```

Entra al bootloader (**Fn + Esc**) y flashea tu distribución:

```bash
scripts/flash.sh build/out/keychron_k4_rgb_v2_ansi_via.bin
```

(o `..._iso_via.bin` si tu teclado es ISO). Al arrancar por primera vez con VIA
se reinician los ajustes guardados (luces) a los de fábrica; es normal.

Luego:

1. Abre **https://usevia.app** en **Google Chrome** o Edge (Safari no sirve:
   no permite que las webs hablen con teclados USB). Usa el modo **Cable**.
2. En **Settings** activa **Show Design tab**.
3. En la pestaña **Design**, arrastra el archivo `via/k4v2_ansi_via.json`
   (o `via/k4v2_iso_via.json`) de este repo. Como el K4 V2 no está en la lista
   oficial de VIA, este paso hay que repetirlo cada vez que abras VIA.
4. Pulsa **Authorize device**, elige el teclado y ya puedes usar:
   - **Lighting**: efecto, brillo, velocidad y un selector de color.
   - **Configure**: cambiar teclas. Capas: **0 = Windows**, **1 = Mac**,
     **2 = Fn** (compartida). En modo Mac, Fn + F1…F12 envía lo que tenga la
     fila de F de la capa 0. La tecla de batería aparece en *Custom* como **Batt**.

Por memoria (el chip tiene 8 KB de RAM) la versión VIA usa 3 capas en lugar de 4.

## Estado del Bluetooth

El Bluetooth del K4 V2 lo gestiona un módulo ITON separado que habla con el
microcontrolador por SPI. Lo que hay en este repo:

- **Transporte** (`firmware/keyboards/keychron/k4/rgb/v2/k4v2_bt.c`): portado del
  Keychron **K3 V2** ([SonixQMK PR #501](https://github.com/SonixQMK/qmk_firmware/pull/501)),
  donde se escribió imitando el firmware original y está **probado en hardware**
  (emparejar y escribir por Bluetooth). El K3 V2 y el K4 V2 son la misma
  plataforma: mismo chip, mismos pines de la matriz RGB, y el firmware original
  del K4 V2 usa el mismo periférico SPI0.
- **Lógica de teclado** (`v2.c`): perfiles, emparejamiento, batería y suspensión,
  basada en el K3 V2 y en la integración para K4 V2 de
  [SonixQMK PR #486](https://github.com/SonixQMK/qmk_firmware/pull/486).
- **Compila** para ANSI e ISO, y se verificó que el vector de interrupción SPI0
  apunta al nuevo driver y que cabe en los 8 KB de RAM.
- **Probado en un K4 V2 ANSI físico** (firmware VIA, con un Mac): el perfil 1
  empareja y se escribe por Bluetooth sin cable. Falta confirmar en hardware los
  perfiles 2 y 3, el indicador de batería, la suspensión y la versión ISO. Si
  algo no responde, nada se rompe: el modo cable sigue funcionando y puedes
  volver al original.

### Si algo falla

- **El Bluetooth no empareja / no escribe:** anota qué hace la tecla del perfil
  (¿parpadea azul?, ¿nada?) y abre un issue con esos datos.
- **Teclas "fantasma" o repetidas solo en modo BT:** añade
  `#define K4V2_MATRIX_SELECT_DELAY_US 20` en
  `firmware/keyboards/keychron/k4/rgb/v2/config.h`, recompila y flashea.
- **Mac y Windows invertidos:** añade `#define K4V2_OS_SWITCH_DISABLE` en ese
  mismo `config.h` y avisa en un issue.
- **Cualquier problema grave:** `scripts/restore-stock.sh`.

## Créditos

dexter93 y el equipo de SonixQMK, mgrenonville (transporte Bluetooth del K3 V2),
reubenjs (integración Bluetooth del K4 V2), 1Conan (driver ITON original),
PythonDeployer y ephb (port del K4 V2).
