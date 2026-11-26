"""Korean text data for PARIS DISPARU (called by tools/gen_assets.py).

Inputs : assets/dialogue/ko.txt, assets/text/ui_ko.txt (same ids/keys as the
         English files), tools/fonts/galmuri/Galmuri11.bdf and Galmuri7.bdf.
Outputs: include/ko_data.h, src/ko_data.c (ROM bank KO_BANK, read by src/ko.c),
         src/ko_blob_N.c (ROM banks KO_BLOB_BANK.., glyphs and dialogue pages).

Dialogue (Galmuri11, variable width): each page is two lines of at most
LINE_PX pixels, drawn by src/ko.c into 16-pixel-high strips of window tiles.
Page record: n1, n2, then n1 + n2 glyph indices (2 bytes each, high first).
Glyph record (GLYPH_SIZE bytes): advance width, then 14 rows of 16 bits
(high byte = left 8 pixels); row 10 sits on the baseline.

Menus and notebook: a UI string with Hangul is one Galmuri11 line, 16 pixels
high (record 0x01, glyph count, glyph indices); src/ko.c lays such boxes out
with two tile rows per line. A string without Hangul keeps the 8x8 English
font (tile bytes 0x80.., 0-terminated).

An id or key missing from the Korean files falls back to the English text
(drawn with Galmuri11 / the English font) and is counted as untranslated.
"""
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
FONT_DIR = os.path.join(ROOT, "tools", "fonts", "galmuri")

KO_BANK = 6            # src/ko.c and src/ko_data.c
KO_BLOB_BANK = 20      # first bank of glyph and page blobs
KO_LAST_BANK = 31      # ROM is 32 banks (512 KiB)
LINE_PX = 144          # 18 tiles
GLYPH_ROWS = 14
GLYPH_SIZE = 1 + 2 * GLYPH_ROWS
BASE_ROW = 10          # glyph row just above the baseline
MAX_PAGE_GLYPHS = 64
UI_CACHE_SMALL = 56    # syllable tiles free while a dialogue box is open
UI_CACHE_FULL = 128    # full-screen boxes (notebook, rules, intro)


def load_bdf(path):
    glyphs, cur, bm = {}, None, None
    for line in open(path, encoding="latin-1"):
        p = line.split()
        if not p:
            continue
        if p[0] == "ENCODING":
            cur = int(p[1])
        elif p[0] == "DWIDTH":
            dw = int(p[1])
        elif p[0] == "BBX":
            bbx = tuple(int(v) for v in p[1:5])
        elif p[0] == "BITMAP":
            bm = []
        elif p[0] == "ENDCHAR":
            glyphs[cur] = (dw, bbx, bm)
            bm = None
        elif bm is not None:
            bm.append(p[0])
    return glyphs


def glyph_rows(g, n_rows, base_row, width):
    """BDF glyph -> n_rows ints of `width` bits (MSB = left), clipped."""
    dw, (w, h, xo, yo), bm = g
    rows = [0] * n_rows
    for r, hexrow in enumerate(bm):
        bits = int(hexrow, 16)
        nb = len(hexrow) * 4
        y = base_row - (yo + h - 1) + r
        if not 0 <= y < n_rows:
            continue
        for c in range(w):
            if bits >> (nb - 1 - c) & 1:
                x = xo + c
                if 0 <= x < width:
                    rows[y] |= 1 << (width - 1 - x)
    return rows


def is_hangul(ch):
    return 0xAC00 <= ord(ch) <= 0xD7A3


def parse_dialogue(path, fail):
    """Same format as en.txt: {id: ([paragraph, ...], choice or None)}."""
    out, cur = {}, None
    if not os.path.exists(path):
        return out
    for ln, line in enumerate(open(path, encoding="utf-8"), 1):
        line = line.rstrip("\n")
        if line.startswith("#"):
            continue
        if line.startswith("[") and line.endswith("]"):
            cur = line[1:-1]
            if cur in out:
                fail("ko.txt:%d: duplicate id %s" % (ln, cur))
            out[cur] = ([], None)
            continue
        if cur is None:
            if line.strip():
                fail("ko.txt:%d: text before first [id]" % ln)
            continue
        paras, choice = out[cur]
        if line.startswith("? "):
            opts = tuple(o.strip() for o in line[2:].split("|"))
            if len(opts) != 2 or any(not o for o in opts):
                fail("ko.txt:%d: a choice needs two options" % ln)
            out[cur] = (paras, opts)
            continue
        if not line.strip():
            if paras and paras[-1]:
                paras.append("")
            continue
        if not paras:
            paras.append("")
        paras[-1] = (paras[-1] + " " + line.strip()).strip()
    return out


