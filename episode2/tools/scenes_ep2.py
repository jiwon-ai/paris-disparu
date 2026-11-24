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
    for x, y in [(6, 4), (150, 9), (12, 30), (147, 36), (3, 60), (155, 70), (9, 82)]:
        p.dot(x, y, STAR)
    L0, L1, R0, R1 = 20, 60, 100, 140
    # spire behind the crossing
    p.rect(78, 2, 81, 22, SH)
    p.poly([(77, 14), (79.5, 0), (82, 14)], SH)
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
    for gx in range(L0 + 3, R1 - 2, 6):
        pointed(p, gx, gx + 3, 39, 46, INK)
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
    for kx in range(L0 + 2, R1 - 2, 5):
        p.rect(kx, 85, kx + 2, 91, SH)
        p.rect(kx + 1, 85, kx + 1, 86, STONEL)       # a king's head
    p.rect(L0, 92, R1, 93, STONEL)
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
    p.ellipse(-12, 60, 14, 100, LEAF0)
    p.ellipse(-9, 62, 10, 92, LEAF1)
    p.rect(2, 96, 4, 112, LEAF0)
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


# Placeholders until each room is painted.
for _n in ("fleurs", "dauphine", "stmichel", "huchette", "caveau", "basport", "stgermain", "terrasse"):
    from scenes import SCENES as _S
    if _n not in _S:
        _S[_n] = notredame
