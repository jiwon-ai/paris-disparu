#!/usr/bin/env python3
"""Convert the text-based assets into C data for GBDK-2020.

Inputs : tools/art.py, tools/font.py, assets/maps/*.txt,
         assets/dialogue/fr.txt, assets/text/ui.txt
Outputs: include/assets.h, src/assets.c

The script also validates the assets (map sizes, unknown legend characters,
characters missing from the font, words too long for the text box) and exits
with an error instead of producing broken data.
"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import art   # noqa: E402
import font  # noqa: E402

MAP_W, MAP_H = 20, 18
FONT_BASE = 128          # font tiles live at 128.. in VRAM bank 0
TEXT_COLS, TEXT_ROWS = 18, 3
PAGE_SIZE = TEXT_COLS * TEXT_ROWS
ROOMS = ["street", "entrance", "market", "cafe", "quai", "pont"]   # order defines ROOM_* ids

errors = []


def fail(msg):
    errors.append(msg)


# ---------------------------------------------------------------------------
# Tile encoding
# ---------------------------------------------------------------------------
def encode_tile(rows):
    """8 rows of 8 colour digits -> 16 bytes of GB 2bpp data."""
    assert len(rows) == 8, rows
    out = []
    for row in rows:
        assert len(row) == 8, row
        lo = hi = 0
        for x, ch in enumerate(row):
            c = int(ch)
            lo |= (c & 1) << (7 - x)
            hi |= ((c >> 1) & 1) << (7 - x)
        out += [lo, hi]
    return out


# ---------------------------------------------------------------------------
# Font
# ---------------------------------------------------------------------------
BOX_TILES = {
    # text box frame, ink on paper
    "BOX_TL": ["........", "........", "...#####", "..#.....", "..#.....", "..#.....", "..#.....", "..#....."],
    "BOX_T":  ["........", "........", "########", "........", "........", "........", "........", "........"],
    "BOX_TR": ["........", "........", "#####...", ".....#..", ".....#..", ".....#..", ".....#..", ".....#.."],
    "BOX_L":  ["..#....."] * 8,
    "BOX_R":  [".....#.."] * 8,
    "BOX_BL": ["..#.....", "..#.....", "..#.....", "..#.....", "...#####", "........", "........", "........"],
    "BOX_B":  ["........", "........", "........", "........", "########", "........", "........", "........"],
    "BOX_BR": [".....#..", ".....#..", ".....#..", ".....#..", "#####...", "........", "........", "........"],
    # timing bar for the porter's lift
    "BAR":    ["........", "........", "........", "########", "########", "........", "........", "........"],
    "BAR_ZONE": ["........", "........", "########", "########", "########", "########", "........", "........"],
    "BAR_CURSOR": ["...##...", "..####..", ".######.", "########", "########", ".######.", "..####..", "...##..."],
}

font_chars = list(font.GLYPHS.keys())
font_index = {ch: FONT_BASE + i for i, ch in enumerate(font_chars)}
font_data = []
for ch in font_chars:
    rows = font.glyph_rows(ch)
    font_data += encode_tile([r.replace(".", "1").replace("#", "3") for r in rows])
box_index = {}
for name, rows in BOX_TILES.items():
    box_index[name] = FONT_BASE + len(font_chars) + len(box_index)
    font_data += encode_tile([r.replace(".", "1").replace("#", "3") for r in rows])
font_count = len(font_chars) + len(BOX_TILES)
if FONT_BASE + font_count > 256:
    fail("font does not fit in tiles %d-255" % FONT_BASE)


def encode_char(ch, where):
    ch = font.ALIASES.get(ch, ch)
    if ch not in font_index:
        fail("%s: character %r is not in the font" % (where, ch))
        return font_index[" "]
    return font_index[ch]


def encode_text(s, where):
    return [encode_char(c, where) for c in s]


# ---------------------------------------------------------------------------
# Background tiles
# ---------------------------------------------------------------------------
env_names = list(art.TILES.keys())
env_index = {n: i for i, n in enumerate(env_names)}
env_data = []
for n in env_names:
    env_data += encode_tile(art.TILES[n])
if len(env_names) > 256:
    fail("too many background tiles")

# ---------------------------------------------------------------------------
# Sprites
# ---------------------------------------------------------------------------
SPR_MAP = {".": "0", "#": "1", "o": "2", "s": "3"}
spr_data = []
spr_defs = []
for name, rows in art.SPRITE_FRAMES:
    if len(rows) != 16 or any(len(r) != 16 for r in rows):
        fail("sprite %s is not 16x16" % name)
        continue
    rows = ["".join(SPR_MAP[c] for c in r) for r in rows]
    spr_defs.append((name, len(spr_data) // 16))
    for (y0, x0) in ((0, 0), (8, 0), (0, 8), (8, 8)):   # TL, BL, TR, BR
        spr_data += encode_tile([r[x0:x0 + 8] for r in rows[y0:y0 + 8]])
if len(spr_data) // 16 > FONT_BASE:
    fail("sprite tiles overlap the font")

# ---------------------------------------------------------------------------
# Rooms
# ---------------------------------------------------------------------------
ATTR_BANK1 = 0x08
ATTR_PRIORITY = 0x80
rooms = []
for room in ROOMS:
    path = os.path.join(ROOT, "assets", "maps", room + ".txt")
    grid, signs = [], []
    for ln, line in enumerate(open(path, encoding="utf-8"), 1):
        line = line.rstrip("\n")
        if line.startswith(";") or line == "":
            continue
        if line.startswith("!sign "):
            _, x, y, text = line.split(" ", 3)
            signs.append((int(x), int(y), text, "%s:%d" % (path, ln)))
            continue
        grid.append(line)
    if len(grid) != MAP_H or any(len(r) != MAP_W for r in grid):
        fail("%s: map must be %dx%d (got %d rows, widths %s)"
             % (path, MAP_W, MAP_H, len(grid), sorted({len(r) for r in grid})))
        continue
    tiles, attrs, solid = [], [], []
    for y, row in enumerate(grid):
        for x, ch in enumerate(row):
            if ch not in art.LEGEND:
                fail("%s: unknown map character %r at %d,%d" % (path, ch, x, y))
                ch = " "
            entry = art.LEGEND[ch]
            tname, pal, sol = entry[:3]
            prio = len(entry) > 3 and entry[3]
            tiles.append(env_index[tname])
            attrs.append(pal | ATTR_BANK1 | (ATTR_PRIORITY if prio else 0))
            solid.append(1 if sol else 0)
    for x, y, text, where in signs:
        for i, code in enumerate(encode_text(text, where)):
            if x + i >= MAP_W:
                fail("%s: sign runs off the map" % where)
                break
            tiles[y * MAP_W + x + i] = code
            attrs[y * MAP_W + x + i] = 5           # text palette, bank 0
    rooms.append((room, tiles, attrs, solid))

# ---------------------------------------------------------------------------
# UI strings
# ---------------------------------------------------------------------------
ui = []
for ln, line in enumerate(open(os.path.join(ROOT, "assets", "text", "ui.txt"), encoding="utf-8"), 1):
    line = line.rstrip("\n")
    if not line.strip() or line.startswith("#"):
        continue
    key, text = [p.strip() for p in line.split("=", 1)]
    limit = TEXT_COLS if key.startswith(("caption", "pause")) else MAP_W  # boxed text
    if len(text) > limit:
        fail("ui.txt:%d: %r is longer than %d characters" % (ln, key, limit))
    ui.append((key, encode_text(text, "ui.txt:%d" % ln)))

# ---------------------------------------------------------------------------
# Dialogue
# ---------------------------------------------------------------------------
def wrap(paragraph, where):
    lines, cur = [], ""
    for word in paragraph.split():
        if len(word) > TEXT_COLS:
            fail("%s: word %r is longer than %d columns" % (where, word, TEXT_COLS))
            word = word[:TEXT_COLS]
        if not cur:
            cur = word
        elif len(cur) + 1 + len(word) <= TEXT_COLS:
            cur += " " + word
        else:
            lines.append(cur)
            cur = word
    if cur:
        lines.append(cur)
    return lines


dlg_path = os.path.join(ROOT, "assets", "dialogue", "en.txt")
entries = []           # (id, [paragraph, ...])
choices = {}           # id -> (option 1, option 2) or None
para = []
for ln, line in enumerate(open(dlg_path, encoding="utf-8"), 1):
    line = line.rstrip("\n")
    if line.startswith("#"):
        continue
    if line.startswith("[") and line.endswith("]"):
        entries.append((line[1:-1], [], ln))
        choices[line[1:-1]] = None
        continue
    if line.startswith("? ") and entries:
        opts = tuple(o.strip() for o in line[2:].split("|"))
        if len(opts) != 2 or any(not o or len(o) > 7 for o in opts):
            fail("en.txt:%d: a choice needs exactly two options of 1-7 characters" % ln)
        choices[entries[-1][0]] = opts
        continue
    if not entries:
        if line.strip():
            fail("en.txt:%d: text before first [id]" % ln)
        continue
    paras = entries[-1][1]
    if not line.strip():
        if paras and paras[-1]:
            paras.append("")
        continue
    if not paras:
        paras.append("")
    paras[-1] = (paras[-1] + " " + line.strip()).strip()

pages, dlg_index, seen = [], [], set()
for did, paras, ln in entries:
    if did in seen:
        fail("en.txt:%d: duplicate id %s" % (ln, did))
    seen.add(did)
    first = len(pages)
    for p in [p for p in paras if p]:
        lines = wrap(p, "en.txt:[%s]" % did)
        for i in range(0, len(lines), TEXT_ROWS):
            page = []
            for row in range(TEXT_ROWS):
                text = lines[i + row] if i + row < len(lines) else ""
                page += encode_text(text.ljust(TEXT_COLS), "en.txt:[%s]" % did)
            pages.append(page)
    if len(pages) == first:
        fail("en.txt:[%s] has no text" % did)
    if did.startswith("bark_") and len(pages) - first != 1:
        fail("en.txt:[%s]: a bark must fit on one page (3 lines of 18)" % did)
    dlg_index.append((did, first, len(pages) - first))
choice_pairs = []      # distinct option pairs; dlg_choice holds index + 1
dlg_choice = []
for did, _, _ in dlg_index:
    opts = choices.get(did)
    if opts is None:
        dlg_choice.append(0)
        continue
    if opts not in choice_pairs:
        choice_pairs.append(opts)
    dlg_choice.append(choice_pairs.index(opts) + 1)
choice_text = []
for a, b in choice_pairs:
    choice_text.append(encode_text(a.ljust(7), "choice") + encode_text(b.ljust(7), "choice"))
if len(pages) > 255:
    fail("too many dialogue pages for an 8-bit page index")

if errors:
    for e in errors:
        print("gen_assets: error: " + e, file=sys.stderr)
    sys.exit(1)

# ---------------------------------------------------------------------------
# Output
# ---------------------------------------------------------------------------
def c_bytes(data, per_line=16):
    lines = []
    for i in range(0, len(data), per_line):
        lines.append("    " + ",".join("0x%02X" % b for b in data[i:i + per_line]) + ",")
    return "\n".join(lines)


def c_name(s):
    return s.upper().replace("-", "_")


def rgb(c):
    r, g, b = c
    return "RGB(%d, %d, %d)" % (r, g, b)


H = []
H.append("/* Generated by tools/gen_assets.py - do not edit by hand. */")
H.append("#ifndef ASSETS_H\n#define ASSETS_H\n\n#include <gb/gb.h>\n#include <gb/cgb.h>\n#include <stdint.h>\n")
H.append("#define MAP_W %d\n#define MAP_H %d" % (MAP_W, MAP_H))
H.append("#define TEXT_COLS %d\n#define TEXT_ROWS %d\n#define PAGE_SIZE %d\n" % (TEXT_COLS, TEXT_ROWS, PAGE_SIZE))
H.append("#define ASSET_BANK 1   /* bank of src/assets.c, normally mapped */")
H.append("#define DLG_BANK 2     /* bank of src/dialogue_text.c */")
H.append("#define FONT_BASE %d\n#define FONT_TILE_COUNT %d" % (FONT_BASE, font_count))
H.append("#define GLYPH_SPACE %d" % font_index[" "])
H.append("#define GLYPH_ARROW_DOWN %d" % font_index["▼"])
H.append("#define GLYPH_ARROW_RIGHT %d" % font_index["▶"])
for name, idx in box_index.items():
    H.append("#define GLYPH_%s %d" % (name, idx))
H.append("\n#define ENV_TILE_COUNT %d" % len(env_names))
for name, idx in env_index.items():
    H.append("#define TILE_%s %d" % (c_name(name), idx))
H.append("#define SPRITE_TILE_COUNT %d" % (len(spr_data) // 16))
for name, idx in spr_defs:
    H.append("#define SPR_%s %d" % (name, idx))
H.append("")
for i, (room, _, _, _) in enumerate(rooms):
    H.append("#define ROOM_%s %d" % (c_name(room), i))
H.append("#define ROOM_COUNT %d\n" % len(rooms))
H.append("enum {")
for did, _, _ in dlg_index:
    H.append("    DLG_%s," % c_name(did))
H.append("    DLG_COUNT\n};\n")
H.append("extern const palette_color_t bg_palettes[32];")
H.append("extern const palette_color_t spr_palettes[32];")
H.append("extern const uint8_t font_tiles[];")
H.append("extern const uint8_t env_tiles[];")
H.append("extern const uint8_t water_frames[];   /* 2 tiles, swapped into TILE_WATER */")
H.append("extern const uint8_t sprite_tiles[];")
H.append("extern const uint8_t * const room_tiles[ROOM_COUNT];")
H.append("extern const uint8_t * const room_attrs[ROOM_COUNT];")
H.append("extern const uint8_t * const room_solid[ROOM_COUNT];")
H.append("extern const uint8_t dlg_pages[];")
H.append("extern const uint8_t dlg_first_page[DLG_COUNT];")
H.append("extern const uint8_t dlg_page_count[DLG_COUNT];")
H.append("extern const uint8_t dlg_choice[DLG_COUNT];   /* 0 = none, else pair index + 1 */")
H.append("extern const uint8_t choice_text[];           /* 14 bytes per pair: 7 + 7 */")
for key, _ in ui:
    H.append("extern const uint8_t txt_%s[];" % key)
H.append("\n#endif")

# All asset data lives in ROM bank 1, which is mapped at 0x4000 and never
# switched out, so code can read it directly. Code stays in bank 0.
C = ["/* Generated by tools/gen_assets.py - do not edit by hand. */", "#pragma bank 1", '#include "assets.h"', ""]
C.append("const palette_color_t bg_palettes[32] = {")
for p in art.BG_PALETTES:
    C.append("    " + ", ".join(rgb(c) for c in p) + ",")
C.append("};\n")
C.append("const palette_color_t spr_palettes[32] = {")
for p in art.SPR_PALETTES:
    C.append("    " + ", ".join(rgb(c) for c in p) + ",")
C.append("};\n")
C.append("const uint8_t font_tiles[] = {\n%s\n};\n" % c_bytes(font_data))
C.append("const uint8_t env_tiles[] = {\n%s\n};\n" % c_bytes(env_data))
C.append("const uint8_t water_frames[] = {\n%s\n};\n" % c_bytes([b for f in art.WATER_FRAMES for b in encode_tile(f)]))
C.append("const uint8_t sprite_tiles[] = {\n%s\n};\n" % c_bytes(spr_data))
for room, tiles, attrs, solid in rooms:
    C.append("static const uint8_t %s_tiles[] = {\n%s\n};" % (room, c_bytes(tiles, MAP_W)))
    C.append("static const uint8_t %s_attrs[] = {\n%s\n};" % (room, c_bytes(attrs, MAP_W)))
    C.append("static const uint8_t %s_solid[] = {\n%s\n};\n" % (room, c_bytes(solid, MAP_W)))
for kind in ("tiles", "attrs", "solid"):
    C.append("const uint8_t * const room_%s[ROOM_COUNT] = { %s };"
             % (kind, ", ".join("%s_%s" % (r[0], kind) for r in rooms)))
C.append("")
flat = [b for p in pages for b in p]
if len(flat) > 16384:
    sys.exit("gen_assets: error: dialogue pages exceed one 16 KiB ROM bank")
T = ["/* Generated by tools/gen_assets.py - do not edit by hand. */",
     "/* Dialogue text lives in ROM bank DLG_BANK: map it with SWITCH_ROM before reading. */",
     "#pragma bank 2", '#include "assets.h"', "",
     "const uint8_t dlg_pages[] = {\n%s\n};" % c_bytes(flat, PAGE_SIZE // 3), ""]
C.append("const uint8_t dlg_first_page[DLG_COUNT] = { %s };" % ", ".join(str(f) for _, f, _ in dlg_index))
C.append("const uint8_t dlg_page_count[DLG_COUNT] = { %s };" % ", ".join(str(n) for _, _, n in dlg_index))
C.append("const uint8_t dlg_choice[DLG_COUNT] = { %s };" % ", ".join(str(c) for c in dlg_choice))
C.append("const uint8_t choice_text[] = {\n%s\n};\n" % c_bytes([b for t in choice_text for b in t] or [0], 14))
for key, data in ui:
    C.append("const uint8_t txt_%s[] = { %s, 0x00 };" % (key, ",".join("0x%02X" % b for b in data)))
C.append("")

outputs = {
    os.path.join(ROOT, "include", "assets.h"): "\n".join(H) + "\n",
    os.path.join(ROOT, "src", "assets.c"): "\n".join(C) + "\n",
    os.path.join(ROOT, "src", "dialogue_text.c"): "\n".join(T) + "\n",
}
if "--check" in sys.argv:
    stale = [p for p, text in outputs.items()
             if not os.path.exists(p) or open(p, encoding="utf-8").read() != text]
    for p in stale:
        print("gen_assets: error: %s is out of date, run 'make assets'" % os.path.relpath(p, ROOT),
              file=sys.stderr)
    if stale:
        sys.exit(1)
else:
    for p, text in outputs.items():
        with open(p, "w", encoding="utf-8") as f:
            f.write(text)
print("gen_assets: %d font tiles, %d bg tiles, %d sprite tiles, %d rooms, %d dialogue pages"
      % (font_count, len(env_names), len(spr_data) // 16, len(rooms), len(pages)))
