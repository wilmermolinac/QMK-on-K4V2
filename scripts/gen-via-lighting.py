#!/usr/bin/env python3
"""Writes the custom Lighting menu into via/k4v2_ansi_via.json and via/k4v2_iso_via.json.

The effect numbers must match the firmware's effect list: the built-in effects
left enabled by firmware/.../v2/post_config.h, then the Zones effects from
rgb_matrix_kb.inc. k4v2_zones.c has static asserts that fail the build if they
drift apart, so update EFFECTS here and the asserts there together.

Usage: python3 scripts/gen-via-lighting.py
"""
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent

EFFECTS = [
    ("All Off", 0),
    ("Solid Color", 1),
    ("Breathing", 2),
    ("Cycle All", 3),
    ("Cycle Left/Right", 4),
    ("Rainbow Beacon", 5),
    ("Raindrops", 6),
    ("Hue Wave", 7),
    ("Reactive Simple", 8),
    ("Splash", 9),
    ("Zones: Custom (edit below)", 10),
    ("Zones: Classic", 11),
    ("Zones: Ocean", 12),
    ("Zones: Sunset", 13),
    ("Zones: Neon", 14),
    ("Zones: Forest", 15),
    ("Zones: Candy", 16),
    ("Zones: Ice", 17),
]
ZONES_CUSTOM = 10

# VIA value IDs 1..9 = enum k4v2_zone in k4v2_zones.h (custom channel 0).
ZONES = ["Esc", "F1-F12", "Numbers", "Letters", "Symbols", "Modifiers", "Navigation", "Numpad", "Light key"]

EFFECT = "{id_qmk_rgb_matrix_effect}"


def lighting_menu():
    backlight = [
        {"label": "Brightness", "type": "range", "options": [0, 255],
         "content": ["id_qmk_rgb_matrix_brightness", 3, 1]},
        {"label": "Effect", "type": "dropdown", "options": [list(e) for e in EFFECTS],
         "content": ["id_qmk_rgb_matrix_effect", 3, 2]},
        {"showIf": f"{EFFECT} != 0 && {EFFECT} != 1 && {EFFECT} < {ZONES_CUSTOM}",
         "label": "Effect Speed", "type": "range", "options": [0, 255],
         "content": ["id_qmk_rgb_matrix_effect_speed", 3, 3]},
        {"showIf": f"{EFFECT} != 0 && {EFFECT} != {ZONES_CUSTOM}",
         "label": "Color (rotates the fixed Zones palettes)", "type": "color",
         "content": ["id_qmk_rgb_matrix_color", 3, 4]},
    ]
    zones = [
        {"showIf": f"{EFFECT} == {ZONES_CUSTOM}", "label": name, "type": "color",
         "content": [f"id_k4v2_zone_{i}", 0, i]}
        for i, name in enumerate(ZONES, start=1)
    ]
    return {"label": "Lighting", "content": [
        {"label": "Backlight", "content": backlight},
        {"label": "Zones colors", "content": zones},
    ]}


for layout in ("ansi", "iso"):
    path = ROOT / "via" / f"k4v2_{layout}_via.json"
    data = json.loads(path.read_text())
    data["menus"] = [lighting_menu()]
    path.write_text(json.dumps(data, indent=2) + "\n")
    print(f"updated {path.relative_to(ROOT)}")
