"""Dialogue portraits (24x32 busts), drawn in code. Original art.

Each portrait uses the speaker's sprite palette: '.' transparent, '#' colour 1
(outline / dark), 'o' colour 2 (clothing), 's' colour 3 (skin / light).
Shapes are filled first; a dark outline is then added around them.
"""

W, H = 24, 32


def blank():
    return [["." for _ in range(W)] for _ in range(H)]


def ellipse(g, cx, cy, rx, ry, ch):
    for y in range(H):
        for x in range(W):
            if ((x + 0.5 - cx) / rx) ** 2 + ((y + 0.5 - cy) / ry) ** 2 <= 1:
                g[y][x] = ch


def rect(g, x0, y0, x1, y1, ch):
    for y in range(max(0, y0), min(H, y1 + 1)):
        for x in range(max(0, x0), min(W, x1 + 1)):
            g[y][x] = ch


def outline(g):
    out = [row[:] for row in g]
    for y in range(H):
        for x in range(W):
            if g[y][x] != ".":
                continue
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                nx, ny = x + dx, y + dy
                if 0 <= nx < W and 0 <= ny < H and g[ny][nx] not in ".#":
                    out[y][x] = "#"
                    break
    return out


def face(g, cy=15, eyes_y=None):
    """Head, eyes, nose shadow and mouth: shared by every bust."""
    ellipse(g, 12, cy, 5.5, 6.5, "s")
    ey = eyes_y if eyes_y is not None else cy - 1
    g[ey][9] = g[ey][14] = "#"
    g[cy + 3][11] = g[cy + 3][12] = "#"     # mouth
    g[cy + 1][12] = "#"                      # nose shadow


def shoulders(g, ch="o", top=24):
    ellipse(g, 12, top + 10, 11.5, 9, ch)
    rect(g, 9, top - 2, 14, top, "s")      # neck


def simone():
    g = blank()
    shoulders(g, "o")                        # white shirt
    rect(g, 2, 26, 7, 31, "#")               # black waistcoat
    rect(g, 16, 26, 21, 31, "#")
    face(g)
    ellipse(g, 12, 9, 6.5, 4, "#")           # dark hair, pinned up
    ellipse(g, 12, 4, 3, 3, "#")             # bun
    rect(g, 6, 9, 7, 15, "#")
    rect(g, 16, 9, 17, 15, "#")
    return outline(g)


def fernand():
    g = blank()
    shoulders(g, "o")                        # blue blouse, no plate tonight
    face(g)
    ellipse(g, 12, 9.5, 6, 3, "#")           # short hair, bareheaded
    for x in (8, 10, 13, 15):                # tired lines
        g[17][x] = "s"
    g[13][8] = g[13][9] = g[13][14] = g[13][15] = "#"   # heavy brows
    return outline(g)


def dede():
    g = blank()
    shoulders(g, "o")
    rect(g, 10, 27, 13, 29, "s")             # his fort's plate
    face(g, cy=16)
    ellipse(g, 12, 9, 11.5, 2.2, "s")        # the coltin: very wide brim
    ellipse(g, 12, 6, 5, 3.5, "s")           # crown
    rect(g, 1, 9, 22, 9, "#")                # brim edge
    return outline(g)


def germaine():
    g = blank()
    shoulders(g, "o")                        # red shawl
    face(g, cy=16)
    ellipse(g, 12, 10, 7.5, 5, "o")          # headscarf
    rect(g, 4, 10, 6, 18, "o")
    rect(g, 17, 10, 19, 18, "o")
    g[22][12] = "o"                          # knot
    return outline(g)


def stranger():
    g = blank()
    shoulders(g, "o")                        # grey overcoat
    rect(g, 10, 24, 13, 31, "s")             # scarf
    face(g, cy=16)
    ellipse(g, 12, 9, 8.5, 2, "o")           # felt hat brim
    ellipse(g, 12, 6, 5, 3.5, "o")
    rect(g, 7, 8, 16, 8, "#")                # hat band
    rect(g, 7, 15, 10, 15, "#")              # round glasses
    rect(g, 13, 15, 16, 15, "#")
    return outline(g)


def painter():
    g = blank()
    shoulders(g, "o")                        # red smock
    rect(g, 11, 24, 12, 31, "#")             # loose collar
    face(g, cy=16)
    ellipse(g, 11, 9, 7, 3, "#")             # beret
    g[6][12] = "#"                           # its stalk
    rect(g, 9, 20, 14, 21, "#")              # moustache
    return outline(g)


