#!/usr/bin/env python3
"""Headless gameplay test of the ROM in the PyBoy emulator (CGB mode).

Drives the joypad and checks RAM / VRAM state: boot, title, movement,
collision, NPC dialogue (A to advance, B to close) and the pause box.
Optionally saves screenshots:  python3 tests/play_test.py ROM [SHOT_DIR]
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
failures = []


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        failures.append(msg)


def symbols(rom):
    noi = os.path.splitext(rom)[0] + ".noi"
    out = {}
    for line in open(noi):
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
    return s16(a), s16(a + 2), mem[a + 4]


def hold(button, frames):
    pb.button_press(button)
    pb.tick(frames, True)
    pb.button_release(button)
    pb.tick(1, True)


def tap(button, wait=8):
    pb.button_press(button)
    pb.tick(2, True)
    pb.button_release(button)
    pb.tick(wait, True)


def window_on():
    return bool(mem[0xFF40] & 0x20)


def window_row(row):
    base = 0x9C00 if mem[0xFF40] & 0x40 else 0x9800
    text = ""
    for x in range(20):
        t = mem[base + row * 32 + x]
        i = t - FONT_BASE
        text += CHARS[i] if 0 <= i < len(CHARS) else "|"
    return text


def shot(name):
    if SHOTS:
        os.makedirs(SHOTS, exist_ok=True)
        img = pb.screen.image.resize((480, 432))
        img.save(os.path.join(SHOTS, name + ".png"))


print("boot")
pb.tick(90, True)
check(mem[0xFF40] & 0x80, "LCD is on after boot")
check(pb.cartridge_title.strip().startswith("PARISDISPAR"), "cartridge title read by emulator")
shot("01_title")

print("title -> street")
tap("start", 40)
x, y, d = player()
check((x, y) == (28, 72), "player spawned at (28,72), got (%d,%d)" % (x, y))
check(window_on() and "Rambuteau" in window_row(1), "location caption shown: %r" % window_row(1))
shot("02_street")
pb.tick(160, True)
check(not window_on(), "caption closes by itself")

print("movement and collision")
hold("right", 20)
x2, y2, d = player()
check(x2 == x + 20 and d == 3, "moved right 20 px facing right (x=%d)" % x2)
hold("right", 120)
x3, _, _ = player()
check(x3 == 67, "stopped by the lamp post at x=67 (x=%d)" % x3)
hold("up", 60)
_, y4, _ = player()
check(y4 == 62, "stopped by the facade at y=62 (y=%d)" % y4)
hold("down", 200)
_, y5, _ = player()
check(y5 == 128, "stopped at the bottom edge y=128 (y=%d)" % y5)
hold("left", 200)
x6, _, _ = player()
check(x6 == -3, "stopped at the left edge x=-3 (x=%d)" % x6)

print("talk to the porter")
hold("right", 109)          # x = 106
hold("up", 80)              # back to the pavement under the facade
x7, y7, _ = player()
check((x7, y7) == (106, 62), "next to the porter at (106,62), got (%d,%d)" % (x7, y7))
tap("right", 4)
tap("a", 90)
check(window_on(), "dialogue box opened")
check(window_row(1).strip("|").startswith("FERNAND"), "first line: %r" % window_row(1))
shot("03_dialogue")
pages = 1
for _ in range(10):
    tap("a", 70)  # next page; 70 frames is enough to type a full page
    if not window_on():
        break
    pages += 1
check(not window_on(), "dialogue closed after the last page (%d pages)" % pages)
check(mem[sym["_game_flags"]] & 1, "FLAG_MET_PORTER set")

tap("a", 60)
check(window_on() and "Encore" in window_row(1), "second talk uses the repeat line: %r" % window_row(1))
tap("b", 4)
check(not window_on(), "B closes the dialogue")

print("examine and pause")
hold("left", 60)            # x = 47: in front of the shop window at tiles 8-13
hold("up", 4)
tap("a", 90)
check(window_on(), "examining the shopfront opens text: %r" % window_row(1))
tap("b", 4)
tap("start", 10)
check(window_on() and "PAUSE" in window_row(1), "START opens the pause box")
shot("04_pause")
tap("start", 10)
check(not window_on(), "START closes the pause box")

pb.stop(save=False)
print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
