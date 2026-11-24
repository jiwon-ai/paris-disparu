#!/usr/bin/env python3
"""Episode 2: headless play-through from the title to THE END in PyBoy.

Plays the main story the way a player would: talks to people, rides the
métro, plays the mini-games for real (reading the screen to time the
pétanque throw, find the red scarf, etc.) and checks the story flags after
each step. Usage: python3 tests/story_test.py [ROM] [SHOT_DIR]
"""
import io
import os
import re
import sys

from pyboy import PyBoy

HERE = os.path.dirname(os.path.abspath(__file__))
ROM = sys.argv[1] if len(sys.argv) > 1 else os.path.join(HERE, "..", "build", "paris-disparu.gbc")
SHOTS = sys.argv[2] if len(sys.argv) > 2 else None
sym = {}
for line in open(os.path.splitext(ROM)[0] + ".noi"):
    m = re.match(r"DEF (\S+) 0x([0-9A-Fa-f]+)", line)
    if m:
        sym[m.group(1)] = int(m.group(2), 16)
H = {}
for line in open(os.path.join(HERE, "..", "include", "assets.h")):
    m = re.match(r"#define (\w+) (\d+)", line.strip())
    if m:
        H[m.group(1)] = int(m.group(2))
for line in open(os.path.join(HERE, "..", "include", "game.h")):
    m = re.match(r"#define (\w+)\s+(0x[0-9A-Fa-f]+|\d+)", line.strip())
    if m:
        H[m.group(1)] = int(m.group(2), 0)

ST_WALK, ST_DIALOGUE, ST_NOTEBOOK, ST_METRO, ST_DIAL, ST_END, ST_GAME, ST_HOWTO, ST_DAWN, ST_EPILOGUE = range(10)
pb = PyBoy(ROM, window="null", cgb=True, sound_emulated=False, ram_file=io.BytesIO(bytes(8192)))
M = pb.memory
failures = []
frames = 0


def check(cond, msg):
    print(("  ok    " if cond else "  FAIL  ") + msg)
    if not cond:
        failures.append(msg)


def run(n=1):
    global frames
    for _ in range(n):
        pb.tick()
    frames += n


def press(b, n=4, after=6):
    pb.button_press(b)
    run(n)
    pb.button_release(b)
    run(after)


def shot(name):
    if SHOTS:
        pb.screen.image.save(os.path.join(SHOTS, name + ".png"))


def state():
    return M[sym["_state"]]


def room():
    return M[sym["_cur_room"]]


def flags16(name):
    return M[sym[name]] | M[sym[name] + 1] << 8


def clue(bit):
    return bool(M[sym["_clue_flags"]] & bit)


def has(bit):
    return bool(flags16("_game_flags") & bit)


def place(x, y):
    p = sym["_player"]
    M[p], M[p + 1], M[p + 2], M[p + 3] = x & 255, (x >> 8) & 255, y & 255, (y >> 8) & 255


def win(x, y):
    base = 0x9C00 if M[0xFF40] & 0x40 else 0x9800
    return M[0, base + y * 32 + x]


def oam(i):
    a = 0xFE00 + i * 4
    return M[a], M[a + 1], M[a + 2], M[a + 3]


# ---------------------------------------------------------------- mini-games
def play_petanque():
    jack = next((c for c in range(1, 19) if win(c, 3) == H["GLYPH_DOT"]), None)
    cur = next((c for c in range(2, 18) if win(c, 5) == H["GLYPH_BAR_CURSOR"]), None)
    if jack is not None and cur == jack:
        press("a", 2, 2)
    else:
        run(1)


def play_find():
    who = None
    for i in range(6):
        y, x, t, a = oam(24 + i * 2)
        if y and (a & 7) == 6:
            who = i
    hand = oam(16)[1]
    if who is None:
        run(1)
        return
    target = [12, 36, 60, 84, 108, 132][who] + 8
    if hand < target:
        press("right", 2, 4)
    elif hand > target:
        press("left", 2, 4)
    else:
        press("a", 2, 4)


def play_jazz():
    if win(4, 3) == H["GLYPH_NOTE"] or win(3, 3) == H["GLYPH_NOTE"]:
        press("a", 2, 6)
    else:
        run(1)


def play_spam():
    press("a", 2, 3)


def play_game():
    if room() == H["ROOM_BASPORT"] and any(oam(24 + i * 2)[0] for i in range(6)):
        play_find()
    elif win(3, 2) == H["GLYPH_ARROW_DOWN"] and win(3, 4) in (H["GLYPH_PIPE"], H["GLYPH_HEART"]):
        play_jazz()
    elif any(win(c, 5) in (H["GLYPH_BAR"], H["GLYPH_BAR_CURSOR"]) for c in range(2, 18)):
        play_petanque()
    else:
        play_spam()


def settle(limit=6000, choice=0):
    """Press through dialogues, rule cards and games until walking again."""
    quiet = 0
    for _ in range(limit):
        s = state()
        if s == ST_WALK:
            quiet += 1
            run(1)
            if quiet > 40:
                return True
            continue
        quiet = 0
        if s in (ST_DIALOGUE, ST_HOWTO):
            if choice:
                press("right", 2, 2)
            press("a", 3, 8)
        elif s == ST_GAME:
            play_game()
        elif s in (ST_METRO, ST_END, ST_DAWN, ST_EPILOGUE):
            return True
        else:
            press("b")
    return False


def talk(x, y, choice=0):
    """Stand below the NPC (or thing) at pixel (x, y), face up, press A."""
    settle()
    place(x, y + 8)
    press("up", 2, 4)
    place(x, y + 8)
    press("a")
    return settle(choice=choice)


