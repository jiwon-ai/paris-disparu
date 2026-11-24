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
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.join(ROOT, "tools"))

import art   # noqa: E402
import font  # noqa: E402
import title_art  # noqa: E402
import portraits  # noqa: E402
import items  # noqa: E402

MAP_W, MAP_H = 20, 18
FONT_BASE = 128          # font tiles live at 128.. in VRAM bank 0
TEXT_COLS, TEXT_ROWS = 18, 2   # dialogue: 2 lines a page, a blank row between
PAGE_SIZE = TEXT_COLS * TEXT_ROWS
ROOMS = ["street", "entrance", "market", "cafe", "quai", "pont", "metro", "bourse", "hotel", "brasserie"]   # order defines ROOM_* ids

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
    # coffee gauge: the dark zone is framed above and below, coffee is mid tone
    "POUR_ZONE": ["........", "########", "........", "########", "########", "........", "########", "........"],
    "COFFEE": ["........", "........", "22222222", "22222222", "22222222", "22222222", "........", "........"],
    "COFFEE_ZONE": ["........", "########", "22222222", "22222222", "22222222", "22222222", "########", "........"],
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
if len(env_names) > 128:   # VRAM bank 1 BG tiles 0-127 (0x9000-0x97FF)
    fail("too many background tiles (%d > 128)" % len(env_names))

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

# Dialogue portraits: 24x32 busts = 3 columns x 2 rows of 8x16 sprites.
PORTRAIT_BASE = 64      # sprite tiles 64.. in VRAM bank 1 (0x8400); scenery is at 0x9000
portrait_data = []
for name, grid, _ in portraits.PORTRAITS:
    rows = ["".join(SPR_MAP[c] for c in r) for r in portraits.rows(grid)]
    for col in range(3):
        for half in range(2):
            for y0 in (half * 16, half * 16 + 8):
                portrait_data += encode_tile([r[col * 8:col * 8 + 8] for r in rows[y0:y0 + 8]])
