#!/usr/bin/env python3
"""New places (2026-11-24): the covered street west of rue Baltard to the
Bourse de commerce, the all-night brasserie and its telephone, the hotel.
Usage: python3 tests/places_test.py ROM [SCREENSHOT_DIR]
"""
import io
import os
import re
import sys

from pyboy import PyBoy

ROM = sys.argv[1] if len(sys.argv) > 1 else "build/paris-disparu.gbc"
SHOTS = sys.argv[2] if len(sys.argv) > 2 else None
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools"))
import font  # noqa: E402

FONT_BASE = 128
CHARS = list(font.GLYPHS.keys())
(ROOM_STREET, ROOM_ENTRANCE, ROOM_MARKET, ROOM_CAFE, ROOM_QUAI, ROOM_PONT, ROOM_METRO,
 ROOM_BOURSE, ROOM_HOTEL, ROOM_BRASSERIE) = range(10)
FLAG_TASK_DONE, FLAG_PHONE_DONE, FLAG_SOLVED = 0x0008, 0x0200, 0x1000
failures = []


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        failures.append(msg)


def symbols(rom):
    out = {}
    for line in open(os.path.splitext(rom)[0] + ".noi"):
        m = re.match(r"DEF (\S+) 0x([0-9A-Fa-f]+)", line)
        if m:
            out[m.group(1)] = int(m.group(2), 16)
    return out


sym = symbols(ROM)
HDR = {}
for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "include", "assets.h")):
    m = re.match(r"#define (\w+) (\d+)$", line.strip())
    if m:
        HDR[m.group(1)] = int(m.group(2))
sym_spr1_sparkle = int(re.search(r"#define SPR1_SPARKLE (\d+)", open(os.path.join(
    os.path.dirname(os.path.abspath(__file__)), "..", "include", "assets.h")).read()).group(1))
pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=False,
           ram_file=io.BytesIO(bytes(8192)))   # blank battery RAM: no save
mem = pb.memory


def s16(addr):
    v = mem[addr] | (mem[addr + 1] << 8)
    return v - 0x10000 if v & 0x8000 else v


def player():
    a = sym["_player"]
    return s16(a), s16(a + 2)


def room():
    return mem[sym["_cur_room"]]


def clues():
    return mem[sym["_clue_flags"]]


def flags():
    a = sym["_game_flags"]
    return mem[a] | (mem[a + 1] << 8)


def tick(n=1):
    pb.tick(n, True)


def hold(button, frames):
    pb.button_press(button)
    tick(frames)
    pb.button_release(button)
    tick(1)


def tap(button, wait=8):
    pb.button_press(button)
    tick(2)
    pb.button_release(button)
    tick(wait)


def window_on():
    return bool(mem[0xFF40] & 0x20)


def window_row(row):
    base = 0x9C00 if mem[0xFF40] & 0x40 else 0x9800
    text = ""
    for x in range(20):
        i = mem[base + row * 32 + x] - FONT_BASE
        text += CHARS[i] if 0 <= i < len(CHARS) else "|"
    return text.strip("|").strip()




def shot(name):
    if SHOTS:
        os.makedirs(SHOTS, exist_ok=True)
        pb.screen.image.resize((480, 432)).save(os.path.join(SHOTS, name + ".png"))


def walk(axis, target, limit=600):
    """Walk along one axis until the coordinate reaches target or the player stops."""
    idx = 0 if axis == "x" else 1
    start_room = room()
    stuck = 0
    for _ in range(limit):
        pos = player()[idx]
        if pos == target or room() != start_room:
            break
        if axis == "x":
            button = "right" if target > pos else "left"
        else:
            button = "down" if target > pos else "up"
        pb.button_press(button)
        tick(1)
        pb.button_release(button)
        tick(1)
        stuck = stuck + 1 if player()[idx] == pos else 0
        if stuck > 6:
            break
    tick(1)
    return player()[idx]


def goto(x, y, order="yx"):
    for axis in order:
        walk(axis, x if axis == "x" else y)
    return player()


def push(button, frames=90):
    """Walk until blocked or until a door/edge takes the player to another room."""
    start = room()
    pb.button_press(button)
    last, still = None, 0
    for _ in range(frames):
        tick(1)
        if room() != start:
            break
        still = still + 1 if player() == last else 0
        last = player()
        if still > 4:
            break
    pb.button_release(button)
    tick(30)   # room fade / settle