def parse_ui(path, fail):
    out = {}
    if not os.path.exists(path):
        return out
    for ln, line in enumerate(open(path, encoding="utf-8"), 1):
        line = line.rstrip("\n")
        if not line.strip() or line.startswith("#"):
            continue
        key, text = [p.strip() for p in line.split("=", 1)]
        out[key] = (text, ln)
    return out


def c_bytes(data, per_line=16):
    return "\n".join("    " + ",".join("0x%02X" % b for b in data[i:i + per_line]) + ","
                     for i in range(0, len(data), per_line))


def build(entries, en_choices, ui, ui_offsets, encode_char, fail):
    """entries: [(id, [paragraphs])] in DLG_* order; en_choices: {id: pair};
    ui: [(key, en_bytes)]; ui_offsets: {key: offset in ui_text}.
    Returns (outputs {path: text}, report string)."""
    g11 = load_bdf(os.path.join(FONT_DIR, "Galmuri11.bdf"))
    ko = parse_dialogue(os.path.join(ROOT, "assets", "dialogue", "ko.txt"), fail)
    ko_ui = parse_ui(os.path.join(ROOT, "assets", "text", "ui_ko.txt"), fail)
    ids = [e[0] for e in entries]
    for k in ko:
        if k not in ids:
            fail("ko.txt: [%s] is not an id of en.txt" % k)
    for k in ko_ui:
        if k not in ui_offsets:
            fail("ui_ko.txt: %s is not a key of ui.txt" % k)

    # ---- dialogue: Galmuri11 glyphs, pixel-width word wrap
    glyph_index, glyph_list = {}, []

    def gid(ch):
        if ord(ch) not in g11:
            fail("Galmuri11 has no glyph for %r" % ch)
            ch = "?"
        if ch not in glyph_index:
            glyph_index[ch] = len(glyph_list)
            glyph_list.append(ch)
        return glyph_index[ch]

    def width(s):
        return sum(g11[ord(c)][0] if ord(c) in g11 else 0 for c in s)

    def wrap(par, where):
        lines, cur = [], ""
        for word in par.split():
            while width(word) > LINE_PX:          # very long word: cut it
                cut = len(word)
                while width(word[:cut]) > LINE_PX:
                    cut -= 1
                if cur:
                    lines.append(cur)
                    cur = ""
                lines.append(word[:cut])
                word = word[cut:]
            if not cur:
                cur = word
            elif width(cur + " " + word) <= LINE_PX:
                cur += " " + word
            else:
                lines.append(cur)
                cur = word
        if cur:
            lines.append(cur)
        return lines

    pages, first, count, missing = [], [], [], []
    for did, en_paras in entries:
        paras = ko[did][0] if did in ko else en_paras
        if did not in ko:
            missing.append(did)
        f = len(pages)
        page_lines = []
        for p in [p for p in paras if p]:
            # Pages break between sentences where possible: sentences are
            # packed while they fit in 2 lines; a longer one is spread evenly.
            cur = ""
            for sent in re.split(r"(?<=[.!?…\"])\s+", p):
                if cur and len(wrap(cur + " " + sent, did)) <= 2:
                    cur += " " + sent
                    continue
                if cur:
                    page_lines.append(wrap(cur, did))
                cur = sent
                lines = wrap(cur, did)
                if len(lines) > 2:
                    n_pages = -(-len(lines) // 2)
                    i = 0
                    for k in range(n_pages - 1):
                        take = -(-(len(lines) - i) // (n_pages - k))
                        page_lines.append(lines[i:i + take])
                        i += take
                    cur = " ".join(lines[i:])
            if cur:
                page_lines.append(wrap(cur, did))
        for ls in page_lines:
            ls = ls + [""] * (2 - len(ls))
            rec = [len(ls[0]), len(ls[1])]
            for s in ls:
                for ch in s:
                    g = gid(ch)
                    rec += [g >> 8, g & 0xFF]
            if rec[0] + rec[1] > MAX_PAGE_GLYPHS:
                fail("ko [%s]: page too long" % did)
            pages.append(rec)
        if len(pages) == f:
            fail("ko [%s] has no text" % did)
        if did.startswith("bark_") and len(pages) - f != 1:
            fail("ko.txt [%s]: a bark must fit on one page (2 lines)" % did)
        first.append(f)
        count.append(len(pages) - f)

    # Choices: Korean pair if given, else the English one.
    pairs, ko_choice = [], []
    for did, _ in entries:
        en = en_choices.get(did)
        if en is None:
            ko_choice.append(None)
            continue
        kc = ko[did][1] if did in ko and ko[did][1] else en
        ko_choice.append(kc)

    # ---- UI: a string with Hangul is one Galmuri11 line, 16 pixels high
    # (record: 0x01, glyph count, then 2 bytes per glyph); a string without
    # Hangul stays in the 8x8 English font (its tile bytes, 0-terminated).
    def ui_encode(text, where, max_px=LINE_PX):
        if not any(is_hangul(c) for c in text):
            return [encode_char(ch, where) for ch in text]
        if width(text) > max_px:
            fail("%s: %r is %d px wide (max %d)" % (where, text, width(text), max_px))
        out = [1, len(text)]
        for ch in text:
            g = gid(ch)
            out += [g >> 8, g & 0xFF]
        return out

    ui_ko_data, ui_missing = [], []
    for key, en_bytes in ui:
        if key in ko_ui:
            text, ln = ko_ui[key]
            ui_ko_data.append(ui_encode(text, "ui_ko.txt:%d %s" % (ln, key)))
        else:
            ui_missing.append(key)
            ui_ko_data.append(list(en_bytes))
    choice_ui = []
    for kc in ko_choice:
        choice_ui.append(None if kc is None else (ui_encode(kc[0], "choice", 56), ui_encode(kc[1], "choice", 56)))
    lang_menu = ui_encode("한국어", "language menu")

    # ---- blobs: glyphs then pages, packed into banks KO_BLOB_BANK..
    blobs = [[]]          # one list of bytes per bank

    def put(data):
        if len(blobs[-1]) + len(data) > 16384:
            blobs.append([])
        pos = (len(blobs) - 1, len(blobs[-1]))
        blobs[-1].extend(data)
        return pos

    glyph_pos = []
    for ch in glyph_list:
        g = g11[ord(ch)]
        rows = glyph_rows(g, GLYPH_ROWS, BASE_ROW, 16)
        rec = [g[0]]
        for r in rows:
            rec += [r >> 8, r & 0xFF]
        glyph_pos.append(put(rec))
    page_pos = [put(p) for p in pages]
    if KO_BLOB_BANK + len(blobs) - 1 > KO_LAST_BANK:
        fail("Korean data needs %d banks from %d: over bank %d"
             % (len(blobs), KO_BLOB_BANK, KO_LAST_BANK))

    # ---- C output
    H = ["/* Generated by tools/ko_gen.py - do not edit by hand. */",
         "#ifndef KO_DATA_H\n#define KO_DATA_H\n#include <stdint.h>\n",
         "#define KO_BANK %d" % KO_BANK,
         "#define KO_GLYPH_SIZE %d" % GLYPH_SIZE,
         "#define KO_GLYPH_ROWS %d" % GLYPH_ROWS,
         "#define KO_MAX_PAGE %d   /* bytes of the largest page record */" % (2 + 2 * MAX_PAGE_GLYPHS),
         "#define KO_UI_COUNT %d" % len(ui),
         "#define KO_BLOB_BANK %d" % KO_BLOB_BANK,
         "extern const uint16_t ko_glyph_end[];      /* per blob bank: first glyph past it */",
         "extern const uint8_t * const ko_glyph_ptr[];",
         "extern const uint16_t ko_page_end[];       /* per blob bank: first page past it */",
         "extern const uint8_t * const ko_page_ptr[];",
         "extern const uint16_t ko_first_page[];     /* per DLG_* id */",
         "extern const uint8_t ko_page_count[];",
         "extern const uint8_t * const ko_choice_ptr[];  /* 2 per DLG_* id, 0 = none */",
         "extern const uint16_t ko_ui_off[];         /* English offsets in ui_text, ascending */",
         "extern const uint8_t * const ko_ui_ptr[];",
         "extern const uint8_t ko_lang_name[];       /* the word for Korean */",
         "#endif"]

    C = ["/* Generated by tools/ko_gen.py - do not edit by hand. */",
         "#pragma bank %d" % KO_BANK, "#include <stdint.h>", '#include "ko_data.h"', ""]
    for b in range(len(blobs)):
        C.append("extern const uint8_t ko_blob_%d[];" % (KO_BLOB_BANK + b))
    C.append("")
    # Glyph and page pointers are split as bank + address (3 bytes an item).
    # Items are laid out bank after bank, so a bank is found from where each
    # bank's items end (saves a byte per item in the full bank 6).
    def ends(pos):
        banks = [b for b, _ in pos]
        assert banks == sorted(banks), "ko_gen: items out of bank order"
        return ", ".join(str(sum(1 for x in banks if x <= b)) for b in range(len(blobs)))
    C.append("const uint16_t ko_glyph_end[] = { %s };" % ends(glyph_pos))
    C.append("const uint8_t * const ko_glyph_ptr[] = {\n%s\n};" % "\n".join(
        "    ko_blob_%d + %d," % (KO_BLOB_BANK + b, o) for b, o in glyph_pos))
    C.append("const uint16_t ko_page_end[] = { %s };" % ends(page_pos))
    C.append("const uint8_t * const ko_page_ptr[] = {\n%s\n};" % "\n".join(
        "    ko_blob_%d + %d," % (KO_BLOB_BANK + b, o) for b, o in page_pos))
    C.append("const uint16_t ko_first_page[] = { %s };" % ", ".join(map(str, first)))
    C.append("const uint8_t ko_page_count[] = { %s };" % ", ".join(map(str, count)))
    # identical strings (choices like "Yes" / "No") are stored once
    pooled = {}

    def pool(data, name):
        key = tuple(data)
        if key not in pooled:
            pooled[key] = name
            C.append("static const uint8_t %s[] = { %s0 };" % (name, "".join("%d," % b for b in data)))
        return pooled[key]
    ch_names = []
    for i, cu in enumerate(choice_ui):
        ch_names.append(None if cu is None else
                        (pool(cu[0], "ko_ch_%d_a" % i), pool(cu[1], "ko_ch_%d_b" % i)))
    C.append("const uint8_t * const ko_choice_ptr[] = {\n%s\n};" % "\n".join(
        ("    0, 0," if n is None else "    %s, %s," % n) for n in ch_names))
    order = sorted(range(len(ui)), key=lambda i: ui_offsets[ui[i][0]])
    C.append("const uint16_t ko_ui_off[] = { %s };" % ", ".join(str(ui_offsets[ui[i][0]]) for i in order))
    ui_names = [pool(ui_ko_data[i], "ko_ui_%d" % i) for i in order]
    C.append("const uint8_t * const ko_ui_ptr[] = { %s };" % ", ".join(ui_names))
    C.append("const uint8_t ko_lang_name[] = { %s, 0 };" % ",".join(map(str, lang_menu)))

    outputs = {
        os.path.join(ROOT, "include", "ko_data.h"): "\n".join(H) + "\n",
        os.path.join(ROOT, "src", "ko_data.c"): "\n".join(C) + "\n",
    }
    for b, data in enumerate(blobs):
        n = KO_BLOB_BANK + b
        outputs[os.path.join(ROOT, "src", "ko_blob_%d.c" % n)] = "\n".join([
            "/* Generated by tools/ko_gen.py - do not edit by hand. */",
            "#pragma bank %d" % n, "#include <stdint.h>", "",
            "const uint8_t ko_blob_%d[] = {\n%s\n};" % (n, c_bytes(data or [0])), ""]) + "\n"
    report = ("ko_gen: %d glyphs, %d pages in %d banks; untranslated: %d dialogue, %d ui"
              % (len(glyph_list), len(pages), len(blobs), len(missing), len(ui_missing)))
    return outputs, report
