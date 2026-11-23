#!/usr/bin/env python3
"""The player must be on screen and on free ground after every screen-edge
crossing, wherever along the edge it happens (regression: entering the quai
or the bridge beside the opening pushed the player off the top: invisible).

Usage: python3 tests/visibility_test.py ROM
"""
import os
import re
import sys

from pyboy import PyBoy

ROM = sys.argv[1] if len(sys.argv) > 1 else "build/paris-disparu.gbc"
sym = {}
for line in open(os.path.splitext(ROM)[0] + ".noi"):
    m = re.match(r"DEF (\S+) 0x([0-9A-Fa-f]+)", line)
    if m:
        sym[m.group(1)] = int(m.group(2), 16)
hdr = {}
for line in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "include", "assets.h")):
    m = re.match(r"#define (\w+) (\d+)$", line.strip())
    if m:
        hdr[m.group(1)] = int(m.group(2))
PLAYER_TILES = range(hdr["SPR_PLAYER_DOWN_0"], hdr["SPR_PORTER_0"])
ROOMS = ["street", "entrance", "market", "cafe", "quai", "pont"]

pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=False)
mem = pb.memory
failures = []


def s16(a):
    v = mem[a] | (mem[a + 1] << 8)
    return v - 0x10000 if v & 0x8000 else v


def w16(a, v):
    v &= 0xFFFF
    mem[a], mem[a + 1] = v & 0xFF, v >> 8


def player():
    return s16(sym["_player"]), s16(sym["_player"] + 2)


def room():
    return mem[sym["_cur_room"]]


def hold(button, frames):
    pb.button_press(button)
    pb.tick(frames, False)
    pb.button_release(button)
    pb.tick(2, False)


def player_oam():
    for i in range(40):
        y, x, t = mem[0xFE00 + i * 4], mem[0xFE00 + i * 4 + 1], mem[0xFE00 + i * 4 + 2]
        if t in PLAYER_TILES and 0 < y < 160 and 0 < x < 168:
            return x, y
    return None


def cross(start_room_name, x, y, button, expect_room):
    """Put the player at (x, y) in the current room, walk off the edge."""
    w16(sym["_player"], x)
    w16(sym["_player"] + 2, y)
    pb.tick(2, False)
    hold(button, 40)
    hold(button, 12)            # a few steps into the new room
    px, py = player()
    oam = player_oam()
    ok = room() == expect_room and -3 <= px <= 147 and -9 <= py <= 128 and oam is not None
    msg = "%s x=%d %s -> %s: player at %s, sprite %s" % (
        start_room_name, x, button, ROOMS[room()], (px, py), oam)
    print(("  ok    " if ok else "  FAIL  ") + msg)
    if not ok:
        failures.append(msg)


pb.tick(150, False)
hold("start", 2)
pb.tick(20, False)
hold("b", 2)                     # skip the intro
pb.tick(40, False)
hold("b", 2)                     # close the opening caption
STREET, ENTRANCE, QUAI, PONT = 0, 1, 4, 5
for x in (4, 20, 40, 60, 80, 100, 130):
    cross("street", x, 126, "down", QUAI)       # quai opening is x=56-95
    cross("quai", x if 56 <= x <= 84 else 60, 0, "up", STREET)
for x in (66, 72, 80):                          # the quai's south edge is open only at the deck
    cross("street", 60, 126, "down", QUAI)
    cross("quai", x, 127, "down", PONT)
    cross("pont", x, 0, "up", QUAI)
    cross("quai", 60, 0, "up", STREET)
for y in (64, 80, 100, 120):
    cross("street", 147, y, "right", ENTRANCE)
    cross("entrance", -3, y, "left", STREET)

pb.stop(save=False)
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
