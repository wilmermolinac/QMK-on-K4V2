#!/usr/bin/env python3
"""Draws SVG previews of the Zones lighting effects into docs/images/.

Reads the key maps and palettes straight from
firmware/keyboards/keychron/k4/rgb/v2/rgb_matrix_kb.inc, so the pictures
always match the firmware. Colors are approximate: LEDs under keycaps look
different from a screen.

Usage: python3 scripts/gen-zone-previews.py
"""
import colorsys
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parent.parent
INC = (ROOT / "firmware/keyboards/keychron/k4/rgb/v2/rgb_matrix_kb.inc").read_text()
OUT = ROOT / "docs/images"

# ANSI layout: per matrix row, (label, matrix column or None for a gap, width in keys).
LAYOUT = [
    [("Esc", 0, 1)] + [(f"F{i}", i, 1) for i in range(1, 13)]
    + [("Del", 13, 1), ("Home", 14, 1), ("End", 15, 1), ("PgUp", 16, 1), ("PgDn", 17, 1), ("Light", 18, 1)],
    [("`", 0, 1)] + [(str(i % 10), i, 1) for i in range(1, 11)]
    + [("-", 11, 1), ("=", 12, 1), ("Bksp", 13, 2), ("Num", 15, 1), ("/", 16, 1), ("*", 17, 1), ("-", 18, 1)],
    [("Tab", 0, 1.5)] + [(c, i, 1) for i, c in enumerate("QWERTYUIOP", start=1)]
    + [("[", 11, 1), ("]", 12, 1), ("\\", 13, 1.5), ("7", 15, 1), ("8", 16, 1), ("9", 17, 1), ("+", 18, 1)],
    [("Caps", 0, 1.75)] + [(c, i, 1) for i, c in enumerate("ASDFGHJKL", start=1)]
    + [(";", 10, 1), ("'", 11, 1), ("Enter", 13, 2.25), ("4", 15, 1), ("5", 16, 1), ("6", 17, 1), ("", None, 1)],
    [("Shift", 0, 2.25)] + [(c, i, 1) for i, c in enumerate("ZXCVBNM", start=2)]
    + [(",", 9, 1), (".", 10, 1), ("/", 11, 1), ("Shift", 13, 1.75), ("↑", 14, 1),
       ("1", 15, 1), ("2", 16, 1), ("3", 17, 1), ("Ent", 18, 1)],
    [("Ctrl", 0, 1.25), ("Win", 1, 1.25), ("Alt", 2, 1.25), ("", 6, 6.25), ("Alt", 10, 1), ("Fn", 11, 1),
     ("Ctrl", 12, 1), ("←", 13, 1), ("↓", 14, 1), ("→", 15, 1), ("0", 16, 1), (".", 17, 1), ("", None, 1)],
]

GENERAL_GROUPS = ["", "Esc", "F1–F12", "Numbers", "Letters", "Symbols", "Modifiers", "Navigation", "Numpad", "Light key"]
GAMING_GROUPS = ["", "Everything else", "W A S D", "Q E R F G", "1–5", "Space, Shift, Ctrl, Alt",
                 "Esc, Tab, T, Z–B, M, arrows", "Light key"]
CODE_GROUPS = ["", "Letters", "1–8", "Brackets [ ] 9 0", "Operators - = / * +", "Punctuation ; ' , . \\ `",
               "Editing: Bksp, Del, Enter, Tab, Space", "Navigation", "Shortcuts: Ctrl, Cmd, Alt, Shift, Fn, Caps",
               "Esc + debug: F5, F9–F12", "Other F-keys", "Numpad digits", "Light key"]


def symbols():
    """Map the #define shorthands (_L, _W, ...) of each key map to group numbers."""
    code = re.sub(r"//[^\n]*", "", INC)  # comments may contain braces
    enums = {}
    for m in re.finditer(r"enum\s*\w*\s*\{([^}]*)\}", code):
        for i, e in enumerate(re.findall(r"(\w+)\s*(?:=\s*\d+\s*)?(?:,|$)", m.group(1).strip())):
            enums[e] = i
    for i, e in enumerate(["K4V2_ZONE_NONE", "K4V2_ZONE_ESC", "K4V2_ZONE_FROW", "K4V2_ZONE_NUM", "K4V2_ZONE_LET",
                           "K4V2_ZONE_SYM", "K4V2_ZONE_MOD", "K4V2_ZONE_NAV", "K4V2_ZONE_PAD", "K4V2_ZONE_LIGHT"]):
        enums[e] = i  # enum k4v2_zone lives in k4v2_zones.h
    result = {}
    for defines, name in re.findall(r"((?:#\s*define\s+\w+\s+\w+\s*\n)+)static const uint8_t PROGMEM (k4v2_map_\w+)", code):
        result[name] = {s: enums[v] for s, v in re.findall(r"#\s*define\s+(\w+)\s+(\w+)", defines)}
    return result


def key_map(name, syms):
    body = INC[INC.index(f"{name}[6][19] = {{"):]
    body = body[body.index("{") + 1: body.index("};")]
    rows = re.findall(r"\{([^{}]*)\}", body)
    return [[syms[c.strip()] for c in r.split(",") if c.strip()] for r in rows]


def palette(name):
    body = INC[INC.index(f"{name}["):]
    body = body[body.index("= {") + 3: body.index("};")]
    return [tuple(int(x) for x in t) for t in re.findall(r"\{\s*(\d+),\s*(\d+),\s*(\d+)\s*\}", body)]