def dialogue(choice=None, max_pages=24):
    """Read a whole dialogue: returns the first line of each page seen."""
    lines = []
    tick(70)
    for _ in range(max_pages):
        if window_on() and window_row(1) == "HOW TO PLAY":   # rules card: start the game
            lines.append("<howto %s>" % window_row(3))
            shot("27_howto_" + window_row(3).split()[-1].lower())
            tap("a", 6)
            continue
        if not window_on() or window_row(1).startswith(("Up/Down", "Lift!", "Punch!", "New clue", "Extra done", "Secret found", "QUEST COMPLETE",
                                                        "Pour coffee", "Watch the", "Which cup", "Got ", "Catch him",
                                                        "Curiosity", "Mystery solved", "You got")) \
                or window_row(2) == "THE END":
            break
        if "▶" in window_row(2):
            if choice == 1:
                tap("right", 4)
            tap("a", 30)
            lines.append("<choice %d>" % (choice or 0))
            tick(60)
            continue
        lines.append(page_text())
        for key, name in NEWS_SHOTS:          # 1967 news lines (screenshots for docs)
            if key in lines[-1]:
                shot(name)
        tap("a", 70)
    return lines


def page_text():
    """The text box's rows 1-3 as one line (dialogue uses rows 1 and 3)."""
    return " ".join(r for r in (window_row(1), window_row(2), window_row(3)) if r)


def win_tile(x, y):
    base = 0x9C00 if mem[0xFF40] & 0x40 else 0x9800
    return mem[base + y * 32 + x]


def skip_howto():
    """The first time a mini-game starts, a rules card comes first: A starts it."""
    tick(4)
    if window_on() and window_row(1) == "HOW TO PLAY":
        shot("27_howto_" + window_row(3).split()[-1].lower())
        tap("a", 4)


def lift(hit=True):
    """Porter's lift: press A when the cursor is inside the zone (or at once to miss)."""
    tap("a", 4)
    skip_howto()
    if not hit:
        tap("a", 2)
        return
    for _ in range(400):
        cols = [x for x in range(2, 18) if win_tile(x, 3) == HDR["GLYPH_BAR_CURSOR"]]
        if cols and 9 <= cols[0] <= 10:
            pb.button_press("a")
            tick(1)
            pb.button_release("a")
            tick(2)
            return
        tick(1)


EXCH = ["ALÉ", "ARC", "BRE", "CAR", "MED", "ODÉ", "OPÉ", "PAS", "ROQ"]   # ex_0.. in ui.txt


def dial_shown():
    """What the dial shows, e.g. 'ARC4127'."""
    return "".join(c for c in window_row(3) if c.isdigit() or c.isalpha())


def dial(number, notes=False):
    """Phone: number = 'ARC 4127'. Set the exchange (Up), then the 4 digits
    (Right, Up), or START: notebook numbers; then A."""
    want = number.replace(" ", "")
    if notes:
        for _ in range(4):
            tap("start", 4)
            if dial_shown() == want:
                break
        check(dial_shown() == want, "START picks %s from the notebook (%s)" % (number, dial_shown()))
        tap("a", 0)
        return
    ex, digits = number.split()
    for _ in range(EXCH.index(ex)):
        tap("up", 2)
    for d in digits:
        tap("right", 2)
        for _ in range(int(d)):
            tap("up", 2)
    check(dial_shown() == want, "dialled %s (%s)" % (number, dial_shown()))
    shot("09_dial")
    tap("a", 0)


def talk(expect, choice=None, clue=None, got=None):
    """Talk/examine; clue = the clue bit this should add (its note is checked and closed);
    got = (name, ITEM_*) of an object received ("You got" caption)."""
    before = clues()
    tap("a", 0)
    lines = dialogue(choice)
    if clue is not None:
        note = window_on() and ("New clue" in window_row(1) or "QUEST COMPLETE" in window_row(1))
        check(note and clues() == before | clue, "new clue 0x%02x noted (clues 0x%02x)" % (clue, clues()))
        if clue == 0x08:
            check(item_shown(HDR["ITEM_BLEU"]), "quest note shows the petit bleu")
            shot("40_quest_item_bleu")
        tap("b", 4)
    if got:
        got_note(*got)
    ok = any(expect in l for l in lines)
    check(ok and not window_on(), "dialogue %r (%d pages, first: %r)" % (expect, len(lines), lines if not ok else lines[0]))
    return lines


def item_shown(item):
    """The item picture: OAM 24 uses ITEM_TILE, with that item's tiles in VRAM bank 1."""
    o = 0xFE00 + 24 * 4
    if not (mem[o] and mem[o + 2] == HDR["ITEM_TILE"]):
        return False
    want = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "src", "portrait_art.c")).read()
    data = re.search(r"item_tiles\[\] = \{(.*?)\};", want, re.S).group(1)
    data = [int(v, 0) for v in data.replace("\n", "").split(",") if v.strip()]
    start = (item - 1) * 192
    vram = [mem[1, 0x8000 + HDR["ITEM_TILE"] * 16 + i] for i in range(192)]
    return vram == data[start:start + 192]


