#!/usr/bin/env python3
"""Save / continue in battery RAM (PyBoy, CGB mode).

1. New game, set some progress, walk into the café: the room change saves.
2. A fresh emulator with that cartridge RAM: title -> language -> the menu
   offers Continue; Continue puts the player back in the café with the
   progress intact and says "Welcome back!".
3. Same RAM, "New game": the intro plays and progress starts from zero.
Usage: python3 tests/save_test.py [ROM]
"""
import io
import os
import re
import sys

from pyboy import PyBoy

ROM = sys.argv[1] if len(sys.argv) > 1 else "build/paris-disparu.gbc"
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "tools"))
import font  # noqa: E402

FONT_BASE = 128
CHARS = list(font.GLYPHS.keys())
ROOM_STREET, ROOM_CAFE = 0, 3
failures = []
sym = {}
for line in open(os.path.splitext(ROM)[0] + ".noi"):
    m = re.match(r"DEF (\S+) 0x([0-9A-Fa-f]+)", line)
    if m:
        sym[m.group(1)] = int(m.group(2), 16)


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        failures.append(msg)


class Game:
    def __init__(self, ram):
        self.pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=True, ram_file=ram)
        self.mem = self.pb.memory

    def tick(self, n=1):
        self.pb.tick(n, True)

    def tap(self, button, wait=8):
        self.pb.button_press(button)
        self.tick(2)
        self.pb.button_release(button)
        self.tick(wait)

    def s16(self, a):
        v = self.mem[a] | (self.mem[a + 1] << 8)
        return v - 0x10000 if v & 0x8000 else v

    def player(self):
        return self.s16(sym["_player"]), self.s16(sym["_player"] + 2)

    def room(self):
        return self.mem[sym["_cur_room"]]

    def row(self, r):
        base = 0x9C00 if self.mem[0xFF40] & 0x40 else 0x9800
        t = ""
        for x in range(20):
            i = self.mem[base + r * 32 + x] - FONT_BASE
            t += CHARS[i] if 0 <= i < len(CHARS) else "|"
        return t.strip("|").strip()

    def screen_text(self):
        return " / ".join(self.row(r) for r in range(18))

    def walk(self, axis, target):
        idx = 0 if axis == "x" else 1
        start, stuck = self.room(), 0
        for _ in range(600):
            pos = self.player()[idx]
            if pos == target or self.room() != start:
                break
            b = ("right" if target > pos else "left") if axis == "x" else ("down" if target > pos else "up")
            self.pb.button_press(b)
            self.tick(1)
            self.pb.button_release(b)
            self.tick(1)
            stuck = stuck + 1 if self.player()[idx] == pos else 0
            if stuck > 6:
                break

    def push(self, b, frames=90):
        start = self.room()
        self.pb.button_press(b)
        for _ in range(frames):
            self.tick(1)
            if self.room() != start:
                break
        self.pb.button_release(b)
        self.tick(40)


print("1. new game, progress, save on entering the café")
g = Game(io.BytesIO(bytes(8192)))
g.tick(240)
g.tap("a", 60)            # title
g.tap("a", 30)            # language: English
check("Continue" not in g.screen_text(), "blank RAM: no Continue menu")
for _ in range(3):
    g.tap("a", 12)        # intro pages
g.tick(60)
check(g.room() == ROOM_STREET, "new game starts in the street")
g.mem[sym["_clue_flags"]] = 0x05
g.mem[sym["_secret_flags"]] = 0x22
g.tap("b", 10)            # close the caption
g.walk("y", 90)
g.walk("x", 28)
g.push("up")
check(g.room() == ROOM_CAFE, "walked into the café (room %d %s)" % (g.room(), g.player()))
pos = g.player()
ram = io.BytesIO()
g.pb.stop(ram_file=ram)
data = ram.getvalue()
check(data[:4] == b"PDS4", "cartridge RAM holds the save: %r" % data[:4])

print("2. continue")
g = Game(io.BytesIO(data))
g.tick(240)
g.tap("a", 60)
g.tap("a", 30)
check("Continue" in g.screen_text() and "New game" in g.screen_text(), "a save: the menu offers Continue / New game")
g.tap("a", 80)            # Continue (first option)
check(g.room() == ROOM_CAFE and g.player() == pos, "back in the café at %s (room %d %s)" % (pos, g.room(), g.player()))
check(g.mem[sym["_clue_flags"]] == 0x05 and g.mem[sym["_secret_flags"]] == 0x22,
      "progress restored: clues 0x%02x secrets 0x%02x" % (g.mem[sym["_clue_flags"]], g.mem[sym["_secret_flags"]]))
check("Welcome back" in g.row(1), "caption: %r" % g.row(1))
g.tap("select", 30)
check(g.mem[sym["_music_muted"]] == 1 and "Sound: off" in g.row(1) and g.mem[0xFF1C] & 0x60 == 0
      and g.mem[0xFF25] == 0,
      "SELECT: all sound off (caption %r, wave volume 0x%02x, NR51 0x%02x)" % (g.row(1), g.mem[0xFF1C], g.mem[0xFF25]))
