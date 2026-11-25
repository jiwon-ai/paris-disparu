"""Episode 2 scenes (see scenes.py). One function per room."""
from scenes import Painter, scene, TEXT_PAL, LAMP_PAL

# Shared night colours (5-bit CGB RGB).
NIGHT = (2, 2, 7)
NIGHT2 = (4, 4, 11)
NIGHT3 = (7, 6, 15)
STAR = (24, 24, 28)
INK = (2, 2, 5)
SH = (8, 7, 10)
STONE = (16, 14, 12)
STONEL = (23, 21, 17)
AMBER = (28, 19, 6)
GLOW = (31, 28, 16)
SETT0, SETT1, SETT2, SETT3 = (3, 3, 7), (7, 7, 11), (11, 11, 14), (16, 15, 16)
LEAF0, LEAF1, LEAF2 = (4, 8, 6), (7, 13, 8), (12, 18, 10)
WOOD0, WOOD1, WOOD2 = (12, 8, 5), (20, 14, 8), (26, 22, 15)
WATER0, WATER1, WATER2 = (2, 3, 8), (4, 7, 14), (9, 13, 20)

COBBLE = ["aabbaabc", "abbcabbc", "bccdbccd", "cddccddc",
          "baabbaab", "cabbcabb", "dbccdbcc", "ccddccdd"]


def cobbles(p, x0, y0, x1, y1):
    p.pattern(x0, y0, x1, y1, ["0110 0110"[0:8], "12211221", "11001100", "00000000",
                               "10011001", "22112211", "00110011", "00000000"],
              {"0": SETT0, "1": SETT2, "2": SETT3})


def lamp(p, x, top, bottom):
    """A cast-iron street lamp: lantern at top, post down to bottom."""
    p.rect(x, top + 8, x + 1, bottom, LAMP_PAL[1])
    p.rect(x - 2, bottom - 1, x + 3, bottom, LAMP_PAL[1])
    p.poly([(x - 3, top + 1), (x + 4, top + 1), (x + 3, top + 7), (x - 2, top + 7)], LAMP_PAL[2])
    p.rect(x - 1, top + 2, x + 2, top + 5, LAMP_PAL[3])
    p.poly([(x - 4, top + 1), (x + 0.5, top - 2), (x + 5, top + 1)], LAMP_PAL[1])


