#!/usr/bin/env python3
"""Korean option smoke test (PyBoy): language menu, Korean intro and
notebook (Galmuri7 syllable tiles), a Korean dialogue page (Galmuri11 tiles
in VRAM bank 0 at 0x9000). Run: python3 tests/ko_test.py [rom]"""
import re
import sys
from pyboy import PyBoy

ROM = sys.argv[1] if len(sys.argv) > 1 else "build/paris-disparu.gbc"
sym = {}
for line in open(ROM[:-4] + ".noi"):
    m = re.match(r"DEF (\S+) 0x([0-9A-Fa-f]+)", line)
    if m:
        sym[m.group(1)] = int(m.group(2), 16)
pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=False)
mem = pb.memory
failures = []


def check(ok, msg):
    print(("  ok    " if ok else "  FAIL  ") + msg)
    if not ok:
        failures.append(msg)


def tap(b, wait=8):
    pb.button_press(b)
    pb.tick(2, False)
    pb.button_release(b)
    pb.tick(wait, False)


SHOTS = sys.argv[2] if len(sys.argv) > 2 else None


def shot(name):
    if SHOTS:
        pb.tick(1, True)
        pb.screen.image.resize((480, 432)).save("%s/%s.png" % (SHOTS, name))


def player():
    a = sym["_player"]
    def s16(i):
        v = mem[i] | mem[i + 1] << 8
        return v - 0x10000 if v & 0x8000 else v
    return s16(a), s16(a + 2)


def room():
    return mem[sym["_cur_room"]]


def walk(axis, target):
    idx = 0 if axis == "x" else 1
    start, stuck = room(), 0
    for _ in range(600):
        pos = player()[idx]
        if pos == target or room() != start:
            break
        b = ("right" if target > pos else "left") if axis == "x" else ("down" if target > pos else "up")
        pb.button_press(b)
        pb.tick(1, False)
        pb.button_release(b)
        pb.tick(1, False)
        stuck = stuck + 1 if player()[idx] == pos else 0
        if stuck > 6:
            break


def push(b, frames=90):
    start = room()
    pb.button_press(b)
    for _ in range(frames):
        pb.tick(1, False)
        if room() != start:
            break
    pb.button_release(b)
    pb.tick(30, False)


def win_tiles(row):
    base = 0x9C00 if mem[0xFF40] & 0x40 else 0x9800
    return [mem[0, base + row * 32 + x] for x in range(20)]


pb.tick(150, False)
tap("start", 30)
tap("down", 8)
tap("a", 30)
check(mem[sym["_lang"]] == 1, "Korean chosen on the language menu")
# Intro page 1, row 3: "파리, 1967년" = syllable tiles (ids < 128) + font tiles.
row = win_tiles(3)
check(any(t < 128 for t in row) and any(t >= 128 for t in row), "Korean intro mixes syllable and font tiles: %s" % row)
tile = bytes(mem[0, 0x9000 + row[2] * 16: 0x9000 + row[2] * 16 + 16])
check(any(tile[1::2]), "a syllable tile has ink")
tap("b", 40)                       # skip the intro: caption
tap("b", 40)
tap("b", 40)
mem[sym["_player"]], mem[sym["_player"] + 1] = 120, 0
mem[sym["_player"] + 2], mem[sym["_player"] + 3] = 82, 0
tap("up", 10)
for _ in range(40):
    pb.tick(1, False)
for _ in range(60):
    pb.tick(1, False)
row1 = win_tiles(1) + win_tiles(2)
check(any(t < 72 for t in row1), "a Korean dialogue line uses text tiles 0-71: %s" % win_tiles(1))
ink = sum(1 for i in range(72 * 16) if i % 2 and mem[0, 0x9000 + i])
check(ink > 20, "Galmuri11 text tiles have ink (%d bytes)" % ink)
shot("ko_bark")
tap("b", 10)
tap("start", 30)
check(any(t < 128 for t in win_tiles(1)), "Korean notebook title")
shot("ko_notebook")
tap("b", 30)                       # close the notebook
# Dede in rue Baltard: a multi-page dialogue ending on a Korean choice.
walk("x", 88)
walk("y", 120)
push("down")
check(room() == 1, "rue Baltard (room %d) %s" % (room(), player()))
walk("x", 88)
walk("y", 70)
walk("x", 64)
push("up")
tap("a", 60)
shot("ko_dede_1")
check(any(t < 72 for t in win_tiles(1) + win_tiles(3)), "Dede speaks Korean")
for i in range(3):
    tap("a", 4)                    # finish the page
    tap("a", 60)                   # next page
    shot("ko_dede_%d" % (i + 2))
for _ in range(12):                # finish Dede's pages until the options
    row = win_tiles(2)
    if row[3] >= 128 and row[3] != row[4] and any(72 <= t < 128 for t in row[4:8]):
        break
    tap("a", 45)                   # pages reveal in at most 36 frames
row = win_tiles(2)
check(row[3] >= 128 and any(72 <= t < 128 for t in row[4:8]), "Korean yes/no choice: %s" % row)
shot("ko_choice")
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