def got_note(name, item):
    """A "You got" caption with the object's picture; closed with B."""
    tick(4)
    ok = window_on() and window_row(1).startswith("You got") and name in window_row(2) and item_shown(HDR[item])
    check(ok, "you got %r, picture %s: %r %r" % (name, item, window_row(1), window_row(2)))
    shot("40_got_" + item.lower())
    tap("b", 4)
    check(mem[0xFE00 + 24 * 4] == 0, "the picture goes with the caption")


def quest_note(name, clue, item=None):
    check(window_on() and "QUEST COMPLETE" in window_row(1) and name in window_row(2) and clues() & clue,
          "quest complete: %r %r (clues 0x%02x)" % (window_row(1), window_row(2), clues()))
    if item:
        check(item_shown(HDR[item]), "quest note shows the %s picture" % item)
        shot("40_quest_" + item.lower())
    tap("b", 4)


def play_serve(win=True):
    """Pour coffee: hold A until the coffee reaches the dark zone (win) or
    hold it until it spills (lose), for each cup."""
    for _ in range(40):
        if not (window_on() and window_row(1).startswith("Pour coffee")):
            return
        if "Let go" not in window_row(4):
            tick(1)
            continue
        pb.button_press("a")
        for _ in range(200):
            tick(1)
            full = sum(1 for x in range(2, 18) if win_tile(x, 3) in (HDR["GLYPH_COFFEE"], HDR["GLYPH_COFFEE_ZONE"]))
            if (win and full == 12) or "Oops" in window_row(4):
                break
        pb.button_release("a")
        tick(60)


def play_cups(win=True):
    """Cup game: wait for the shuffle to end, then pick (from RAM) the right cup."""
    for _ in range(900):
        if window_row(1).startswith("Which cup"):
            break
        tick(1)
    shot("24_cups")
    ans = mem[sym["_cup_where"]]
    xs = (3, 8, 13)
    cur = [i for i, x in enumerate(xs) if win_tile(x, 5) == HDR["GLYPH_ARROW_RIGHT"]][0]
    target = ans if win else (ans + 1) % 3
    while cur != target:
        tap("right", 2)
        cur = (cur + 1) % 3
    tap("a", 2)


def play_catch():
    """Cabbages: keep the player under the falling one."""
    for f in range(6000):
        if not (window_on() and window_row(1).startswith("Got ")):
            return
        if f == 150:
            shot("22_catch")
        cx, px = s16(sym["_cab_x"]), player()[0]
        btn = "right" if cx > px + 1 else "left" if cx < px - 1 else None
        if btn:
            pb.button_press(btn)
            tick(1)
            pb.button_release(btn)
        else:
            tick(1)


def play_chase():
    """The thief on the quai: walk straight at him (the larger gap first)."""
    stuck = 0
    for f in range(30 * 60 + 60):
        if not (window_on() and window_row(1).startswith("Catch him")):
            return f
        if f == 120:
            shot("28_chase")
        tx, ty = s16(sym["_thief_x"]), s16(sym["_thief_y"])
        px, py = player()
        if os.environ.get("CHASE_LOG") and f % 8 == 0:
            print("    chase f=%d thief=(%d,%d) player=(%d,%d)" % (f, tx, ty, px, py))
        dx, dy = tx - px, ty - py
        horiz = abs(dx) >= abs(dy)
        if stuck > 3:                   # blocked (a tree, the singer): other axis
            horiz = not horiz
            if (dy if not horiz else dx) == 0:
                dy = 1 if py < 76 else -1
        if horiz:
            btn = "right" if dx > 0 else "left"
        else:
            btn = "down" if dy > 0 else "up"
        pb.button_press(btn)
        tick(1)
        pb.button_release(btn)
        stuck = stuck + 1 if player() == (px, py) else 0
        if stuck > 30:
            stuck = 0
    return -1


def get_jeton():
    """Win a jeton from the porter at the bar."""
    goto(88, 60, "yx")
    push("up")
    tap("a", 0)
    lines = dialogue(choice=0)
    check(any("cups" in l for l in lines), "the porter offers his cup game: %s" % lines[:1])
    play_cups()
    lines = dialogue()
    check(any("jeton" in l for l in lines) and flags() & 0x0100, "won a jeton at the cups: %s" % lines[:1])
    got_note("jeton", "ITEM_JETON")


# ----------------------------------------------------------------------------
DEBUG = 0
def player_dir():
    return mem[sym["_player"] + 4]


def secret(expect, n, presses=1):
    """Examine what the player faces (presses times); the last look is secret n/9."""
    for _ in range(presses):
        tap("a", 0)
        lines = dialogue()
        if DEBUG: print("   press", player(), player_dir(), lines[:1], window_on() and window_row(1))
    check(any(expect in l for l in lines) and window_on() and "Secret found" in window_row(1)
          and "%d/9" % n in window_row(1), "secret %d: %r -> %s %r" % (n, expect, lines[:1], window_row(1)))
    tap("b", 4)


