#!/usr/bin/env python3
"""Headless play-through of Episode 1 in the PyBoy emulator (CGB mode).

Drives the joypad and checks RAM / VRAM state: boot, title, movement,
collision, room changes, every NPC, choices, the porter's crates, the cafe
telephone and the pause box.
Usage: python3 tests/play_test.py ROM [SCREENSHOT_DIR]
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
ROOM_STREET, ROOM_ENTRANCE, ROOM_MARKET, ROOM_CAFE, ROOM_QUAI, ROOM_PONT, ROOM_METRO = range(7)
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


NEWS_SHOTS = (("Headline", "29_news_paper"), ("One franc ninety", "29_news_taxi"),
              ("Tutankhamun", "29_news_painter"), ("Tournier", "29_news_roux"))


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


print("boot")
tick(150)   # boot + title fade-in
check(mem[0xFF40] & 0x80, "LCD is on after boot")
check(pb.cartridge_title.strip().startswith("PARISDISPAR"), "cartridge title read by emulator")
shot("01_title")

print("language menu")
tap("start", 20)
check(window_on() and "LANGUAGE" in window_row(5) and "English" in window_row(8), "language menu after the title: %r" % window_row(8))
shot("01_language")
tap("a", 20)                       # English is preselected

print("intro")
check(window_on() and "PARIS, 21 NOV 1967" in window_row(3), "intro page 1, the setting: %r" % window_row(3))
shot("00_intro")
tap("a", 10)
check("TONIGHT" in window_row(3) and "Find Fernand" in window_row(13), "intro page 2, the goal: %r" % window_row(13))
tap("a", 10)
check("CONTROLS" in window_row(3) and "notebook" in window_row(8), "intro page 3, controls: %r" % window_row(8))
shot("00_controls")
tap("a", 50)   # the street fades in

print("street: the goal and the cat")
check(player() == (84, 92) and room() == ROOM_STREET, "spawned in the street at (84,92): %s" % (player(),))
check(window_on() and "Find Fernand" in window_row(1), "one-line goal on screen: %r" % window_row(1))
shot("02_street")
tick(300)
check(window_on(), "goal is still readable after 5 s")
tap("b", 4)
check(not window_on(), "B dismisses it")
FLAG_CAT_GONE = 0x8000
for _ in range(8):              # chase the cat: it waits for you, then darts on
    if flags() & FLAG_CAT_GONE:
        break
    cx, cy = s16(sym["_cat_x"]), s16(sym["_cat_y"])
    goto(cx + 14, max(cy, 74), "yx")
    tick(60)
check(flags() & FLAG_CAT_GONE, "the cat ran into the cafe (flags %04x)" % flags())
check(window_on() and "café" in window_row(2), "the game says where it went: %r" % window_row(2))
shot("02_cat")
tap("b", 4)

goto(106, 90, "yx")
push("up")
check(player() == (106, 62), "next to the man with the notebook: %s" % (player(),))
bark = page_text()
check(window_on() and "Psst" in bark, "he calls out by himself when approached: %r" % bark)
shot("13_bark")
tick(600)
check(window_on() and "Psst" in page_text(),
      "his line stays while the player stands near (10 s)")
hold("down", 60)
check(not window_on(), "walking away closes his line (y=%d)" % player()[1])
goto(106, 90, "yx")
push("up")
check(window_on() and "Psst" in page_text(),
      "coming back, he calls out again")
tap("right", 2)
tap("a", 0)
tick(70)
check(window_row(1).startswith("MAN WITH NOTEBOOK"), "speaker shown: %r" % window_row(1))
shot("03_dialogue")
lines = dialogue()
check(window_on() and "New clue" in window_row(1) and clues() == 0x01, "the missing hat is the first clue (0x%02x)" % clues())
seen = 0
for _ in range(12):                          # the "got it!" star blinks over the player
    tick(1)
    if mem[0xFE00 + 20 * 4] and mem[0xFE00 + 20 * 4 + 2] == sym_spr1_sparkle:
        seen += 1
        if seen == 1:
            shot("34_sparkle")
check(seen > 0, "a star sparkles over the player for the new clue (%d frames)" % seen)
tap("b", 4)
check(not window_on(), "A pages through and closes (%d pages)" % (len(lines) + 1))
tap("a", 50)
check(window_on() and "yellow" in page_text(), "repeat line: %r" % page_text())
tap("b", 4)
check(not window_on(), "B closes the dialogue")
tap("a", 50)
for _ in range(4):
    if not window_on():
        break
    tap("start", 50)
check(not window_on(), "START pages through and closes a dialogue too")
tick(10)
check(not window_on(), "closing with START does not also open the notebook")

tap("start", 10)
check(window_on() and "NOTEBOOK" in window_row(1) and "0/5" in window_row(1), "START opens the notebook: %r" % window_row(1))
check("Quests" in window_row(5) and "Coffee rush" in window_row(6), "the notebook lists the quests: %r" % window_row(6))
check("café" in window_row(3), "goal: ask at the cafe: %r" % window_row(3))
check("ARC 4127" in window_row(13), "the notebooks' number is written down: %r" % window_row(13))
tap("start", 10)
check(not window_on(), "START closes the notebook")

print("secrets in rue Rambuteau")
goto(52, 86, "yx")
tap("up", 2)
secret("coin", 1)
goto(14, 82, "yx")
push("up")
secret("moth", 2, presses=5)
goto(70, 90, "yx")
push("up")
secret("Knock", 3, presses=3)
goto(110, 100, "yx")
push("right")
cameo("Saint-Germain", "chanteuse")
talk("hat first")
goto(60, 108, "yx")
push("left")
cameo("brasserie", "actor")
talk("look where")
# 1967 transport in rue Rambuteau (sourced in HISTORICAL_RESEARCH)
goto(28, 96, "yx")
goto(28, 72, "xy")
push("right")
talk("VéloSolex")
goto(92, 96, "yx")
push("up")
talk("bus stop")
goto(100, 112, "yx")
push("right")
talk("lorry")
shot("30_transport")
goto(32, 96, "yx")
goto(32, 62, "xy")
goto(44, 62, "xy")
push("up", 20)
talk("DÉFENSE D'AFFICHER")
goto(32, 96, "xy")                 # back down past the lamp
tap("start", 10)
tap("right", 10)
check("Secrets found 3/9" in window_row(15), "the notebook counts the secrets: %r" % window_row(15))
tap("start", 10)

print("rue Baltard: Dede and Fernand's crates")
street_to_baltard()
goto(88, 16, "xy")
push("left")
cameo("A city of glass", "filmmaker")
talk("Comedy")
goto(88, 50, "xy")
goto(32, 50, "xy")
tap("left", 2)                 # face the covered street (walking on would lead west)
secret("covered street", 4)
shot("07_entrance")
goto(88, 80, "xy")
goto(32, 80, "xy")
push("down")
check(player()[1] < 104, "the photographer stands in the way (y=%d)" % player()[1])
tap("a", 70)
shot("16_photographer")
lines = [page_text()] + dialogue()
check(any("Rungis" in l for l in lines) and any("Fernand" in l for l in lines),
      "the photographer's cameo mentions Rungis and Fernand (%d pages)" % len(lines))
to_dede()
check(player() == (64, 46), "below Dede: %s" % (player(),))
bark = page_text()
check("Seen Fernand" in bark, "Dede calls out by himself: %r" % bark)
talk("DÉDÉ", choice=0)


def fetch_crate(expect, miss_first=False):
    goto(88, 70, "xy")
    goto(48, 70, "xy")
    push("up")
    if miss_first:
        lift(hit=False)
        lines = dialogue()
        check(any("slips back" in l for l in lines), "missing the lift drops the crate: %s" % lines[:1])
        check(mem[sym["_carrying"]] == 0xFF, "not carrying after a miss")
    lift()
    lines = dialogue()
    check(any(expect in l for l in lines), "lifted the crate of %s: %s" % (expect, lines[:1]))


def to_market():
    goto(110, 46, "xy")
    goto(110, 26, "xy")
    push("right")
    check(room() == ROOM_MARKET, "through the arch into the pavilion (room %d)" % room())


def to_entrance():
    goto(72, 100, "yx")
    push("down")
    check(room() == ROOM_ENTRANCE, "back out to the entrance (room %d)" % room())


def street_to_cafe():
    goto(28, 90, "yx")
    push("up")
    check(room() == ROOM_CAFE and player() == (72, 118), "entered the cafe through its door: room %d %s" % (room(), player()))


def cafe_to_street():
    goto(72, 100, "yx")
    push("down")
    check(room() == ROOM_STREET and player() == (28, 72), "back in the street at the cafe door: %d %s" % (room(), player()))


def phone(number, expect, clue=None, notes=False):
    goto(128, 60, "yx")
    push("up")
    tap("a", 0)
    lines = dialogue(choice=0)  # "You have a jeton..." -> Yes -> (rules, first time) -> dial box
    if number == "ARC 4127":
        check(any("telephone" in l for l in lines), "the first call shows the phone's rules: %s" % lines[-2:])
    check(window_on() and "Up/Down" in window_row(1), "dial box opened: %r" % window_row(1))
    before = clues()
    dial(number, notes)
    lines = dialogue()
    check(any(expect in l for l in lines), "calling %s: %s" % (number, lines))
    if clue is not None:
        check(window_on() and "New clue" in window_row(1) and clues() == before | clue,
              "call %s adds clue 0x%02x (clues 0x%02x)" % (number, clue, clues()))
        tap("b", 4)


def to_bridge():
    street_to_quai()
    goto(72, 60, "yx")
    push("down")
    check(room() == ROOM_PONT, "the bridge deck leads onto the bridge (room %d, %s)" % (room(), player()))


def to_fernand():
    goto(66, 100, "xy")
    push("right")
    check(player()[1] == 100 and 78 <= player()[0] <= 84, "next to Fernand on the bridge: %s" % (player(),))


fetch_crate("cabbages", miss_first=True)
check(mem[sym["_carrying"]] == 0, "carrying the cabbages")
to_market()
shot("08_market")
goto(72, 38, "xy")
push("up")
talk("Wrong stall")
goto(36, 38, "xy")
push("up")
talk("My cabbages")
push("left")
talk("The price")               # the merchant herself
goto(110, 66, "xy")
push("right")
secret("tiny", 5)
goto(124, 96, "yx")
push("right")
talk("Five minutes")       # the market is busier: another fort
shot("35_market_people")
to_entrance()
fetch_crate("apples")
to_market()
goto(72, 38, "xy")
push("up")
talk("You set the crate")
to_entrance()
fetch_crate("carrots")
to_market()
goto(112, 38, "xy")
push("up")
talk("You set the crate")
to_entrance()
to_dede()
talk("proud")
check(flags() & FLAG_TASK_DONE, "porter's task done (FLAG_TASK_DONE)")
to_market()
goto(72, 38, "xy")
goto(36, 38, "xy")
push("up")
push("left")
tap("a", 0)                     # the crates earned Germaine's trust: one more job
lines = dialogue(choice=0)
check(any("cabbages" in l for l in lines) and window_on() and window_row(1).startswith("Got "),
      "Germaine's cabbage game starts: %s" % lines[-1:])
play_catch()
lines = dialogue()
check(any("Rungis" in l for l in lines), "six cabbages caught: Germaine talks about Rungis")
quest_note("crates", 0x20)
to_entrance()

print("the cafe: the coltin, the telephone, the petit bleu")
baltard_to_street()
street_to_cafe()
shot("05_cafe")
goto(14, 112, "yx")
tap("left", 2)
secret("cat from", 6)
goto(60, 112, "yx")
goto(60, 64, "xy")
push("left")
cameo("my companion", "philosopher")
talk("reasons")
goto(60, 80, "xy")
push("left")
cameo("steak and", "critic")
talk("gratinée")
goto(72, 100, "xy")
push("up")
check(player()[1] == 38, "stopped by the counter (y=%d)" % player()[1])
tap("a", 0)
lines = dialogue(choice=0)
check(any("Give me a" in l for l in lines) and window_row(1).startswith("Pour coffee"), "Simone asks for help: pour the coffee")
shot("23_serve")
tick(30)
check("Oops" not in window_row(4) and "Let go" in window_row(4),
      "the A that starts the coffee game does not pour a cup: %r" % window_row(4))
pb.button_press("a")                # held from a released state: the coffee rises
tick(40)
poured = sum(1 for x in range(2, 18) if win_tile(x, 3) in (HDR["GLYPH_COFFEE"], HDR["GLYPH_COFFEE_ZONE"]))
check(poured >= 4, "holding A pours: %d cells after 40 frames" % poured)
pb.button_release("a")
tick(60)
check(item_shown(HDR["ITEM_CUP"]), "the coffee game shows a café cup")
play_serve(win=False)
lines = dialogue(choice=0)
check(any("coffee everywhere" in l for l in lines), "a bad rush: Simone asks again")
play_serve()
lines = dialogue()
check(any("Fernand's" in l for l in lines), "a good rush: Simone gives the coltin")
quest_note("Coffee rush", 0x02, "ITEM_COLTIN")
goto(128, 60, "yx")
push("up")
talk("telephone")
goto(72, 60, "yx")
push("up")
talk("porter at the bar")       # Simone is out of jetons
get_jeton()
phone("ARC 4127", "courier", clue=0x04)
check(flags() & FLAG_PHONE_DONE, "telephone call made (FLAG_PHONE_DONE)")
shot("06_phone")
goto(72, 60, "yx")
push("up")
talk("Marcel", clue=0x08)
cafe_to_street()

print("the Seine")
street_to_quai()
goto(4, 60, "yx")
push("down")
secret("pigeon", 7)
shot("10_quai")
goto(100, 60, "yx")
push("right")
check(window_on() and "SINGER" in window_row(1), "the singer hums by himself: %r" % window_row(1))
tap("a", 0)
lines = dialogue()
check(any("minor key" in l for l in lines) and any("punches holes" in l for l in lines) and not window_on(),
      "the singer's cameo: %d pages" % len(lines))
shot("21_singer")
talk("books along")              # his hint: the first missing clue (the plate)
goto(100, 44, "yx")
goto(118, 44, "xy")
push("right")
cameo("a sigh", "novelist")
talk("leaves something")
goto(40, 64, "yx")
push("up")
talk("something hot")   # Mme Roux has his plate: the trade starts
check(mem[sym["_quest_flags"]] & 1, "Mme Roux's request noted")
goto(24, 64, "yx")
push("down")
talk("7302")
goto(72, 60, "yx")
push("down")
check(room() == ROOM_PONT, "the bridge deck leads onto the bridge (room %d, %s)" % (room(), player()))
x0 = mem[0xFE00 + 36 * 4 + 1]
tick(60)
x1 = mem[0xFE00 + 36 * 4 + 1]
check(x0 != x1, "the barge moves (%d -> %d)" % (x0, x1))
goto(64, 16, "xy")
push("left")
secret("cornice", 8)
goto(72, 16, "xy")
push("down")
check(player()[1] < 72, "the painter blocks the middle of the deck (y=%d)" % player()[1])
talk("big fellow")
shot("11_pont")
goto(66, 56, "xy")
push("left")
cameo("Irishman", "playwright")
talk("big man down")
to_fernand()
talk("brass plate")             # not enough yet: he says what is missing
goto(66, 110, "xy")
push("down")
check(player()[1] < 118, "roadworks barrier stops the player (y=%d)" % player()[1])
talk("ROADWORKS")
push("up", 400)
check(room() == ROOM_QUAI, "back up to the quai (room %d)" % room())
goto(60, 40, "xy")
push("up", 200)
check(room() == ROOM_ENTRANCE, "quai leads back up to rue Baltard (room %d)" % room())

print("metro: the ticket puncher (Les Halles station)")
goto(60, 100, "yx")
push("right")
check(room() == ROOM_METRO and player() == (72, 108), "down the metro steps into the ticket hall: room %d %s" % (room(), player()))
shot("17_metro")
goto(84, 72, "xy")
push("up")
check(window_on() and "Tickets" in window_row(1) + window_row(3), "the puncher calls out: %r" % window_row(3))
talk("No ticket")
goto(28, 72, "yx")
push("up")
talk("second-class ticket", got=("métro ticket", "ITEM_TICKET"))
check(flags() & 0x0400, "bought a ticket (FLAG_TICKET)")
talk("POUND DEVALUED")
goto(60, 72, "yx")
push("up")
shot("32_metro_map")
talk("LINE 4")      # line map: the other stops open in later episodes
goto(84, 72, "yx")
push("up")
tap("a", 0)
lines = dialogue()
check(any("Clac" in l for l in lines) and window_on(), "he punches the ticket and offers a go: %s" % lines[:1])
def punch(hit=True):
    """The punch bar is already up: press A in the zone, or far from it to miss."""
    for _ in range(400):
        cols = [x for x in range(2, 18) if win_tile(x, 3) == HDR["GLYPH_BAR_CURSOR"]]
        if cols and ((9 <= cols[0] <= 10) if hit else cols[0] <= 4):
            tap("a", 2)
            return
        tick(1)


punch(hit=False)
lines = dialogue(choice=0)
check(any("punched the air" in l for l in lines) and window_on(), "a miss: %s" % lines[:1])
for n in range(3):
    punch()
    lines = dialogue(choice=0 if n < 2 else None)
    if n < 2:
        check(any("clean hole" in l for l in lines) and window_on(), "passenger %d: clean hole: %s" % (n + 1, lines[:1]))
check(mem[sym["_punched"]] == 3, "three tickets punched (%d)" % mem[sym["_punched"]])
check(any("river" in l for l in lines), "his hint: Fernand went towards the river: %s" % lines[-1:])
check(window_on() and "Extra done" in window_row(1) and "1/3" in window_row(1), "side goal noted: %r" % window_row(1))
tap("b", 4)
tap("start", 10)
tap("right", 10)
check("EXTRAS" in window_row(1) and window_row(3).startswith("OK") and window_row(6).startswith("--"),
      "notebook page 2 lists the extras: %r %r" % (window_row(3), window_row(6)))
shot("19_extras")
tap("right", 10)
album = [window_row(r) for r in range(18)]
check("VANISHED PARIS" in album[1] and "Métro ticket" in album[9] and "punched until 1973" in album[10]
      and album[12] == "???", "notebook page 3, the album: ticket found, plate not yet: %r" % album[1:13])
shot("37_album")
tap("left", 10)
check("EXTRAS" in window_row(1), "left goes back a page: %r" % window_row(1))
tap("start", 10)
shot("18_punch")
talk("Porte des Lilas")
goto(12, 100, "yx")
push("down")
secret("tiny mouse", 9)
goto(48, 96, "yx")
push("down")
talk("first train")
goto(110, 80, "yx")
push("right")
cameo("strike", "writer")
talk("mice")
goto(120, 104, "xy")
push("right")
cameo("twenty-two", "young_man")
talk("addresses")

print("a curiosity: the green ticket under the bench, then the one who knows")
goto(28, 104, "yx")
push("down")
tap("a", 0)
lines = dialogue()
check("green ticket" in " ".join(lines) and window_on() and "Curiosity: ???" in window_row(1)
      and mem[sym["_curio_flags"]] == 0x01, "found a curiosity: %s %r flags 0x%02x"
      % (lines[:1], window_row(1), mem[sym["_curio_flags"]]))
check(item_shown(HDR["ITEM_GREEN"]), "the curiosity note shows the green ticket")
shot("40_curio_green")
tap("b", 4)
tap("a", 0)
lines = dialogue()
check(any("bench" in l for l in lines) and not window_on(), "the bench again: just a bench: %s" % lines[:1])
tap("start", 10)
stage = mem[sym["_dawn_stage"]]
check(window_row(5).endswith("0%d:00" % (3 + stage)), "the notebook shows the hour, %d quests in: %r" % (stage, window_row(5)))
tap("left", 10)
page = [window_row(r) for r in range(18)]
check("MYSTERIES" in page[1] and "0/3" in page[1] and "green ticket" in page[3] and page[7] == "???",
      "notebook page 4, mysteries: the ticket found, not yet explained: %r" % page[1:8])
shot("38_mysteries")
tap("start", 10)
goto(84, 72, "yx")
push("up")
tap("a", 0)
lines = dialogue()
check(any("first class" in l for l in lines) and window_on() and "Mystery solved" in window_row(1)
      and "1/3" in window_row(1) and mem[sym["_curio_flags"]] == 0x11,
      "the puncher knows: %s %r" % (lines[:1], window_row(1)))
shot("39_mystery_solved")
tap("b", 4)
talk("Porte des Lilas")
tap("start", 10)
tap("left", 10)
check("1st-class ticket" in window_row(3) and "1991" in window_row(4), "solved: %r %r" % (window_row(3), window_row(4)))
tap("start", 10)
goto(104, 56, "xy")
goto(136, 56, "xy")
push("up")
talk("1881")
shot("36_metro_filled")
goto(104, 56, "xy")
goto(76, 100, "yx")
goto(76, 100, "xy")
push("down")
check(room() == ROOM_ENTRANCE and player() == (64, 100), "back up the steps to rue Baltard (room %d %s)" % (room(), player()))
baltard_to_street()

print("one more call, then Fernand")
street_to_cafe()
goto(116, 100, "xy")
push("up")
talk("onion soup")              # vanished Paris: the gratinee of Les Halles
goto(72, 60, "yx")
push("up")
talk("Carry it", got=("gratinée", "ITEM_GRATINEE"))   # Simone's gratinee for Mme Roux
check(mem[sym["_quest_flags"]] & 2, "carrying the gratinee")
get_jeton()
phone("MED 7302", "her box", notes=True)
for number, expect, n in (("ROQ 0614", "Butcher", 2), ("PAS 2508", "soup", None)):
    get_jeton()
    phone(number, expect)
    if n:
        check(window_on() and "Extra done" in window_row(1) and "%d/3" % n in window_row(1), "extra: %r" % window_row(1))
        tap("b", 4)
get_jeton()
phone("ODÉ 8400", "speaking clock")       # ODÉon 8400 (read): pencilled under the butcher
get_jeton()
phone("CAR 4127", "rings into nothing")   # right digits, wrong exchange
tap("start", 10)
check("3/5" in window_row(1) and "Take the soup" in window_row(2), "notebook: 3/5 quests, goal: %r %r" % (window_row(1), window_row(2)))
check(any("MED 7302" in window_row(r) for r in range(13, 17)), "Mme Roux's number noted")
shot("04_notebook")
tap("start", 10)
cafe_to_street()
street_to_quai()
goto(40, 64, "yx")
push("up")
tap("a", 0)                     # the trade: soup for the brass plate...
lines = dialogue()
check(any("an angel" in l for l in lines) and any("lighter" in l for l in lines),
      "Mme Roux gives the plate, then a pickpocket grabs it: %s" % lines[-2:])
check(window_on() and window_row(1).startswith("Catch him"), "the chase is on: %r" % window_row(1))
frames = play_chase()
check(frames > 0, "caught the thief after %d frames" % frames)
lines = dialogue()
check(any("Only brass" in l for l in lines), "the thief drops the plate: %s" % lines[:2])
quest_note("brass plate", 0x10, "ITEM_PLATE")
goto(72, 60, "yx")
push("down")
check(room() == ROOM_PONT, "onto the bridge (room %d)" % room())
push("down")
tap("a", 0)
lines = dialogue()
check(any("Cold soup" in l for l in lines), "the painter gets his wife's message: %s" % lines[:1])
check(window_on() and "3/3" in window_row(1), "all three extras done: %r" % window_row(1))
tap("b", 4)
to_fernand()
check("FERNAND" in window_row(1), "Fernand's silent call-out: %r" % window_row(1))
# Before all three clues he must say what is missing (and the notebook too).
full = clues()
for have, says, goal in ((0x01, "Simone's café", "Help Simone"),
                         (0x07, "courier came", "Tell Simone"),
                         (0x0F, "books", "Mme Roux wants")):
    mem[sym["_clue_flags"]] = have
    tap("a", 70)
    lines = dialogue()
    check(any(says in l for l in lines) and not window_on(),
          "clues 0x%02x: Fernand hints %r (%s)" % (have, says, lines[-1:] if lines else None))
    tap("start", 10)
    check(goal in window_row(2),
          "clues 0x%02x: notebook goal %r %r" % (have, window_row(2), window_row(3)))
    tap("start", 10)
mem[sym["_clue_flags"]] = full
tap("a", 70)
shot("15_fernand")
lines = dialogue(choice=0, max_pages=60)
check(any("My coltin" in l for l in lines), "Fernand opens up: %d pages" % len(lines))
check(any("two winters" in l for l in lines), "ending: he stays for the last winters")
check(any("ticket full" in l for l in lines), "every extra done: the photographer's bonus scene")
check(window_on() and window_row(2) == "THE END" and "6/6" in window_row(4) and "3/3" in window_row(5) and "9/9" in window_row(13),
      "end screen: %r %r %r" % (window_row(2), window_row(4), window_row(5)))
check(window_row(15).startswith("Rank:") and window_row(15)[5:].strip() in ("Night owl", "True fort"),
      "end screen rank (one of three curiosities): %r" % window_row(15))
check(flags() & FLAG_SOLVED, "mystery solved (FLAG_SOLVED)")
shot("14_end")
tap("start", 20)
check(window_on() and "EPISODE 2" in window_row(2) and "Who took it" in window_row(13),
      "START shows the next episode's hook: %r / %r" % (window_row(2), window_row(13)))
shot("20_next")
tap("start", 60)
check(not window_on() and clues() == 0, "START at the end returns to a fresh title screen")

pb.stop(save=False)
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
