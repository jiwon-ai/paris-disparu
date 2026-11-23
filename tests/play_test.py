#!/usr/bin/env python3
"""Headless play-through of Episode 1 in the PyBoy emulator (CGB mode).

Drives the joypad and checks RAM / VRAM state: boot, title, movement,
collision, room changes, every NPC, choices, the porter's crates, the cafe
telephone and the pause box.
Usage: python3 tests/play_test.py ROM [SCREENSHOT_DIR]
"""
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
pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=False)
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


def dialogue(choice=None, max_pages=12):
    """Read a whole dialogue: returns the first line of each page seen."""
    lines = []
    tick(70)
    for _ in range(max_pages):
        if not window_on() or window_row(1).startswith(("Dial", "Lift!", "Punch!", "New clue", "Extra done", "QUEST COMPLETE",
                                                        "Coffee rush", "Watch the", "Which cup", "Got ")) \
                or window_row(2) == "THE END":
            break
        if "▶" in window_row(2):
            if choice == 1:
                tap("right", 4)
            tap("a", 30)
            lines.append("<choice %d>" % (choice or 0))
            tick(60)
            continue
        lines.append(" ".join(window_row(r) for r in (1, 2, 3)))
        tap("a", 70)
    return lines


def win_tile(x, y):
    base = 0x9C00 if mem[0xFF40] & 0x40 else 0x9800
    return mem[base + y * 32 + x]


def lift(hit=True):
    """Porter's lift: press A when the cursor is inside the zone (or at once to miss)."""
    tap("a", 4)
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


def dial(number):
    """Phone: set the 4 digits with Up and Right, then A."""
    for i, d in enumerate(number):
        for _ in range(int(d)):
            tap("up", 2)
        if i < 3:
            tap("right", 2)
    shot("09_dial")
    tap("a", 0)


def talk(expect, choice=None, clue=None):
    """Talk/examine; clue = the clue bit this should add (its note is checked and closed)."""
    before = clues()
    tap("a", 0)
    lines = dialogue(choice)
    if clue is not None:
        note = window_on() and ("New clue" in window_row(1) or "QUEST COMPLETE" in window_row(1))
        check(note and clues() == before | clue, "new clue 0x%02x noted (clues 0x%02x)" % (clue, clues()))
        tap("b", 4)
    ok = any(expect in l for l in lines)
    check(ok and not window_on(), "dialogue %r (%d pages, first: %r)" % (expect, len(lines), lines if not ok else lines[0]))
    return lines


def quest_note(name, clue):
    check(window_on() and "QUEST COMPLETE" in window_row(1) and name in window_row(2) and clues() & clue,
          "quest complete: %r %r (clues 0x%02x)" % (window_row(1), window_row(2), clues()))
    tap("b", 4)


def play_serve(win=True):
    """Coffee rush: read each order and serve it (or serve the wrong thing)."""
    items = ("COFFEE", "SOUP", "BREAD")
    xs = (1, 8, 13)
    for _ in range(3000):
        if not (window_on() and window_row(1).startswith("Coffee rush")):
            return
        r2 = window_row(2)
        want = next((i for i, w in enumerate(items) if "Table" in r2 and w in r2), None)
        sel = [i for i, x in enumerate(xs) if win_tile(x, 4) == HDR["GLYPH_ARROW_RIGHT"]]
        if want is None or not sel:
            tick(1)
            continue
        target, cur = (want if win else (want + 1) % 3), sel[0]
        while cur != target:
            tap("right", 2)
            cur = (cur + 1) % 3
        tap("a", 2)
        tick(34)


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


# ----------------------------------------------------------------------------
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

print("intro")
tap("start", 20)
check(window_on() and "PARIS, 1967" in window_row(3), "intro page 1, the setting: %r" % window_row(3))
shot("00_intro")
tap("a", 10)
check("TONIGHT" in window_row(3) and "Find Fernand" in window_row(13), "intro page 2, the goal: %r" % window_row(13))
tap("a", 10)
check("CONTROLS" in window_row(3) and "notebook" in window_row(8), "intro page 3, controls: %r" % window_row(8))
shot("00_controls")
tap("a", 30)

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
bark = " ".join(window_row(r) for r in (1, 2, 3))
check(window_on() and "Psst" in bark, "he calls out by himself when approached: %r" % bark)
shot("13_bark")
tick(600)
check(window_on() and "Psst" in " ".join(window_row(r) for r in (1, 2, 3)),
      "his line stays while the player stands near (10 s)")
hold("down", 60)
check(not window_on(), "walking away closes his line (y=%d)" % player()[1])
goto(106, 90, "yx")
push("up")
check(window_on() and "Psst" in " ".join(window_row(r) for r in (1, 2, 3)),
      "coming back, he calls out again")