def photographer():
    g = blank()
    shoulders(g, "o")                        # dark jacket
    rect(g, 7, 26, 16, 31, "#")              # camera at the chest
    ellipse(g, 11.5, 29, 2.5, 2.5, "s")      # lens
    face(g, cy=16)
    ellipse(g, 12, 9.5, 6.5, 2.5, "o")       # flat cap
    rect(g, 12, 10, 19, 11, "o")             # its peak
    return outline(g)


def puncher():
    g = blank()
    shoulders(g, "o")                        # uniform jacket
    for y in (26, 29):                       # buttons
        g[y][12] = "#"
    face(g, cy=16)
    ellipse(g, 12, 9, 6.5, 3, "#")           # peaked cap
    rect(g, 5, 11, 18, 11, "#")              # its peak
    rect(g, 10, 8, 13, 8, "s")               # cap badge
    return outline(g)


def singer():
    g = blank()
    shoulders(g, "o")                        # dark jacket
    rect(g, 10, 24, 13, 31, "s")             # open white shirt
    face(g, cy=16)
    ellipse(g, 12, 9.5, 7, 4, "#")           # tousled dark hair
    for x in (5, 8, 15, 18):
        g[6][x] = "#"
    for x in (9, 11, 13, 15):                # stubble
        g[20][x] = "#"
    return outline(g)


def actor():
    g = blank()
    shoulders(g, "o")                        # jacket
    rect(g, 9, 24, 14, 31, "s")              # open shirt collar
    face(g, cy=16)
    ellipse(g, 12, 9.5, 7, 3.5, "#")         # tousled hair
    for x in (6, 9, 14, 17):
        g[6][x] = "#"
    g[19][12] = g[18][12] = "#"              # strong nose
    return outline(g)


def chanteuse():
    g = blank()
    shoulders(g, "#")                        # dark dress
    rect(g, 10, 22, 13, 25, "s")             # neck
    rect(g, 3, 10, 7, 27, "#")               # long dark hair
    rect(g, 16, 10, 20, 27, "#")
    face(g, cy=16)
    ellipse(g, 12, 9, 7.5, 4.5, "#")
    rect(g, 7, 10, 16, 11, "#")              # fringe
    g[15][8] = g[15][15] = "#"               # dark eye make-up
    return outline(g)


def philosopher():
    g = blank()
    shoulders(g, "o")                        # jacket
    rect(g, 11, 24, 12, 31, "#")             # tie
    face(g, cy=16)
    ellipse(g, 12, 9.5, 6.5, 2.5, "#")       # thin, combed-back hair
    rect(g, 7, 14, 10, 16, "#")              # thick round glasses
    rect(g, 13, 14, 16, 16, "#")
    g[15][8] = g[15][15] = "s"
    return outline(g)


def filmmaker():
    g = blank()
    shoulders(g, "o")                        # coat
    rect(g, 11, 24, 12, 31, "#")
    face(g, cy=17)
    ellipse(g, 12, 9, 9, 2, "o")             # hat brim
    ellipse(g, 12, 6, 5, 3.5, "o")
    rect(g, 7, 8, 16, 8, "#")                # hat band
    return outline(g)


def writer():
    g = blank()
    shoulders(g, "o")                        # suit
    rect(g, 10, 24, 13, 25, "#")             # bow tie
    face(g, cy=16)
    ellipse(g, 12, 10, 6.5, 3, "#")          # receding hair
    rect(g, 7, 15, 10, 15, "#")              # glasses
    rect(g, 13, 15, 16, 15, "#")
    rect(g, 10, 15, 13, 15, "#")
    return outline(g)


# Order = portrait index used by the game; palette = sprite palette index.
PORTRAITS = [
    ("SIMONE", simone(), 3),
    ("FERNAND", fernand(), 1),
    ("DÉDÉ", dede(), 1),
    ("GERMAINE", germaine(), 2),
    ("MAN WITH NOTEBOOK", stranger(), 4),
    ("THE PAINTER", painter(), 2),
    ("THE PHOTOGRAPHER", photographer(), 4),
    ("TICKET PUNCHER", puncher(), 4),
    ("THE SINGER", singer(), 4),
    ("THE ACTOR", actor(), 4),
    ("THE CHANTEUSE", chanteuse(), 3),
    ("THE PHILOSOPHER", philosopher(), 4),
    ("THE FILMMAKER", filmmaker(), 0),
    ("THE WRITER", writer(), 4),
]
# Other speakers who reuse a portrait.
ALIASES = {"A PORTER": "DÉDÉ"}


def rows(grid):
    return ["".join(r) for r in grid]


if __name__ == "__main__":
    for name, g, _ in PORTRAITS:
        print(name)
        print("\n".join(rows(g)))
