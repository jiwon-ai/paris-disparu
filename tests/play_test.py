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
ROOM_STREET, ROOM_ENTRANCE, ROOM_MARKET, ROOM_CAFE = range(4)
FLAG_TASK_DONE, FLAG_PHONE_DONE = 0x0008, 0x0200
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
        if not window_on():
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


def talk(expect, choice=None):
    tap("a", 0)
    lines = dialogue(choice)
    ok = any(expect in l for l in lines)
    check(ok and not window_on(), "dialogue %r (%d pages, first: %r)" % (expect, len(lines), lines[0] if lines else ""))
    return lines


# ----------------------------------------------------------------------------
print("boot")
tick(90)
check(mem[0xFF40] & 0x80, "LCD is on after boot")
check(pb.cartridge_title.strip().startswith("PARISDISPAR"), "cartridge title read by emulator")
shot("01_title")

print("street")
tap("start", 40)
check(player() == (28, 72) and room() == ROOM_STREET, "spawned in the street at (28,72): %s" % (player(),))
check(window_on() and "Rambuteau" in window_row(1), "location caption: %r" % window_row(1))
shot("02_street")
tick(160)
check(not window_on(), "caption closes by itself")

hold("right", 20)
check(player()[0] == 48, "moved right 20 px (x=%d)" % player()[0])
hold("right", 120)
check(player()[0] == 67, "lamp post blocks at x=67 (x=%d)" % player()[0])
hold("up", 60)
check(player()[1] == 62, "facade blocks at y=62 (y=%d)" % player()[1])
hold("down", 200)
check(player()[1] == 128, "bottom edge blocks at y=128 (y=%d)" % player()[1])
hold("left", 200)
check(player()[0] == -3, "left edge blocks at x=-3 (x=%d)" % player()[0])

goto(106, 90, "yx")
push("up")
check(player() == (106, 62), "next to the man with the notebook: %s" % (player(),))
tap("right", 2)
tap("a", 0)
tick(70)
check(window_row(1).startswith("L'HOMME AU CARNET"), "speaker shown: %r" % window_row(1))
shot("03_dialogue")
lines = dialogue()
check(not window_on(), "A pages through and closes (%d pages)" % (len(lines) + 1))
tap("a", 50)
check(window_on() and "hésite" in window_row(2) + window_row(3), "repeat line differs: %r" % window_row(2))
tap("b", 4)
check(not window_on(), "B closes the dialogue")

tap("start", 10)
check(window_on() and "PAUSE" in window_row(1), "START opens the pause box")
shot("04_pause")
tap("start", 10)
check(not window_on(), "START closes the pause box")

print("cafe")
goto(28, 90, "yx")
push("up")
check(room() == ROOM_CAFE and player() == (72, 118), "entered the cafe through its door: room %d %s" % (room(), player()))
shot("05_cafe")
push("up")
check(player()[1] == 38, "stopped by the counter (y=%d)" % player()[1])
talk("SIMONE", choice=0)
goto(128, 60, "yx")
push("up")
talk("téléphone")
goto(72, 60, "yx")
push("up")
talk("jeton")
goto(128, 60, "yx")
push("up")
lines = talk("jeton", choice=0)
check(flags() & FLAG_PHONE_DONE, "telephone call made (FLAG_PHONE_DONE)")
shot("06_phone")
goto(88, 60, "yx")
push("up")
talk("boucherie")
goto(72, 100, "yx")
push("down")
check(room() == ROOM_STREET and player() == (28, 72), "back in the street at the cafe door: %d %s" % (room(), player()))

print("market entrance and the porter")
goto(140, 90, "yx")
push("right")
check(room() == ROOM_ENTRANCE, "east edge leads to the pavilion (room %d, %s)" % (room(), player()))
shot("07_entrance")
goto(100, 90, "yx")
push("up")
check(player() == (100, 66), "below Fernand: %s" % (player(),))
talk("FERNAND", choice=0)


def fetch_crate(expect):
    goto(124, 90, "yx")
    push("up")
    talk(expect)


def to_market():
    goto(72, 90, "yx")
    push("up")
    check(room() == ROOM_MARKET, "through the arch into the pavilion (room %d)" % room())


def to_entrance():
    goto(72, 100, "yx")
    push("down")
    check(room() == ROOM_ENTRANCE, "back out to the entrance (room %d)" % room())


fetch_crate("choux")
check(mem[sym["_carrying"]] == 0, "carrying the cabbages")
to_market()
shot("08_market")
goto(72, 38, "xy")
push("up")
talk("bon étal")
goto(36, 38, "xy")
push("up")
talk("Mes choux")
push("left")
talk("Le prix")               # the merchant herself
to_entrance()
fetch_crate("pommes")
to_market()
goto(72, 38, "xy")
push("up")
talk("Tu poses")
to_entrance()
fetch_crate("carottes")
to_market()
goto(112, 38, "xy")
push("up")
talk("Tu poses")
to_entrance()
goto(100, 90, "yx")
push("up")
talk("débutant")
check(flags() & FLAG_TASK_DONE, "porter's task done (FLAG_TASK_DONE)")

print("back to the street")
goto(60, 90, "yx")
push("left", 200)
check(room() == ROOM_STREET, "west edge leads back to the street (room %d)" % room())
goto(106, 90, "yx")
push("up")
tap("right", 2)
talk("secrets")

pb.stop(save=False)
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