def pointed(p, x0, x1, top, bottom, c):
    """A pointed (gothic) arch opening filled with colour c."""
    mid = (x0 + x1) / 2
    p.rect(x0, top + (x1 - x0) // 2, x1, bottom, c)
    p.poly([(x0, top + (x1 - x0) // 2), (mid, top), (x1, top + (x1 - x0) // 2)], c)


# ---------------------------------------------------------------------------
# Cité: the square in front of Notre-Dame, at night.
# Facade simplified: two flat-topped towers, three portals, a row of kings,
# the rose window, the open gallery; the spire behind (it stood until 2019).
# ---------------------------------------------------------------------------
@scene("notredame")
def notredame():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],            # 0 sky
        [INK, SH, STONE, STONEL],                 # 1 stone
        [SETT0, SETT1, SETT2, SETT3],             # 2 setts
        LAMP_PAL,                                 # 3 lamps
        [NIGHT, LEAF0, LEAF1, LEAF2],             # 4 trees
        TEXT_PAL,                                 # 5 text
        [INK, SH, AMBER, GLOW],                   # 6 lit glass
        [NIGHT, WOOD0, WOOD1, WOOD2],             # 7 wooden hoarding
    ]
    p = Painter(NIGHT)
    p.dither(0, 0, 159, 40, NIGHT, NIGHT2)
    p.dither(0, 41, 159, 100, NIGHT2, NIGHT3)
    for x, y in [(6, 4), (150, 9), (9, 70)]:
        p.dot(x, y, STAR)
    L0, L1, R0, R1 = 20, 60, 100, 140
    # spire behind the crossing
    p.rect(78, 8, 81, 22, NIGHT3)
    p.poly([(77, 10), (79.5, 0), (82, 10)], NIGHT3)
    p.dot(79, 0, STAR)
    # towers and centre block
    p.rect(L0, 8, L1, 104, STONE)
    p.rect(R0, 8, R1, 104, STONE)
    p.rect(L1, 22, R0, 104, STONE)
    for x0, x1 in ((L0, L1), (R0, R1)):            # tower tops and edges
        p.rect(x0, 6, x1, 8, STONEL)
        p.rect(x0, 9, x0 + 1, 104, STONEL)
        p.rect(x1 - 1, 9, x1, 104, SH)
        for bx in range(x0 + 6, x1 - 6, 14):      # belfry lancets, two per face
            pointed(p, bx, bx + 8, 10, 34, INK)
            p.rect(bx + 4, 18, bx + 4, 34, SH)
    # open gallery across the front (the "gallery of chimeras" level)
    p.rect(L0, 36, R1, 38, STONEL)
    for gx in range(L0 + 4, R1 - 2, 8):
        pointed(p, gx, gx + 4, 39, 46, INK)
    p.rect(L0, 47, R1, 48, SH)
    # rose window, lit faintly from inside
    p.ellipse(66, 52, 94, 80, STONEL)
    p.ellipse(68, 54, 92, 78, INK)
    p.ellipse(72, 58, 88, 74, AMBER)
    p.ellipse(76, 62, 84, 70, GLOW)
    for a, b in [((80, 54), (80, 78)), ((68, 66), (92, 66)), ((71, 57), (89, 75)), ((89, 57), (71, 75))]:
        p.line([a, b], INK)
    for x0, x1 in ((L0, L1), (R0, R1)):           # two lancets each side
        for bx in (x0 + 8, x0 + 24):
            pointed(p, bx, bx + 8, 52, 78, INK)
    # gallery of kings: a row of niches
    p.rect(L0, 82, R1, 83, STONEL)
    for kx in range(L0 + 1, R1 - 2, 4):
        p.rect(kx, 85, kx + 2, 91, SH)
        p.rect(kx + 1, 85, kx + 1, 86, STONEL)       # a king's head
    p.rect(L0, 92, R1, 93, STONEL)
    # pigeons asleep on the ledges, heads tucked in
    for px, py in ((26, 79), (31, 79), (104, 79), (52, 33), (122, 33), (127, 33)):
        pigeon(p, px, py)
    # three portals
    for cx, w in ((40, 26), (80, 30), (120, 26)):
        pointed(p, cx - w // 2, cx + w // 2, 94 - w // 2 + 4, 104, SH)
        pointed(p, cx - w // 2 + 3, cx + w // 2 - 3, 94 - w // 2 + 8, 104, INK)
    # the square: setts, a pale band of slabs, the hoarding of the dig
    p.rect(0, 104, 159, 143, SETT1)
    cobbles(p, 0, 106, 159, 143)
    p.rect(0, 104, 159, 105, SETT3)
    # trees on the right, by the Hôtel-Dieu side
    for tx in (146,):
        p.ellipse(tx - 12, 60, tx + 14, 100, LEAF0)
        p.ellipse(tx - 9, 62, tx + 10, 92, LEAF1)
        p.ellipse(tx - 5, 64, tx + 4, 80, LEAF2)
        p.rect(tx, 96, tx + 2, 112, LEAF0)
    lamp(p, 28, 108, 130)
    lamp(p, 130, 108, 130)
    mask = [
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "#..................#",
        "....................",
        "...#............#...",
        "....................",
        "....................",
    ]
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Shared pieces for the other scenes
# ---------------------------------------------------------------------------
import random  # noqa: E402

BRICK0, BRICK1 = (11, 5, 4), (18, 8, 6)
PLASTER = (19, 17, 14)
GREEN0, GREEN1, GREEN2 = (2, 7, 5), (4, 12, 8), (9, 18, 11)
RED0, RED1 = (12, 3, 4), (22, 6, 6)
CREAM = (27, 25, 19)


def sky(p, y1=100, stars=()):
    p.dither(0, 0, 159, y1 // 2, NIGHT, NIGHT2)
    p.dither(0, y1 // 2 + 1, 159, y1, NIGHT2, NIGHT3)
    for x, y in stars:
        p.dot(x, y, STAR)


def windows(p, x0, x1, y0, y1, frame, dark, lit, seed, w=5, h=7, gx=10, gy=14, lit_rate=0.3):
    """A grid of tall Paris windows; some lit (night)."""
    r = random.Random(seed)
    for y in range(y0, y1 - h + 1, gy):
        for x in range(x0, x1 - w + 1, gx):
            p.rect(x - 1, y - 1, x + w, y + h, frame)
            p.rect(x, y, x + w - 1, y + h - 1, lit if r.random() < lit_rate else dark)
            p.rect(x - 1, y + h + 1, x + w, y + h + 1, frame)    # sill / balcony bar


def house(p, x0, x1, top, bottom, wall, roof, seed, lit=AMBER):
    """An old Paris house: slate roof with dormers, window grid, shop at foot."""
    p.rect(x0, top, x1, bottom, wall)
    p.poly([(x0 - 1, top), (x0 + 4, top - 8), (x1 - 4, top - 8), (x1 + 1, top)], roof)
    for dx in range(x0 + 6, x1 - 6, 14):                   # dormers
        p.rect(dx, top - 7, dx + 4, top - 2, INK)
        p.rect(dx + 1, top - 6, dx + 3, top - 3, lit if (dx + seed) % 3 == 0 else NIGHT)
    windows(p, x0 + 4, x1 - 2, top + 4, bottom - 18, SH, INK, lit, seed)


def tree(p, cx, top, bottom, trunk=None):
    p.ellipse(cx - 14, top, cx + 14, top + 30, LEAF0)
    p.ellipse(cx - 11, top + 2, cx + 9, top + 24, LEAF1)
    p.ellipse(cx - 6, top + 4, cx + 3, top + 14, LEAF2)
    p.rect(cx - 1, top + 26, cx + 1, bottom, trunk or LEAF0)


def pigeon(p, x, y):
    """A roosting pigeon, 6x4 px, top-left at (x, y); stone palette."""
    p.ellipse(x, y + 1, x + 5, y + 3, SH)
    p.ellipse(x + 3, y, x + 5, y + 2, SH)
    p.dot(x + 1, y + 1, STONEL)
    p.dot(x + 4, y + 1, INK)


def guimard(p, x, y):
    """A Guimard métro entrance seen from the front: green cast-iron railing,
    two stems curling up, each holding an orange lamp; the sign between."""
    p.rect(x, y + 12, x + 23, y + 23, GREEN0)                 # stairwell railing
    for i in range(x + 1, x + 23, 3):
        p.rect(i, y + 13, i, y + 22, GREEN1)
    p.rect(x, y + 12, x + 23, y + 12, GREEN2)
    for sx in (x, x + 23):                                    # the two stems
        p.line([(sx, y + 12), (sx, y + 2)], GREEN1, 2)
    p.rect(x + 2, y + 1, x + 21, y + 4, GREEN0)               # sign plate
    p.rect(x + 4, y + 2, x + 19, y + 3, CREAM)
    for sx in (x - 2, x + 22):                                # lamps
        p.ellipse(sx, y - 4, sx + 4, y + 1, AMBER)
        p.dot(sx + 2, y - 2, GLOW)


def river(p, y0, y1, seed=3):
    p.rect(0, y0, 159, y1, WATER0)
    r = random.Random(seed)
    for _ in range(70):
        x, y = r.randrange(0, 156), r.randrange(y0 + 1, y1)
        p.rect(x, y, x + r.randrange(2, 6), y, WATER1 if r.random() < 0.7 else WATER2)


def ground(p, y0, light=False):
    p.rect(0, y0, 159, 143, SETT1)
    cobbles(p, 0, y0 + 2, 159, 143)
    p.rect(0, y0, 159, y0 + 1, SETT3 if light else SETT2)


def mask_rows(top, solid_rows=12, extra=None):
    """Rows 0..solid_rows-1 solid, the rest walkable; extra[(row)] = string."""
    m = ["#" * 20] * solid_rows + ["." * 20] * (18 - solid_rows)
    for k, v in (extra or {}).items():
        m[k] = v
    return m


# ---------------------------------------------------------------------------
# Cité: Place Louis-Lépine, the flower market closed for the night, the métro.
# ---------------------------------------------------------------------------
@scene("fleurs")
def fleurs():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, STONE, AMBER],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [NIGHT, LEAF0, LEAF1, LEAF2],
        TEXT_PAL,
        [INK, GREEN0, GREEN1, GREEN2],          # iron of the stalls, the métro
        [GREEN0, AMBER, GLOW, CREAM],           # métro lamps and sign
    ]
    p = Painter(NIGHT)
    sky(p, 100, [(10, 6), (70, 4), (120, 12), (150, 30), (40, 22)])
    # the Préfecture behind (a long stone front)
    p.rect(0, 20, 159, 60, STONE)
    p.rect(0, 18, 159, 20, SH)
    windows(p, 3, 158, 26, 58, SH, INK, AMBER, 11, gx=9, lit_rate=0.1)
    # trees of the square
    for cx in (60, 100, 140):
        tree(p, cx, 30, 64)
    # the closed flower stalls: green iron pavilions, shutters down
    for x0 in (56, 92, 128):
        p.rect(x0, 56, x0 + 30, 92, GREEN0)
        p.poly([(x0 - 3, 58), (x0 + 15, 46), (x0 + 33, 58)], GREEN1)
        p.rect(x0 - 3, 58, x0 + 33, 60, GREEN2)
        for sx in range(x0 + 2, x0 + 29, 3):
            p.rect(sx, 62, sx, 90, GREEN1)                  # shutter slats
        p.rect(x0, 90, x0 + 30, 92, INK)
    # a few pots left out by the stalls
    for x in (60, 70, 98, 132, 150):
        p.rect(x, 86, x + 4, 91, BRICK1)
        p.ellipse(x - 1, 81, x + 5, 87, LEAF1)
    ground(p, 92)
    guimard(p, 18, 68)
    lamp(p, 50, 64, 92)
    mask = mask_rows(0, 12)
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Cité: Place Dauphine, the triangle of old brick-and-stone houses, trees,
# a gravel ground for pétanque, a bench.
# ---------------------------------------------------------------------------
@scene("dauphine")
def dauphine():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, BRICK0, BRICK1, AMBER],          # brick fronts, lit windows
        [SETT0, (13, 11, 9), (18, 16, 12), (22, 20, 15)],   # gravel
        LAMP_PAL,
        [NIGHT, LEAF0, LEAF1, LEAF2],
        TEXT_PAL,
        [INK, SH, STONE, STONEL],              # stone quoins, roofs
        [NIGHT, WOOD0, WOOD1, WOOD2],          # the bench
    ]
    p = Painter(NIGHT)
    sky(p, 60, [(20, 5), (80, 8), (140, 4)])
    # houses meeting at the point of the triangle (narrow end in the middle)
    for x0, x1, top in ((0, 52, 22), (52, 108, 28), (108, 159, 22)):
        p.rect(x0, top, x1, 92, BRICK1)
        p.poly([(x0, top), (x0 + 3, top - 9), (x1 - 3, top - 9), (x1, top)], SH)
        p.rect(x0, top, x0 + 3, 92, STONE)                # stone quoins
        p.rect(x1 - 3, top, x1, 92, STONE)
        windows(p, x0 + 7, x1 - 4, top + 5, 84, STONE, INK, AMBER, x0, w=6, h=8, gx=12, gy=16, lit_rate=0.25)
    for cx in (22, 58, 102, 140):
        tree(p, cx, 40, 92)
    p.rect(0, 92, 159, 143, (13, 11, 9))
    p.pattern(0, 94, 159, 143, ["1   2   ", "     1  ", "  2     ", "      1 ",
                                "1     2 ", "   1    ", " 2     1", "    2   "],
              {"1": (18, 16, 12), "2": (22, 20, 15)})
    p.rect(0, 92, 159, 93, (22, 20, 15))
    # the bench (hotspot 15..18, row 12)
    p.rect(120, 92, 151, 94, WOOD2)
    p.rect(120, 96, 151, 98, WOOD1)
    p.rect(122, 99, 123, 103, WOOD0)
    p.rect(148, 99, 149, 103, WOOD0)
    # the jack and two balls left on the gravel
    p.dot(76, 120, (31, 31, 31))
    p.ellipse(68, 124, 71, 127, SH)
    p.ellipse(84, 118, 87, 121, SH)
    lamp(p, 8, 70, 100)
    mask = mask_rows(0, 12, {12: "...............####."})
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Saint-Michel: the fountain (1860): Saint Michael and the devil in the
# central arch between columns, a winged dragon on each side, the basin.
# ---------------------------------------------------------------------------
@scene("stmichel")
def stmichel():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, STONE, STONEL],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [NIGHT, (6, 8, 9), (10, 14, 14), (16, 20, 18)],   # bronze dragons
        TEXT_PAL,
        [INK, RED0, RED1, STONEL],             # red columns
        [INK, GREEN0, GREEN1, GREEN2],         # métro railing
    ]
    p = Painter(NIGHT)
    sky(p, 40, [(6, 4), (150, 6)])
    # the building whose wall carries the fountain
    p.rect(0, 0, 159, 96, (12, 11, 10))
    windows(p, 100, 158, 6, 64, SH, INK, AMBER, 21, gx=11, lit_rate=0.35)
    windows(p, 2, 30, 6, 64, SH, INK, AMBER, 22, gx=11, lit_rate=0.35)
    # fountain wall: cols 5-10 (x 40-87)
    p.rect(36, 0, 92, 96, STONE)
    p.rect(36, 0, 92, 3, STONEL)
    for cx in (40, 84):                                    # red columns
        p.rect(cx, 8, cx + 4, 86, RED1)
        p.rect(cx + 3, 8, cx + 4, 86, RED0)
        p.rect(cx - 1, 6, cx + 5, 8, STONEL)
    pointed(p, 50, 78, 10, 72, SH)                         # the arch (round-ish)
    p.ellipse(50, 10, 78, 40, SH)
    # Saint Michael: wings up, sword raised; the devil at his feet
    p.ellipse(60, 30, 68, 38, STONEL)                      # head
    p.rect(59, 38, 69, 58, STONEL)                         # body
    p.poly([(59, 40), (50, 22), (57, 42)], STONEL)         # wings
    p.poly([(69, 40), (78, 22), (71, 42)], STONEL)
    p.line([(70, 38), (76, 14)], STONEL)                   # sword
    p.ellipse(56, 58, 74, 70, STONE)                       # the devil, crouched
    # the rocks and water falling into the basin
    p.rect(44, 72, 84, 84, SH)
    for x in range(48, 82, 4):
        p.line([(x, 74), (x, 88)], WATER2)
    p.rect(30, 86, 98, 96, STONEL)                         # basin rim
    p.rect(32, 88, 96, 94, WATER1)
    # winged dragons on pedestals (cols 3-4 and 11-12, rows 9-11)
    for x0, flip in ((24, 1), (90, -1)):
        p.rect(x0, 84, x0 + 14, 96, STONE)
        cx = x0 + 7
        p.ellipse(cx - 6, 74, cx + 6, 84, (10, 14, 14))
        p.poly([(cx, 76), (cx + 9 * flip, 64), (cx + 3 * flip, 78)], (16, 20, 18))
        p.ellipse(cx + 4 * flip - 2, 70, cx + 4 * flip + 2, 75, (6, 8, 9))
    # métro Saint-Michel at the right
    guimard(p, 124, 76)
    ground(p, 96, light=True)
    lamp(p, 112, 66, 98)
    mask = mask_rows(0, 12)
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Saint-Michel: rue de la Huchette, narrow and lit, the cellar door.
# ---------------------------------------------------------------------------
@scene("huchette")
def huchette():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, PLASTER, AMBER],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [INK, RED0, RED1, GLOW],                # shopfronts
        TEXT_PAL,
        [INK, WOOD0, WOOD1, AMBER],            # the cellar door
        [NIGHT, (14, 10, 6), (20, 16, 10), (26, 22, 15)],   # timber
    ]
    p = Painter(NIGHT)
    p.rect(0, 0, 159, 6, NIGHT2)                           # a slot of sky
    for x0, x1, wall, seed in ((0, 40, PLASTER, 1), (40, 72, (20, 16, 10), 2),
                               (72, 104, PLASTER, 3), (104, 159, (20, 16, 10), 4)):
        p.rect(x0, 4, x1, 96, wall)
        p.rect(x1 - 1, 4, x1, 96, SH)
        windows(p, x0 + 5, x1 - 3, 8, 62, SH, INK, AMBER, seed, gx=10, lit_rate=0.45)
    # shopfronts: a Greek grill, a bookshop, shutters
    p.rect(0, 70, 38, 94, RED0)
    p.rect(4, 74, 34, 90, GLOW)
    p.rect(4, 74, 34, 76, RED1)
    p.rect(110, 70, 156, 94, RED0)
    p.rect(114, 74, 152, 90, AMBER)
    # the cellar door (tiles 9-10, rows 10-11) with a lamp over it
    p.rect(70, 72, 90, 96, SH)
    p.rect(72, 76, 88, 96, WOOD1)
    p.rect(72, 76, 73, 96, WOOD0)
    p.rect(87, 76, 88, 96, WOOD0)
    p.ellipse(77, 82, 83, 88, AMBER)                       # round window
    p.rect(68, 66, 92, 70, WOOD0)                          # sign board (no name)
    p.rect(70, 67, 90, 69, AMBER)
    ground(p, 96)
    # the doorstep where a cat sleeps (cols 15-17, rows 13-14)
    p.rect(120, 104, 143, 112, STONE)
    p.ellipse(126, 100, 138, 107, (8, 7, 10))              # the cat
    p.ellipse(134, 98, 139, 103, (8, 7, 10))
    p.rect(0, 138, 119, 140, SETT0)                        # the gutter, water running
    for x in range(4, 118, 9):
        p.rect(x, 139, x + 3, 139, SETT3)
    lamp(p, 52, 62, 96)
    mask = mask_rows(0, 12, {13: "...............###..", 14: "...............###.."})
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# The jazz cellar: medieval stone vaults, a small stage, a crowd.
# ---------------------------------------------------------------------------
@scene("caveau")
def caveau():
    pals = [
        [INK, SH, STONE, STONEL],              # vaults
        [INK, (12, 8, 6), (20, 13, 8), AMBER],  # people in the warm light
        [INK, WOOD0, WOOD1, WOOD2],            # floor boards
        LAMP_PAL,
        [INK, (20, 18, 8), (28, 25, 12), GLOW],  # brass, the drum kit
        TEXT_PAL,
        [INK, RED0, RED1, AMBER],              # the stage curtain
        [INK, (8, 6, 8), (14, 10, 12), (20, 15, 16)],   # crowd silhouettes
    ]
    p = Painter(INK)
    p.rect(0, 0, 159, 143, SH)
    for cx in (20, 80, 140):                               # three vaults
        p.ellipse(cx - 40, -30, cx + 40, 50, STONE)
        p.ellipse(cx - 34, -24, cx + 34, 44, SH)
    r = random.Random(9)
    for _ in range(120):                                   # stones of the walls
        x, y = r.randrange(160), r.randrange(0, 40)
        p.rect(x, y, x + 5, y + 2, STONE if r.random() < 0.6 else STONEL)
    # the stage (rows 3-6) with a red curtain
    p.rect(40, 14, 120, 40, RED0)
    for x in range(42, 120, 6):
        p.rect(x, 14, x + 2, 40, RED1)
    p.rect(36, 40, 124, 46, WOOD2)
    p.rect(36, 46, 124, 48, WOOD0)
    # crowd along the sides (rows 4-9), dancing heads and shoulders
    for x in range(0, 160, 9):
        if 32 < x < 128:
            continue
        y = 40 + (x * 7) % 18
        p.ellipse(x, y, x + 6, y + 6, (14, 10, 12))
        p.rect(x - 1, y + 6, x + 7, y + 20, (8, 6, 8))
    # floor
    p.rect(0, 56, 159, 143, WOOD1)
    for y in range(58, 144, 6):
        p.rect(0, y, 159, y, WOOD0)
    # small tables with candles
    for tx, ty in ((14, 70), (140, 70), (12, 118)):
        p.ellipse(tx - 6, ty, tx + 6, ty + 4, WOOD2)
        p.rect(tx - 1, ty + 4, tx + 1, ty + 12, WOOD0)
        p.rect(tx, ty - 4, tx, ty - 1, AMBER)
    # the drum kit, bottom right (cols 17-19, rows 13-15)
    p.ellipse(138, 104, 158, 120, (20, 18, 8))
    p.ellipse(141, 107, 155, 117, (28, 25, 12))
    p.ellipse(136, 98, 146, 102, GLOW)                     # cymbal
    # stairs up (bottom middle)
    for i, y in enumerate(range(136, 144, 2)):
        p.rect(64 - i, y, 96 + i, y, STONEL if i % 2 else STONE)
    mask = [
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####....####....####",
        "####............####",
        "####............####",
        "##..............####",
        ".................###",
        ".#................##",
        "..................##",
        ".................###",
        ".#...............###",
        "..................##",
        "....................",
        "####........########",
    ]
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Under the Saint-Michel bridge: the low quay, the stone arch, the Seine.
# ---------------------------------------------------------------------------
@scene("basport")
def basport():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, STONE, STONEL],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [WATER0, WATER1, WATER2, (20, 22, 26)],   # the Seine, the moon in it
        TEXT_PAL,
        [INK, (6, 5, 9), (10, 9, 13), AMBER],  # under the arch, a fire
        [NIGHT, (10, 6, 6), (16, 9, 8), (22, 12, 10)],   # blankets
    ]
    p = Painter(NIGHT)
    sky(p, 70, [(12, 8), (140, 10), (60, 4)])
    # the bridge: a stone arch across the top half
    p.rect(0, 0, 159, 22, STONE)
    p.rect(0, 20, 159, 22, STONEL)
    p.ellipse(-20, 18, 180, 140, NIGHT)                    # the opening
    p.rect(0, 0, 159, 14, STONE)
    for x in range(0, 160, 10):                            # voussoirs
        p.rect(x, 14, x + 1, 22, SH)
    p.ellipse(112, 36, 121, 45, (26, 26, 22))               # the moon
    # Notre-Dame far off, across the water (towers only)
    p.rect(10, 44, 20, 66, NIGHT3)
    p.rect(26, 44, 36, 66, NIGHT3)
    p.rect(20, 52, 26, 66, NIGHT3)
    # far quay and water
    river(p, 66, 92)
    p.rect(0, 64, 159, 66, SH)
    # the low quay (rows 12-14) and the near water (rows 15-17)
    p.rect(0, 92, 159, 120, STONE)
    p.rect(0, 92, 159, 93, STONEL)
    for x in range(0, 160, 12):
        p.rect(x, 94, x, 118, SH)
    p.rect(0, 118, 159, 120, STONEL)                       # the edge
    river(p, 121, 143, seed=7)
    p.ellipse(60, 130, 70, 133, (20, 22, 26))              # the moon on the water
    # the stairs back up (cols 16-19, rows 9-12)
    for i, y in enumerate(range(72, 96, 4)):
        p.rect(128 + i * 4, y, 159, y + 3, STONEL if i % 2 else STONE)
    # a little fire in a can by the sleepers
    p.rect(50, 104, 55, 111, SH)
    p.ellipse(50, 98, 55, 104, AMBER)
    mask = [
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "################....",
        "......#.............",
        "....................",
        "....................",
        "####################",
        "####################",
        "####################",
    ]
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# Saint-Germain-des-Prés: the Romanesque bell tower and its spire, the métro,
# a bookshop still lit.
# ---------------------------------------------------------------------------
@scene("stgermain")
def stgermain():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, STONE, STONEL],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [NIGHT, LEAF0, LEAF1, LEAF2],
        TEXT_PAL,
        [INK, GREEN0, GREEN1, CREAM],          # métro
        [INK, WOOD0, AMBER, GLOW],             # bookshop window
    ]
    p = Painter(NIGHT)
    sky(p, 100, [(8, 30), (100, 6), (150, 20), (130, 50)])
    # the bell tower (square, round-arched openings) and its spire
    p.rect(40, 24, 76, 96, STONE)
    p.rect(40, 24, 42, 96, STONEL)
    p.rect(74, 24, 76, 96, SH)
    p.poly([(38, 24), (58, 0), (78, 24)], SH)              # slate spire
    p.line([(58, 0), (58, 24)], (10, 10, 14))
    for y in (30, 52):                                     # two levels of twin arches
        for x in (46, 60):
            p.rect(x, y + 4, x + 8, y + 16, INK)
            p.ellipse(x, y, x + 8, y + 8, INK)
            p.rect(x + 4, y + 2, x + 4, y + 16, STONE)
    p.rect(40, 48, 76, 49, STONEL)
    p.rect(40, 70, 76, 71, STONEL)
    # nave to the left, buttresses
    p.rect(0, 50, 40, 96, STONE)
    p.poly([(0, 50), (0, 40), (40, 40), (40, 50)], SH)
    for x in (6, 22):
        p.rect(x, 60, x + 6, 78, INK)
        p.ellipse(x, 56, x + 6, 62, INK)
    # the church door at the foot of the tower
    p.rect(52, 78, 64, 96, INK)
    p.ellipse(52, 72, 64, 84, INK)
    # houses on the right with the bookshop (cols 14-19, rows 8-11)
    p.rect(80, 14, 159, 96, (14, 12, 12))
    windows(p, 116, 156, 18, 60, SH, INK, AMBER, 31, gx=10, lit_rate=0.4)
    windows(p, 84, 108, 18, 60, SH, INK, AMBER, 32, gx=10, lit_rate=0.2)
    p.rect(112, 64, 159, 96, WOOD0)
    p.rect(116, 68, 156, 92, AMBER)
    for x in range(118, 156, 4):                           # books in the window
        p.rect(x, 80, x + 2, 92, (12, 8, 5) if x % 8 else GLOW)
    p.rect(116, 78, 156, 79, WOOD0)
    # the shop cat, sitting among the books
    p.ellipse(136, 84, 145, 92, INK)
    p.ellipse(137, 78, 143, 85, INK)
    p.poly([(137, 80), (137, 76), (139, 79)], INK)
    p.poly([(143, 80), (143, 76), (141, 79)], INK)
    p.line([(145, 91), (149, 91), (150, 88)], INK)
    # trees in the little square
    tree(p, 96, 54, 96)
    ground(p, 96)
    guimard(p, 18, 74)
    p.ellipse(50, 124, 54, 127, SETT0)                     # watch your step
    lamp(p, 104, 66, 98)
    mask = mask_rows(0, 12)
    return pals, p.img, mask