# Cameo sprites (16x16, one frame) follow the portraits in VRAM bank 1.
bank1_spr_defs = []
for name, rows in art.BANK1_SPRITES:
    rows = ["".join(SPR_MAP[c] for c in r) for r in rows]
    bank1_spr_defs.append((name, PORTRAIT_BASE + len(portrait_data) // 16))
    for (y0, x0) in ((0, 0), (8, 0), (0, 8), (8, 8)):   # TL, BL, TR, BR
        portrait_data += encode_tile([r[x0:x0 + 8] for r in rows[y0:y0 + 8]])
for name, rows in art.BANK1_ICONS:                       # 8x16: top, bottom
    rows = ["".join(SPR_MAP[c] for c in r) for r in rows]
    bank1_spr_defs.append((name, PORTRAIT_BASE + len(portrait_data) // 16))
    portrait_data += encode_tile(rows[0:8]) + encode_tile(rows[8:16])
# Extra sprites at tile 0.. of VRAM bank 1 (free since scenery is at 0x9000).
low_data, low_defs = [], []
for name, rows in art.BANK1_LOW_SPRITES:
    if len(rows) != 16 or any(len(r) != 16 for r in rows):
        fail("sprite %s is not 16x16" % name)
        continue
    rows = ["".join(SPR_MAP[c] for c in r) for r in rows]
    low_defs.append((name, len(low_data) // 16))
    for (y0, x0) in ((0, 0), (8, 0), (0, 8), (8, 8)):
        low_data += encode_tile([r[x0:x0 + 8] for r in rows[y0:y0 + 8]])
if len(low_data) // 16 > PORTRAIT_BASE:
    fail("low bank-1 sprites overlap the portraits")
# Item pictures: same 24x32 layout, loaded one at a time to bank-1 tile ITEM_TILE.
ITEM_TILE = 52            # after the low sprites; 12 tiles, up to PORTRAIT_BASE 64
if len(low_data) // 16 > ITEM_TILE:
    fail("low bank-1 sprites overlap the item picture tiles")
item_data = []
for name, grid, _ in items.ITEMS:
    rows = ["".join(SPR_MAP[c] for c in r) for r in portraits.rows(grid)]
    for col in range(3):
        for half in range(2):
            for y0 in (half * 16, half * 16 + 8):
                item_data += encode_tile([r[col * 8:col * 8 + 8] for r in rows[y0:y0 + 8]])
portrait_names = [n for n, _, _ in portraits.PORTRAITS]
if PORTRAIT_BASE + len(portrait_data) // 16 > 256:
    fail("portrait tiles do not fit in VRAM bank 1")

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
        if line.startswith("!sign ") or line.startswith("!plaque "):
            kind, x, y, text = line.split(" ", 3)
            pal = 7 if kind == "!plaque" else 5   # 7: white on blue enamel
            signs.append((int(x), int(y), text, "%s:%d" % (path, ln), pal))
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
    for x, y, text, where, pal in signs:
        for i, code in enumerate(encode_text(text, where)):
            if x + i >= MAP_W:
                fail("%s: sign runs off the map" % where)
                break
            tiles[y * MAP_W + x + i] = code
            attrs[y * MAP_W + x + i] = pal         # text palette, bank 0
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
    limit = MAP_W if key.startswith(("title", "dmg")) else TEXT_COLS  # the rest is boxed
    if len(text) > limit:
        fail("ui.txt:%d: %r is longer than %d characters" % (ln, key, limit))
    ui.append((key, encode_text(text, "ui.txt:%d" % ln)))
# Paris telephone exchanges (ex_0.. in ui.txt): only names read in the source
# (britishtelephones.com, Paris exchange codes; HISTORICAL_RESEARCH claim 13).
ui_keys = [k for k, _ in ui]
EXCHANGES = []
while "ex_%d" % len(EXCHANGES) in ui_keys:
    EXCHANGES.append(len(EXCHANGES))
if len(EXCHANGES) < 2:
    fail("ui.txt: the telephone exchanges ex_0.. are missing")

# ---------------------------------------------------------------------------
# Dialogue
# ---------------------------------------------------------------------------
def wrap(paragraph, where):
    lines, cur = [], ""
    for word in [w for w in paragraph.split(" ") if w]:   # a no-break space joins
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


def parse_dialogue(path, name):
    """[id] entries of paragraphs, and "? a | b" choices, from a dialogue file."""
    entries, choices, para = [], {}, []
    for ln, line in enumerate(open(path, encoding="utf-8"), 1):
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
                fail("%s:%d: a choice needs exactly two options of 1-7 characters" % (name, ln))
            choices[entries[-1][0]] = opts
            continue
        if not entries:
            if line.strip():
                fail("%s:%d: text before first [id]" % (name, ln))
            continue
        paras = entries[-1][1]
        if not line.strip():
            if paras and paras[-1]:
                paras.append("")
            continue
        if not paras:
            paras.append("")
        paras[-1] = (paras[-1] + " " + line.strip()).strip()
    return entries, choices


def paginate(did, paras, name):
    """The pages (TEXT_ROWS x TEXT_COLS glyphs each) of one entry."""
    out = []
    for p in [p for p in paras if p]:
        lines = wrap(p, "%s:[%s]" % (name, did))
        # Spread a long paragraph evenly over its pages (5 lines: 3 + 2, not
        # 3 + 3; 4 lines: 2 + 2), so no page holds a lone orphan line.
        n_pages = -(-len(lines) // TEXT_ROWS)
        i = 0
        for k in range(n_pages):
            take = -(-(len(lines) - i) // (n_pages - k))
            page = []
            for row in range(TEXT_ROWS):
                text = lines[i + row] if row < take else ""
                page += encode_text(text.ljust(TEXT_COLS), "%s:[%s]" % (name, did))
            out.append(page)
            i += take
    if not out:
        fail("%s:[%s] has no text" % (name, did))
    if did.startswith("bark_") and len(out) != 1:
        fail("%s:[%s]: a bark must fit on one page (2 lines of 18)" % (name, did))
    return out


entries, choices = parse_dialogue(os.path.join(ROOT, "assets", "dialogue", "en.txt"), "en.txt")

pages, dlg_index, seen = [], [], set()
for did, paras, ln in entries:
    if did in seen:
        fail("en.txt:%d: duplicate id %s" % (ln, did))
    seen.add(did)
    first = len(pages)
    pages += paginate(did, paras, "en.txt")
    dlg_index.append((did, first, len(pages) - first))
# Portrait = the speaker named before ':' at the start of the entry.
dlg_portrait = []
for did, paras, ln in entries:
    first = [p for p in paras if p][0]
    who = first.split(":", 1)[0] if ":" in first[:20] else ""
    who = portraits.ALIASES.get(who, who)
    dlg_portrait.append(portrait_names.index(who) if who in portrait_names else 0xFF)
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
# Pages fill ROM bank 2; the rest go to banks DLG_HI_BANK.. (16 KiB each).
DLG_PAGES_LO = 16384 // PAGE_SIZE
DLG_HI_BANK, DLG_HI_LAST = 8, 10
DLG_HI_PER_BANK = 16384 // PAGE_SIZE
n_hi_banks = max(1, -(-(len(pages) - DLG_PAGES_LO) // DLG_HI_PER_BANK))
if DLG_HI_BANK + n_hi_banks - 1 > DLG_HI_LAST:
    fail("dialogue pages need %d overflow banks: more than banks %d-%d"
         % (n_hi_banks, DLG_HI_BANK, DLG_HI_LAST))
if len(dlg_index) > 0xF6:
    fail("too many dialogue entries: ids 0xF7-0xFF are reserved for events")
# Hotspots (src/rooms.c) store a dialogue id or TALK(npc) = 0x80 | npc, so
# every dialogue a hotspot names must have an id below 0x80.
_names = [n for n, _, _ in dlg_index]
for _m in sorted(set(re.findall(r"\bDLG_(\w+)", open(os.path.join(ROOT, "src", "rooms.c")).read()))):
    if _m.lower() in _names and _names.index(_m.lower()) >= 0x80:
        fail("rooms.c: hotspot dialogue DLG_%s has id >= 0x80; move it up in en.txt" % _m)

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
H.append("#define ROOM_BANK 16   /* bank of src/room_data.c: room maps */")
H.append("#define TITLE_BANK 3   /* bank of src/title_art.c */")
H.append("#define PORTRAIT_BANK 4 /* bank of src/portrait_art.c */")
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
for name, t in bank1_spr_defs:
    H.append("#define SPR1_%s %d   /* VRAM bank 1: use palette | S_BANK1 */" % (name, t))
for name, t in low_defs:
    H.append("#define SPR1_%s %d   /* VRAM bank 1, low tiles */" % (name, t))
H.append("#define LOW_TILE_COUNT %d" % (len(low_data) // 16))
H.append("/* VRAM bank 1 sprite tiles of the item picture (12 from ITEM_TILE) */")
H.append("#define ITEM_TILE %d" % ITEM_TILE)
H.append("#define NO_ITEM 0")
H.append("#define EXCH_COUNT %d" % len(EXCHANGES))
_ui_text = {}
for _ln in open(os.path.join(ROOT, "assets", "text", "ui.txt"), encoding="utf-8"):
    if "=" in _ln and not _ln.startswith("#"):
        _k, _v = [p.strip() for p in _ln.split("=", 1)]
        _ui_text[_k] = _v
for _i in EXCHANGES:
    _name = _ui_text["ex_%d" % _i].upper().replace("É", "E")
    H.append("#define EX_%s %d" % (_name, _i))
for k, (name, _, _) in enumerate(items.ITEMS):
    H.append("#define ITEM_%s %d" % (name, k + 1))
H.append("#define GLYPH_DIGIT0 %d\n#define GLYPH_CUP %d\n#define GLYPH_COIN %d"
         % (font_index["0"], encode_char("∩", "glyph"), encode_char("●", "glyph")))
H.append("#define PORTRAIT_BASE %d\n#define PORTRAIT_TILE_COUNT %d\n#define NO_PORTRAIT 0xFF"
         % (PORTRAIT_BASE, len(portrait_data) // 16))
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
H.append("extern const uint8_t env_tiles[];             /* bank PORTRAIT_BANK */")
H.append("extern const uint8_t water_frames[];   /* 2 tiles, swapped into TILE_WATER */")
H.append("extern const uint8_t sprite_tiles[];")
H.append("extern const uint8_t * const room_tiles[ROOM_COUNT];   /* bank ROOM_BANK */")
H.append("extern const uint8_t * const room_attrs[ROOM_COUNT];")
H.append("extern const uint8_t * const room_solid[ROOM_COUNT];")
H.append("extern const uint8_t dlg_pages[];")
H.append("extern const uint16_t dlg_first_page[DLG_COUNT];   /* bank TITLE_BANK */")
H.append("extern const uint8_t dlg_page_count[DLG_COUNT];   /* bank TITLE_BANK */")
H.append("extern const uint8_t dlg_choice[DLG_COUNT];   /* bank TITLE_BANK; 0 = none, else pair index + 1 */")
H.append("extern const uint8_t choice_text[];           /* 14 bytes per pair: 7 + 7 */")
H.append("extern const uint8_t portrait_tiles[];        /* bank PORTRAIT_BANK */")
H.append("extern const uint8_t item_tiles[];            /* bank PORTRAIT_BANK, 192 bytes per item */")
H.append("extern const uint8_t item_pal[];              /* bank PORTRAIT_BANK */")
H.append("extern const uint8_t low_tiles[];             /* bank PORTRAIT_BANK, VRAM bank 1 tile 0.. */")
H.append("extern const uint8_t portrait_pal[];          /* sprite palette per portrait */")
H.append("extern const uint8_t dlg_portrait[DLG_COUNT]; /* speaker of the first page, or NO_PORTRAIT */")
# All UI strings in one array, so src/ko.c can find a string by its offset.
H.append("extern const uint8_t ui_text[];")
ui_offsets, _off = {}, 0
for key, data in ui:
    ui_offsets[key] = _off
    H.append("#define txt_%s (ui_text + %d)" % (key, _off))
    _off += len(data) + 1
import fr_gen  # noqa: E402
fr_out, fr_header, fr_report = fr_gen.build(
    ROOT, entries, choices, choice_pairs, ui, ui_offsets, PAGE_SIZE, parse_dialogue, paginate,
    encode_text, lambda k: MAP_W if k.startswith(("title", "dmg")) else TEXT_COLS, fail)
H += fr_header
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
C.append("const uint8_t water_frames[] = {\n%s\n};\n" % c_bytes([b for f in art.WATER_FRAMES for b in encode_tile(f)]))
C.append("const uint8_t sprite_tiles[] = {\n%s\n};\n" % c_bytes(spr_data))
RD = ["/* Generated by tools/gen_assets.py from assets/maps - do not edit by hand. */",
      "/* Lives in ROM bank ROOM_BANK: map it with SWITCH_ROM before reading. */",
      "#pragma bank 16", '#include "assets.h"', ""]
for room, tiles, attrs, solid in rooms:
    RD.append("static const uint8_t %s_tiles[] = {\n%s\n};" % (room, c_bytes(tiles, MAP_W)))
    RD.append("static const uint8_t %s_attrs[] = {\n%s\n};" % (room, c_bytes(attrs, MAP_W)))
    RD.append("static const uint8_t %s_solid[] = {\n%s\n};\n" % (room, c_bytes(solid, MAP_W)))
for kind in ("tiles", "attrs", "solid"):
    RD.append("const uint8_t * const room_%s[ROOM_COUNT] = { %s };"
              % (kind, ", ".join("%s_%s" % (r[0], kind) for r in rooms)))
RD.append("")
flat = [b for p in pages[:DLG_PAGES_LO] for b in p]
hi_pages = pages[DLG_PAGES_LO:]
HI = {}
for k in range(n_hi_banks):
    chunk = [b for pg in hi_pages[k * DLG_HI_PER_BANK:(k + 1) * DLG_HI_PER_BANK] for b in pg] or [0]
    HI[os.path.join(ROOT, "src", "dlg_pages_%d.c" % k)] = "\n".join([
        "/* Generated by tools/gen_assets.py - do not edit by hand. */",
        "#pragma bank %d" % (DLG_HI_BANK + k), '#include "assets.h"', "",
        "const uint8_t dlg_pages_%d[] = {\n%s\n};" % (k, c_bytes(chunk, PAGE_SIZE // 3)), ""]) + "\n"
C.append("const uint8_t * const dlg_hi_base[] = { %s };"
         % ", ".join("dlg_pages_%d" % k for k in range(n_hi_banks)))
T = ["/* Generated by tools/gen_assets.py - do not edit by hand. */",
     "/* Dialogue text lives in ROM bank DLG_BANK: map it with SWITCH_ROM before reading. */",
     "#pragma bank 2", '#include "assets.h"', "",
     "const uint8_t dlg_pages[] = {\n%s\n};" % c_bytes(flat, PAGE_SIZE // 3), ""]
C.append("const uint8_t portrait_pal[] = { %s };" % ", ".join(str(p) for _, _, p in portraits.PORTRAITS))
C.append("const uint8_t dlg_portrait[DLG_COUNT] = { %s };" % ", ".join(str(p) for p in dlg_portrait))
C.append("const uint8_t choice_text[] = {\n%s\n};\n" % c_bytes([b for t in choice_text for b in t] or [0], 14))
C.append("const uint8_t ui_text[] = {")
for key, data in ui:
    C.append("    %s0x00,   /* %s */" % ("".join("0x%02X," % b for b in data), key))
C.append("};")
C.append("")

# Title key art (tools/title_art.py): tiles go to VRAM bank 1 during the title.
_, t_tiles, t_map, t_attr, t_pals = title_art.build()
if len(t_tiles) > 256:
    fail("title art has %d unique tiles (max 256)" % len(t_tiles))
H.insert(-1, "#define TITLE_TILE_COUNT %d" % len(t_tiles))
H.insert(-1, "extern const uint8_t title_tiles[];   /* bank TITLE_BANK */")
H.insert(-1, "extern const uint8_t title_map[];")
H.insert(-1, "extern const uint8_t title_attr[];")
H.insert(-1, "extern const palette_color_t title_pal[];")
H.insert(-1, "#define DLG_PAGES_LO %d   /* pages in bank DLG_BANK; the rest in banks DLG_HI_BANK.. */" % DLG_PAGES_LO)
H.insert(-1, "#define DLG_HI_BANK %d" % DLG_HI_BANK)
H.insert(-1, "#define DLG_HI_PER_BANK %d" % DLG_HI_PER_BANK)
H.insert(-1, "extern const uint8_t * const dlg_hi_base[];   /* bank ASSET_BANK */")
for k in range(n_hi_banks):
    H.insert(-1, "extern const uint8_t dlg_pages_%d[];" % k)
TA = ["/* Generated by tools/gen_assets.py from tools/title_art.py - do not edit by hand. */",
      "/* Lives in ROM bank TITLE_BANK: map it with SWITCH_ROM before reading. */",
      "#pragma bank 3", '#include "assets.h"', "",
      "const uint8_t title_tiles[] = {\n%s\n};" % c_bytes([b for t in t_tiles for b in encode_tile(t)]),
      "const uint8_t title_map[] = {\n%s\n};" % c_bytes(t_map, MAP_W),
      "/* palette | 0x08: tiles are in VRAM bank 1 */",
      "const uint8_t title_attr[] = {\n%s\n};" % c_bytes([a | 0x08 for a in t_attr], MAP_W),
      "const palette_color_t title_pal[] = {\n%s\n};" % ",\n".join(
          "    " + ", ".join(rgb(c) for c in p) for p in t_pals),
      "/* First page of each dialogue (read with far_read: bank 1 is full). */",
      "const uint16_t dlg_first_page[DLG_COUNT] = { %s };" % ", ".join(str(f) for _, f, _ in dlg_index),
      "/* Choice pair of each dialogue (read with far_read: bank 1 is full). */",
      "const uint8_t dlg_choice[DLG_COUNT] = { %s };" % ", ".join(str(c) for c in dlg_choice),
      "/* Pages of each dialogue (read with far_read: bank 1 is full). */",
      "const uint8_t dlg_page_count[DLG_COUNT] = { %s };" % ", ".join(str(n) for _, _, n in dlg_index), ""]

outputs = {
    os.path.join(ROOT, "src", "title_art.c"): "\n".join(TA) + "\n",
    os.path.join(ROOT, "include", "assets.h"): "\n".join(H) + "\n",
    os.path.join(ROOT, "src", "assets.c"): "\n".join(C) + "\n",
    os.path.join(ROOT, "src", "dialogue_text.c"): "\n".join(T) + "\n",
    os.path.join(ROOT, "src", "room_data.c"): "\n".join(RD) + "\n",
    os.path.join(ROOT, "src", "portrait_art.c"): "\n".join([
        "/* Generated by tools/gen_assets.py from tools/portraits.py - do not edit by hand. */",
        "/* Lives in ROM bank PORTRAIT_BANK: map it with SWITCH_ROM before reading. */",
        "#pragma bank 4", '#include "assets.h"', "",
        "const uint8_t portrait_tiles[] = {\n%s\n};" % c_bytes(portrait_data),
        "const uint8_t low_tiles[] = {\n%s\n};" % c_bytes(low_data or [0]),
        "const uint8_t env_tiles[] = {\n%s\n};" % c_bytes(env_data),
        "const uint8_t item_tiles[] = {\n%s\n};" % c_bytes(item_data),
        "const uint8_t item_pal[] = { %s };" % ", ".join(str(p) for _, _, p in items.ITEMS), ""]) + "\n",
}
import ko_gen  # noqa: E402
ko_out, ko_report = ko_gen.build([(d, p) for d, p, _ in entries], {k: v for k, v in choices.items() if v},
                                 ui, ui_offsets, encode_char, fail)
outputs.update(ko_out)
outputs.update(fr_out)
outputs.update(HI)
print("fr_gen: %d pages in %d banks; untranslated: %d dialogue, %d ui"
      % (fr_report["pages"], fr_report["banks"], len(fr_report["missing_dlg"]), len(fr_report["missing_ui"])))
if errors:
    for e in errors:
        print("gen_assets: error: " + e, file=sys.stderr)
    sys.exit(1)
_stale_blobs = [os.path.join(ROOT, "src", f) for f in os.listdir(os.path.join(ROOT, "src"))
                if f.startswith(("ko_blob_", "fr_pages_", "dlg_pages_")) and os.path.join(ROOT, "src", f) not in outputs]
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
    for p in _stale_blobs:
        os.remove(p)
print("gen_assets: %d font tiles, %d bg tiles, %d sprite tiles, %d rooms, %d dialogue pages"
      % (font_count, len(env_names), len(spr_data) // 16, len(rooms), len(pages)))
print(ko_report)