tap("right", 2)
tap("a", 0)
tick(70)
check(window_row(1).startswith("MAN WITH NOTEBOOK"), "speaker shown: %r" % window_row(1))
shot("03_dialogue")
lines = dialogue()
check(window_on() and "New clue" in window_row(1) and clues() == 0x01, "the missing hat is the first clue (0x%02x)" % clues())
tap("b", 4)
check(not window_on(), "A pages through and closes (%d pages)" % (len(lines) + 1))
tap("a", 50)
check(window_on() and "yellow hat" in " ".join(window_row(r) for r in (1, 2, 3)), "repeat line: %r" % window_row(2))
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
check("4127" in window_row(13), "the notebooks' number is written down: %r" % window_row(13))
tap("start", 10)
check(not window_on(), "START closes the notebook")

print("rue Baltard: Dede and Fernand's crates")
street_to_baltard()
shot("07_entrance")
goto(88, 80, "xy")
goto(32, 80, "xy")
push("down")
check(player()[1] < 104, "the photographer stands in the way (y=%d)" % player()[1])
tap("a", 70)
shot("16_photographer")
lines = [" ".join(window_row(r) for r in (1, 2, 3))] + dialogue()
check(any("Rungis" in l for l in lines) and any("Fernand" in l for l in lines),
      "the photographer's cameo mentions Rungis and Fernand (%d pages)" % len(lines))
to_dede()
check(player() == (64, 46), "below Dede: %s" % (player(),))
bark = " ".join(window_row(r) for r in (1, 2, 3))
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


def phone(number, expect, clue=None):
    goto(128, 60, "yx")
    push("up")
    tap("a", 0)
    dialogue(choice=0)          # "You have a jeton..." -> Yes -> dial box
    check(window_on() and "Dial" in window_row(1), "dial box opened: %r" % window_row(1))
    before = clues()
    dial(number)
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
push("up")
check(player()[1] == 38, "stopped by the counter (y=%d)" % player()[1])
tap("a", 0)
lines = dialogue(choice=0)
check(any("Give me a hand" in l for l in lines) and window_row(1).startswith("Coffee rush"), "Simone asks for help: the coffee rush")
shot("23_serve")
play_serve(win=False)
lines = dialogue(choice=0)
check(any("wrong cups" in l for l in lines), "a bad rush: Simone asks again")
play_serve()
lines = dialogue()
check(any("Fernand's hat" in l for l in lines), "a good rush: Simone gives the coltin")
quest_note("Coffee rush", 0x02)
goto(128, 60, "yx")
push("up")
talk("telephone")
goto(72, 60, "yx")
push("up")
talk("porter at the bar")       # Simone is out of jetons
get_jeton()
phone("4127", "courier", clue=0x04)
check(flags() & FLAG_PHONE_DONE, "telephone call made (FLAG_PHONE_DONE)")
shot("06_phone")
goto(72, 60, "yx")
push("up")
talk("Marcel", clue=0x08)
cafe_to_street()

print("the Seine")
street_to_quai()
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
goto(40, 64, "yx")
push("up")
talk("something hot first")   # Mme Roux has his plate: the trade starts
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
push("down")
check(player()[1] < 72, "the painter blocks the middle of the deck (y=%d)" % player()[1])
talk("big fellow")
shot("11_pont")
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
check(window_on() and "Tickets" in window_row(1) + window_row(2), "the puncher calls out: %r" % window_row(2))
talk("No ticket")
goto(28, 72, "yx")
push("up")
talk("second-class ticket")
check(flags() & 0x0400, "bought a ticket (FLAG_TICKET)")
talk("newspaper")
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
tap("start", 10)
shot("18_punch")
talk("Porte des Lilas")
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
talk("Carry it gently")         # Simone's gratinee for Mme Roux
check(mem[sym["_quest_flags"]] & 2, "carrying the gratinee")
get_jeton()
phone("7302", "gone to her box")
for number, expect, n in (("0614", "Butcher", 2), ("2508", "soup", None)):
    get_jeton()
    phone(number, expect)
    if n:
        check(window_on() and "Extra done" in window_row(1) and "%d/3" % n in window_row(1), "extra: %r" % window_row(1))
        tap("b", 4)
tap("start", 10)
check("3/5" in window_row(1) and "Take the soup" in window_row(2), "notebook: 3/5 quests, goal: %r %r" % (window_row(1), window_row(2)))
check(any("7302" in window_row(r) for r in range(13, 17)), "Mme Roux's number noted")
shot("04_notebook")
tap("start", 10)
cafe_to_street()
street_to_quai()
goto(40, 64, "yx")
push("up")
talk("an angel", clue=0x10)     # the trade: soup for the brass plate
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
                         (0x0F, "by the books", "Mme Roux wants")):
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
lines = dialogue(choice=0, max_pages=40)
check(any("My coltin" in l for l in lines), "Fernand opens up: %d pages" % len(lines))
check(any("two winters" in l for l in lines), "ending: he stays for the last winters")
check(any("full of holes" in l for l in lines), "every extra done: the photographer's bonus scene")
check(window_on() and window_row(2) == "THE END" and "6/6" in window_row(4) and "3/3" in window_row(5),
      "end screen: %r %r %r" % (window_row(2), window_row(4), window_row(5)))
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