# ---------------------------------------------------------------------------
# A café terrace on the boulevard: awning, wicker chairs, small round tables.
# (No real café's name or sign.)
# ---------------------------------------------------------------------------
@scene("terrasse")
def terrasse():
    pals = [
        [NIGHT, NIGHT2, NIGHT3, STAR],
        [INK, SH, PLASTER, AMBER],
        [SETT0, SETT1, SETT2, SETT3],
        LAMP_PAL,
        [INK, RED0, RED1, CREAM],              # the awning
        TEXT_PAL,
        [INK, (14, 10, 6), (22, 17, 10), GLOW],   # wicker, the lit room
        [NIGHT, (14, 14, 16), (20, 20, 22), (27, 27, 28)],   # marble tables
    ]
    p = Painter(NIGHT)
    sky(p, 24, [(40, 4), (120, 8)])
    p.rect(0, 10, 159, 96, PLASTER)
    windows(p, 6, 156, 14, 44, SH, INK, AMBER, 41, gx=12, lit_rate=0.4)
    # the awning, striped, over the whole front
    p.poly([(0, 48), (159, 48), (159, 62), (0, 62)], RED0)
    for x in range(0, 160, 10):
        p.rect(x, 48, x + 4, 62, RED1)
    for x in range(0, 160, 5):                             # scalloped edge
        p.ellipse(x, 60, x + 5, 65, CREAM if (x // 5) % 2 else RED1)
    # the lit room behind the glass
    p.rect(0, 66, 159, 96, GLOW)
    for x in range(0, 160, 20):
        p.rect(x, 66, x + 1, 96, (14, 10, 6))
    p.rect(0, 80, 159, 81, (14, 10, 6))
    # across the street: a tabac's red "carotte" (shape reconstructed)
    p.poly([(150, 4), (154, 10), (150, 22), (146, 10)], RED1)
    p.line([(150, 6), (150, 20)], RED0)
    # a wall telephone on the left (col 1, rows 9-10)
    p.rect(6, 72, 14, 86, INK)
    p.rect(8, 74, 12, 78, SH)
    p.line([(7, 80), (7, 88)], INK)
    # the pavement terrace
    p.rect(0, 96, 159, 143, SETT2)
    for x in range(0, 160, 16):
        p.rect(x, 96, x, 143, SETT1)
    for y in range(96, 144, 12):
        p.rect(0, y, 159, y, SETT1)
    # round marble tables with wicker chairs; (9-10, row 13) has initials
    for tx, ty in ((24, 108), (76, 106), (120, 116), (40, 132), (100, 134)):
        p.ellipse(tx - 6, ty, tx + 6, ty + 4, (27, 27, 28))
        p.rect(tx - 1, ty + 4, tx, ty + 10, (14, 14, 16))
        for cx in (tx - 11, tx + 8):
            p.rect(cx, ty + 2, cx + 3, ty + 10, (22, 17, 10))
            p.rect(cx, ty - 2, cx + 3, ty + 2, (14, 10, 6))
    mask = [
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "####################",
        "....................",
        "....................",
        "..##.....##.........",
        "..............##....",
        "....................",
        ".....##.....##......",
        "....................",
    ]
    return pals, p.img, mask
