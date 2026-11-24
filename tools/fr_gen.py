"""French option: dialogue and UI strings in the 8x8 font (called by gen_assets.py).

assets/dialogue/fr.txt and assets/text/ui_fr.txt use the same ids and keys as
the English files; a missing one falls back to the English text. Output:
  src/fr_data.c    bank FR_BANK: page tables, choices, UI strings
  src/fr_pages_N.c banks FR_PAGE_BANK + N: dialogue pages
The game reads all of it with far_read() (src/dialogue.c).
"""
import os
import re


def french_spaces(text):
    """French typography: a no-break space before ! ? : ; » and after «, so
    a line never starts with the mark (the font draws it as a space)."""
    text = re.sub(r" +([!?:;»])", "\u00a0\\1", text)
    return re.sub(r"« +", "«\u00a0", text)

FR_BANK = 11
FR_PAGE_BANK = 12
LAST_BANK = 15


# Room left on screen where a key is followed by something else on its row
# (src/notebook.c, main.c, games.c): its maximum length.
ROOM = {"nb_title": 13, "nb_quests": 12, "nb_next": 12, "nb_prev": 12, "nb_album": 12,
        "nb_myst": 12, "nb_close": 5, "x_title": 13, "secrets": 13, "al_title": 14,
        "cu_title": 14, "end_1": 13, "end_2": 12, "end_x": 12, "rank": 5, "cu_solved": 14,
        "secret_done": 13, "extra_done": 13, "lang_title": 13, "save_cont": 12, "save_new": 12,
        "lang_go": 13, "srv_title": 14, "chase_hud": 14}
ROOM.update({"q_%d" % i: 15 for i in range(5)})
ROOM.update({"rank_%d" % i: 12 for i in range(5)})
ROOM.update({k: 14 for k in ("x_0", "x_0h", "x_1", "x_1h", "x_2", "x_2h")})
INTRO_ROOM = 17          # intro pages are printed from column 2


def c_bytes(data, per_line=16):
    return "\n".join("    " + ",".join("0x%02X" % b for b in data[i:i + per_line]) + ","
                     for i in range(0, len(data), per_line))