def hsv_hex(h, s, v):
    r, g, b = colorsys.hsv_to_rgb(h / 255, s / 255, v / 255)
    return "#%02x%02x%02x" % (round(r * 255), round(g * 255), round(b * 255))


def keyboard_svg(title, kmap, pal, groups):
    u, gap, pad = 40, 4, 14
    kb_w = 19 * u + pad * 2
    legend_items = [(i, groups[i]) for i in range(1, len(groups))]
    cols = 3
    legend_rows = (len(legend_items) + cols - 1) // cols
    kb_h = 6 * u + pad * 2
    h = 36 + kb_h + 16 + legend_rows * 24 + 8
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{kb_w}" height="{h}" viewBox="0 0 {kb_w} {h}" '
           f'font-family="-apple-system, Segoe UI, Helvetica, Arial, sans-serif" role="img">',
           f'<title>{title}</title>',
           f'<rect width="{kb_w}" height="{h}" rx="14" fill="#101010"/>',
           f'<text x="{pad}" y="24" font-size="16" font-weight="600" fill="#f2f2f2">{title}</text>',
           f'<g transform="translate(0,36)">']
    for r, row in enumerate(LAYOUT):
        x = pad
        for label, col, w in row:
            width = w * u
            if col is not None:
                zone = kmap[r][col]
                hh, ss, vv = pal[zone]
                fill = hsv_hex(hh, ss, vv)
                text = "#111" if vv > 150 and not (hh > 150 and hh < 200 and ss > 200) else "#eee"
                out.append(f'<rect x="{x + gap / 2}" y="{pad + r * u + gap / 2}" width="{width - gap}" height="{u - gap}" rx="5" fill="{fill}"/>')
                if label:
                    esc = label.replace("&", "&amp;").replace("<", "&lt;")
                    out.append(f'<text x="{x + width / 2}" y="{pad + r * u + u / 2 + 4}" font-size="11" '
                               f'text-anchor="middle" fill="{text}">{esc}</text>')
            x += width
    out.append("</g>")
    y0 = 36 + kb_h + 16
    col_w = (kb_w - 2 * pad) / cols
    for n, (zone, name) in enumerate(legend_items):
        cx = pad + (n % cols) * col_w
        cy = y0 + (n // cols) * 24
        out.append(f'<rect x="{cx}" y="{cy}" width="14" height="14" rx="3" fill="{hsv_hex(*pal[zone])}"/>')
        out.append(f'<text x="{cx + 20}" y="{cy + 12}" font-size="12" fill="#d8d8d8">{name.replace("&", "&amp;")}</text>')
    out.append("</svg>")
    return "\n".join(out) + "\n"


def palettes_svg(names):
    pad, sw, label_w, row_h = 14, 62, 90, 30
    groups = GENERAL_GROUPS[1:]
    w = pad * 2 + label_w + len(groups) * sw
    h = 40 + len(names) * row_h + 8
    out = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
           f'font-family="-apple-system, Segoe UI, Helvetica, Arial, sans-serif" role="img">',
           "<title>Zones color palettes</title>",
           f'<rect width="{w}" height="{h}" rx="14" fill="#101010"/>']
    for i, g in enumerate(groups):
        out.append(f'<text x="{pad + label_w + i * sw + sw / 2}" y="26" font-size="11" text-anchor="middle" fill="#bbb">{g}</text>')
    for r, name in enumerate(names):
        pal = palette(f"k4v2_pal_{name}")
        y = 40 + r * row_h
        out.append(f'<text x="{pad}" y="{y + 17}" font-size="13" font-weight="600" fill="#f2f2f2">{name.capitalize()}</text>')
        for i in range(1, len(pal)):
            out.append(f'<rect x="{pad + label_w + (i - 1) * sw + 3}" y="{y + 3}" width="{sw - 6}" height="{row_h - 6}" rx="5" fill="{hsv_hex(*pal[i])}"/>')
    out.append("</svg>")
    return "\n".join(out) + "\n"


def main():
    syms = symbols()
    general = key_map("k4v2_map_general", syms["k4v2_map_general"])
    OUT.mkdir(parents=True, exist_ok=True)
    jobs = [
        ("zones-classic.svg", "Zones: Classic (also the default of Zones: Custom)", general, palette("k4v2_pal_classic"), GENERAL_GROUPS),
        ("zones-gaming.svg", "Zones: Gaming", key_map("k4v2_map_gaming", syms["k4v2_map_gaming"]), palette("k4v2_pal_gaming"), GAMING_GROUPS),
        ("zones-code.svg", "Zones: Code", key_map("k4v2_map_code", syms["k4v2_map_code"]), palette("k4v2_pal_code"), CODE_GROUPS),
    ]
    for fname, title, kmap, pal, groups in jobs:
        assert len(pal) == len(groups), (fname, len(pal), len(groups))
        (OUT / fname).write_text(keyboard_svg(title, kmap, pal, groups))
        print("wrote", (OUT / fname).relative_to(ROOT))
    (OUT / "zones-palettes.svg").write_text(palettes_svg(["classic", "ocean", "sunset", "neon", "ice"]))
    print("wrote", (OUT / "zones-palettes.svg").relative_to(ROOT))


if __name__ == "__main__":
    main()