def cameo(expect, name):
    """First talk with a cameo: screenshot of the opening page, then the whole text."""
    tap("a", 70)
    shot("26_cameo_" + name)
    lines = [page_text()] + dialogue()
    check(any(expect in l for l in lines) and not window_on(), "cameo %s: %s" % (name, lines[:1]))


def street_to_baltard():
    goto(88, 120, "xy")
    push("down")
    check(room() == ROOM_ENTRANCE, "south from rue Rambuteau into rue Baltard (room %d, %s)" % (room(), player()))


def baltard_to_street():
    if player()[1] > 88:            # below the photographer and the metro: go round
        goto(60, 80, "yx")
    goto(88, 80, "xy")
    push("up", 200)
    check(room() == ROOM_STREET, "north end of rue Baltard back to rue Rambuteau (room %d)" % room())


def street_to_quai():
    street_to_baltard()
    goto(88, 80, "xy")
    goto(60, 80, "xy")
    push("down", 200)
    check(room() == ROOM_QUAI, "south end of rue Baltard leads to the quai (room %d, %s)" % (room(), player()))


def to_dede():
    goto(88, 70, "xy")
    goto(64, 70, "xy")
    push("up")




def read_text(max_pages=12):
    """A, then the text of each page until the box closes."""
    tap("a", 70)
    pages = []
    for _ in range(max_pages):
        if not window_on():
            break
        pages.append(" ".join(r for r in (window_row(1), window_row(2), window_row(3)) if r))
        tap("a", 70)
    return pages


def face(button):
    tap(button, 2)


tick(150)
tap("start", 20)
tap("a", 20)                       # English
for _ in range(3):
    tap("a", 10)
tap("a", 50)
tap("b", 4)
check(room() == ROOM_STREET, "in rue Rambuteau")

# The hotel door (x10) opens now.
goto(76, 72, "yx")
push("up")
check(room() == ROOM_HOTEL, "the hotel door leads into the lobby (room %d)" % room())
tick(30)
shot("40_hotel")
goto(40, 38, "xy")
face("up")
t = read_text()
check(any("NIGHT PORTER" in p for p in t), "the night porter talks across the desk: %s" % t[:1])
goto(120, 26, "xy")
face("up")
t = read_text()
check(any("creak" in p for p in t), "the stairs: %s" % t[:1])
goto(88, 38, "yx")
goto(88, 22, "xy")
face("up")
t = read_text()
check("put them" in " ".join(t) and "jeton" in " ".join(t), "the hotel telephone at reception: %s" % t[:2])
goto(76, 120, "xy")
push("down")
check(room() == ROOM_STREET, "back out to rue Rambuteau (room %d)" % room())

# West through the covered street of rue Baltard.
goto(88, 120, "xy")
push("down")
check(room() == ROOM_ENTRANCE, "rue Baltard")
goto(40, 50, "yx")
push("left", 200)
check(room() == ROOM_BOURSE, "the covered street leads west to the Bourse de commerce (room %d, %s)" % (room(), player()))
tick(30)
shot("41_bourse")
goto(130, 56, "yx")
goto(24, 56, "xy")
face("up")
t = read_text()
check(any("BOURSE" in p for p in t) and any("1885" in p for p in t), "the Bourse: %s" % t[:2])
goto(84, 56, "xy")
face("up")
t = read_text()
check(any("MEDICI" in p for p in t), "the Medici column: %s" % t[:1])
goto(64, 80, "yx")
goto(64, 92, "xy")
face("down")
t = read_text()
check(any("VESPASIENNE" in p for p in t), "the vespasienne: %s" % t[:1])

# The brasserie and its telephone.
goto(64, 56, "yx")
goto(100, 56, "xy")
push("up")
check(room() == ROOM_BRASSERIE, "into the brasserie (room %d, %s)" % (room(), player()))
tick(30)
shot("42_brasserie")
goto(104, 118, "xy")
goto(104, 38, "xy")
face("up")
t = read_text()
check(any("WAITER" in p for p in t), "the waiter talks across the zinc: %s" % t[:1])
goto(0, 38, "yx")
goto(0, 22, "xy")
face("up")
t = read_text()
check(any("telephone" in p for p in t), "a second public telephone: %s" % t[:1])
goto(76, 38, "xy")
goto(76, 120, "yx")
push("down")
check(room() == ROOM_BOURSE, "back out to the Bourse (room %d)" % room())
goto(140, 64, "xy")
goto(140, 52, "yx")
push("right", 200)
check(room() == ROOM_ENTRANCE, "east through the covered street back to rue Baltard (room %d)" % room())

pb.stop(save=False)
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