def build(root, en_entries, en_choices, choice_pairs, ui, ui_offsets, page_size,
          parse_dialogue, paginate, encode_text, text_limit, fail):
    outputs, header = {}, []
    report = {"missing_dlg": [], "missing_ui": [], "longer_ui": []}

    # Dialogue
    fr_entries, fr_choices = parse_dialogue(os.path.join(root, "assets", "dialogue", "fr.txt"), "fr.txt")
    fr_by_id = {d: p for d, p, _ in fr_entries}
    en_ids = [d for d, _, _ in en_entries]
    for d, _, ln in fr_entries:
        if d not in en_ids:
            fail("fr.txt:%d: [%s] is not in en.txt" % (ln, d))
    pages, first, count = [], [], []
    for did, paras, _ in en_entries:
        src = fr_by_id.get(did)
        if src is None:
            report["missing_dlg"].append(did)
            p = paginate(did, paras, "en.txt")
        else:
            p = paginate(did, [french_spaces(x) for x in src], "fr.txt")
            if (fr_choices.get(did) is None) != (en_choices.get(did) is None):
                fail("fr.txt:[%s]: the choice must match en.txt (present or not)" % did)
        if len(p) > 255:
            fail("fr.txt:[%s]: too many pages" % did)
        first.append(len(pages))
        count.append(len(p))
        pages += p

    # Choices: one French pair per English pair, the same everywhere.
    fr_pair = {}
    for did, _, _ in en_entries:
        en = en_choices.get(did)
        if en is None:
            continue
        if did not in fr_by_id:
            continue                    # untranslated: the pair comes from another entry
        fr = fr_choices.get(did)
        if en in fr_pair and fr_pair[en] != fr:
            fail("fr.txt:[%s]: '%s | %s' is translated two ways (%s / %s)"
                 % (did, en[0], en[1], "|".join(fr_pair[en]), "|".join(fr)))
        fr_pair[en] = fr
    choice_text = []
    for pair in choice_pairs:
        a, b = fr_pair.get(pair, pair)
        choice_text += encode_text(a.ljust(7), "fr choice") + encode_text(b.ljust(7), "fr choice")

    # UI strings: fr_ui_at[English offset] = offset in fr_ui_text.
    fr_ui = {}
    path = os.path.join(root, "assets", "text", "ui_fr.txt")
    for ln, line in enumerate(open(path, encoding="utf-8"), 1):
        line = line.rstrip("\n")
        if not line.strip() or line.startswith("#"):
            continue
        key, text = [p.strip() for p in line.split("=", 1)]
        if key not in ui_offsets:
            fail("ui_fr.txt:%d: unknown key %r" % (ln, key))
            continue
        room = ROOM.get(key, INTRO_ROOM if key.startswith("intro_") else text_limit(key))
        if len(text) > room:
            fail("ui_fr.txt:%d: %r is longer than %d characters (room on screen)" % (ln, key, room))
        if key == "catch_hud" and (len(text) != 18 or text[4] != " " or text[15] != " "
                                   or text[5] != "/" or text[16] != "/"):
            fail("ui_fr.txt:%d: catch_hud must keep the digit slots of 'Got  /6   Miss  /3'" % ln)
        fr_ui[key] = encode_text(french_spaces(text), "ui_fr.txt:%d" % ln)
    en_len = sum(len(d) + 1 for _, d in ui)
    at = [0] * en_len
    ui_text = []
    for key, data in ui:
        if key not in fr_ui:
            report["missing_ui"].append(key)
        elif len(fr_ui[key]) > len(data):
            report["longer_ui"].append(key)
        at[ui_offsets[key]] = len(ui_text)
        ui_text += fr_ui.get(key, data) + [0]

    # Pages over banks FR_PAGE_BANK..: as many as fit in 16 KiB each.
    per_bank = 16384 // page_size
    n_banks = -(-len(pages) // per_bank)
    if FR_PAGE_BANK + n_banks - 1 > LAST_BANK:
        fail("French pages need %d banks: more than the ROM has" % n_banks)
    header += [
        "/* French (src/fr_data.c, bank FR_BANK; pages in FR_PAGE_BANK..) */",
        "#define FR_BANK %d" % FR_BANK,
        "#define FR_PAGE_BANK %d" % FR_PAGE_BANK,
        "#define FR_PAGES_PER_BANK %d" % per_bank,
        "extern const uint16_t fr_first_page[DLG_COUNT];",
        "extern const uint8_t fr_page_count[DLG_COUNT];",
        "extern const uint8_t fr_choice_text[];",
        "extern const uint16_t fr_ui_at[];        /* by offset in ui_text */",
        "extern const uint8_t fr_ui_text[];",
        "extern const uint8_t * const fr_page_base[];",
    ]
    for k in range(n_banks):
        header.append("extern const uint8_t fr_pages_%d[];" % k)
    data = ["/* Generated by tools/fr_gen.py from assets/dialogue/fr.txt and assets/text/ui_fr.txt. */",
            "#pragma bank %d" % FR_BANK, '#include "assets.h"', "",
            "const uint16_t fr_first_page[DLG_COUNT] = { %s };" % ", ".join(map(str, first)),
            "const uint8_t fr_page_count[DLG_COUNT] = { %s };" % ", ".join(map(str, count)),
            "const uint8_t fr_choice_text[] = {\n%s\n};" % c_bytes(choice_text),
            "const uint16_t fr_ui_at[] = { %s };" % ", ".join(map(str, at)),
            "const uint8_t fr_ui_text[] = {\n%s\n};" % c_bytes(ui_text),
            "const uint8_t * const fr_page_base[] = { %s };"
            % ", ".join("fr_pages_%d" % k for k in range(n_banks)), ""]
    outputs[os.path.join(root, "src", "fr_data.c")] = "\n".join(data) + "\n"
    for k in range(n_banks):
        flat = [b for pg in pages[k * per_bank:(k + 1) * per_bank] for b in pg]
        outputs[os.path.join(root, "src", "fr_pages_%d.c" % k)] = "\n".join([
            "/* Generated by tools/fr_gen.py - do not edit by hand. */",
            "#pragma bank %d" % (FR_PAGE_BANK + k), '#include "assets.h"', "",
            "const uint8_t fr_pages_%d[] = {\n%s\n};" % (k, c_bytes(flat, page_size // 2)), ""]) + "\n"
    report["pages"] = len(pages)
    report["banks"] = n_banks
    return outputs, header, report