ram2 = io.BytesIO()
g.pb.stop(ram_file=ram2)
check(ram2.getvalue()[11] == 1, "music off is saved (byte 11 = %d)" % ram2.getvalue()[11])
g = Game(io.BytesIO(ram2.getvalue()))
g.tick(240)
g.tap("a", 60)
g.tap("a", 30)
g.tap("a", 80)
check(g.mem[sym["_music_muted"]] == 1 and g.mem[0xFF25] == 0, "continue: sound still off")
g.tap("b", 10)
g.tap("start", 60)
check("SELECT: sound/quit" in g.screen_text(), "the notebook says SELECT opens the menu: %r" % g.screen_text()[:200])
g.tap("select", 30)
check("MENU" in g.screen_text() and "Sound: off" in g.screen_text() and "Save and quit" in g.screen_text(),
      "SELECT in the notebook: the menu: %r" % g.screen_text()[:200])
g.tap("a", 30)
check(g.mem[sym["_music_muted"]] == 0 and "Sound: on" in g.screen_text(), "menu: A on Sound turns it on")
g.tap("b", 60)
check("NOTEBOOK" in g.screen_text(), "B: back to the notebook: %r" % g.screen_text()[:40])
g.tap("start", 20)
g.tap("select", 30)
check(g.mem[sym["_music_muted"]] == 1 and "Sound: off" in g.row(1), "SELECT while walking still toggles the sound")
g.tap("select", 30)
check(g.mem[sym["_music_muted"]] == 0 and "Sound: on" in g.row(1) and g.mem[0xFF25] == 0xFF,
      "SELECT again: sound on (%r, NR51 0x%02x)" % (g.row(1), g.mem[0xFF25]))
# Save and quit: notebook, SELECT, "Save and quit", confirm (cursor starts on No)
pos, rm = g.player(), g.room()
g.tap("b", 10)
g.tap("start", 60)
g.tap("select", 30)
g.tap("down", 10)
g.tap("a", 30)
check("Quit to the title?" in g.screen_text() and "Progress is saved." in g.screen_text(),
      "quit asks first: %r" % g.screen_text()[:120])
g.tap("a", 30)                       # No (the default): back to the menu
check("MENU" in g.screen_text(), "No: back to the menu")
g.tap("a", 30)                       # Save and quit again
g.tap("up", 10)
g.tap("a", 120)                      # Yes
check(not g.screen_text().strip() or "NOTEBOOK" not in g.screen_text(), "Yes: the notebook is gone")
g.tick(240)
g.tap("a", 60)                       # title, language
g.tap("a", 30)
check("Continue" in g.screen_text(), "after quitting, the title offers Continue: %r" % g.screen_text()[:80])
g.tap("a", 80)
check(g.room() == rm and g.player() == pos, "continue after quitting: same place (room %d %s, was %d %s)"
      % (g.room(), g.player(), rm, pos))
g.pb.stop(save=False)

print("3. new game over a save")
g = Game(io.BytesIO(data))
g.tick(240)
g.tap("a", 60)
g.tap("a", 30)
g.tap("down", 8)
g.tap("a", 30)            # New game
check("PARIS 21 NOV 1967" in g.screen_text(), "the intro plays")
check(g.mem[sym["_clue_flags"]] == 0, "no progress carried over (clues 0x%02x)" % g.mem[sym["_clue_flags"]])
g.pb.stop(save=False)

print("4. curiosities: a saved game carrying one, shown to the one who knows")


def crafted(room, x, y, d, curio):
    b = list(b"PDS4") + [room, x & 255, x >> 8, y & 255, y >> 8, d, 1, 0] + [0] * 13 + [curio, 0, 0, 0, 0]   # coin, then the 3 bytes of words found
    return bytes(b + [(0xA5 + sum(b)) & 255]) + bytes(8192 - len(b) - 1)


def ask(room, x, y, d, push, curio, expect, flags_after):
    g = Game(io.BytesIO(crafted(room, x, y, d, curio)))
    g.tick(240)
    g.tap("a", 60)
    g.tap("a", 30)
    g.tap("a", 80)        # Continue
    g.tap("b", 10)        # close "Welcome back"
    g.push(push, 30)
    g.tap("a", 70)
    first = g.row(1) + " " + g.row(3)
    for _ in range(6):    # page on to the "Mystery solved" note
        if "Mystery solved" in g.row(1):
            break
        g.tap("a", 70)
    check(expect in first and g.mem[sym["_curio_flags"]] == flags_after and "Mystery solved" in g.row(1)
          and "/3" in g.row(1), "%r -> flags 0x%02x, note %r" % (first, g.mem[sym["_curio_flags"]], g.row(1)))
    g.pb.stop(save=False)


ask(6, 120, 104, 3, "right", 0x02, "YOUNG MAN", 0x22)    # the roller -> the young man (metro)
ask(5, 80, 30, 0, "down", 0x05, "Tutankhamun", 0x45)     # the stub -> the painter (bridge)

print("\n%d failure(s)" % len(failures))
sys.exit(1 if failures else 0)