def look(tx, ty):
    """Examine the hotspot tile (tx, ty) from the tile below it."""
    settle()
    place(tx * 8 - 4, ty * 8 - 2)      # feet in row ty + 1, facing row ty
    press("up", 2, 4)
    place(tx * 8 - 4, ty * 8 - 2)
    press("a")
    return settle()


def walk(direction, x, y, frames_=200):
    settle()
    place(x, y)
    start = room()
    pb.button_press(direction)
    for _ in range(frames_):
        run(1)
        if room() != start:
            break
    pb.button_release(direction)
    run(90)
    settle()


def ride(stop, hot=None):
    """Use the métro at hotspot tile hot (x, y), buy if needed, ride to stop."""
    for _ in range(3):
        if hot:
            look(*hot)
        if state() == ST_METRO:
            break
        settle()
    if state() != ST_METRO:
        return False
    run(30)
    start = 1 if room() == H["ROOM_METRO"] else 0
    for _ in range((stop - start) % 4):
        press("down")
    press("a")
    run(150)
    settle()
    return True


# ---------------------------------------------------------------- the story
run(200)
press("start")
run(40)
press("a")            # English
run(40)
for _ in range(6):
    press("b", after=20)
run(60)
settle()
check(room() == H["ROOM_BALTARD"], "new game starts in rue Baltard")
shot("01_baltard")

talk(64, 40)
check(has(H["F_MET_FERNAND"]), "Fernand: the plate is stolen")
walk("down", 72, 124)
check(room() == H["ROOM_QUAI"], "rue Baltard -> the quai")
talk(40, 46)
check(clue(H["K_TRAIL"]) and has(H["F_TICKET"]), "Mme Roux: red scarf, a métro ticket")
walk("up", 72, 30)
check(room() == H["ROOM_BALTARD"], "back to rue Baltard")
walk("right", 72, 104)
check(room() == H["ROOM_METRO"], "down into the métro")
check(ride(H["STOP_CITE"], (4, 8)), "métro map opens with the ticket")
check(room() == H["ROOM_FLEURS"], "ride to Cité")
shot("02_cite")

walk("left", 40, 124)
check(room() == H["ROOM_DAUPHINE"], "Cité -> Place Dauphine")
for _ in range(8):
    talk(56, 96)
    if clue(H["K_NAME"]):
        break
check(clue(H["K_NAME"]), "pétanque won: his name")
walk("right", 120, 124)
walk("right", 120, 124)
check(room() == H["ROOM_NOTREDAME"], "-> Notre-Dame")
talk(104, 112)
check(has(H["F_NUMBER"]), "the clocharde: a phone number")
walk("left", 40, 124)
check(room() == H["ROOM_FLEURS"], "-> back to the flower market")
check(ride(H["STOP_STMICHEL"], (3, 11)), "métro from Cité (buys a ticket)")
check(room() == H["ROOM_STMICHEL"], "ride to Saint-Michel")
shot("03_stmichel")

look(7, 11)
check(has(H["F_MUTE"]), "the fountain: a trumpet mute")
walk("right", 120, 124)
check(room() == H["ROOM_HUCHETTE"], "-> rue de la Huchette")
talk(88, 92)
check(has(H["F_INSIDE"]), "the doorman lets you in")
walk("up", 76, 92)
check(room() == H["ROOM_CAVEAU"], "-> the jazz cellar")
for _ in range(8):
    talk(72, 40)
    if clue(H["K_HIDE"]):
        break
check(clue(H["K_HIDE"]), "jazz won: where he sleeps")
shot("04_caveau")
walk("down", 76, 124)
check(room() == H["ROOM_HUCHETTE"], "back up to the street")
walk("left", 40, 124)
check(room() == H["ROOM_STMICHEL"], "-> Saint-Michel")
check(ride(H["STOP_STGERMAIN"], (16, 11)), "métro from Saint-Michel")
check(room() == H["ROOM_STGERMAIN"], "ride to Saint-Germain")
walk("right", 120, 124)
check(room() == H["ROOM_TERRASSE"], "-> the café terrace")
talk(48, 96)
check(clue(H["K_HEART"]), "the old poet: why he took it")
walk("left", 100, 86)
check(room() == H["ROOM_STGERMAIN"], "-> back to the church")
check(ride(H["STOP_STMICHEL"], (3, 11)), "métro from Saint-Germain (free when broke)")
check(room() == H["ROOM_STMICHEL"], "back to Saint-Michel")
walk("left", 16, 104)
check(room() == H["ROOM_BASPORT"], "down to the low quay")
for _ in range(4):
    talk(72, 96)
    if clue(H["K_PLATE"]):
        break
check(clue(H["K_PLATE"]), "found Lulu: the plate")
shot("05_lulu")
walk("right", 140, 104)
check(room() == H["ROOM_STMICHEL"], "back up to Saint-Michel")
check(ride(H["STOP_HALLES"], (16, 11)), "métro home")
check(room() == H["ROOM_METRO"], "ride to Les Halles")
walk("down", 76, 124)
check(room() == H["ROOM_BALTARD"], "up to rue Baltard")
talk(64, 40)
check(has(H["F_SOLVED"]), "the plate goes back to Fernand")
for _ in range(3000):           # flowers, dawn, epilogue, the end
    s = state()
    if s == ST_END:
        break
    if s == ST_GAME:
        play_game()
    else:
        press("a" if s != ST_WALK else "start", 3, 10)
shot("06_end")
check(state() == ST_END, "THE END screen reached")
print("%d frames (%.1f min of play)" % (frames, frames / 60 / 60))
pb.stop(save=False)
print("FAILED: %d" % len(failures) if failures else "ALL OK")
sys.exit(1 if failures else 0)
