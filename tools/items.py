"""Item pictures (24x32, six 8x16 sprites), drawn in code. Original art.

Shown above a caption when an object is obtained, and in the coffee game.
Same characters as the portraits: '.' transparent, '#' colour 1 (dark),
'o' colour 2, 's' colour 3 of the item's sprite palette (tools/art.py).
Each item says what it rests on in docs/HISTORICAL_RESEARCH.md ("Item
pictures").
"""

from portraits import blank, ellipse, rect, outline, rows  # noqa: F401


def hline(g, x0, x1, y, ch="#"):
    for x in range(x0, x1 + 1):
        g[y][x] = ch


def vline(g, x, y0, y1, ch="#"):
    for y in range(y0, y1 + 1):
        g[y][x] = ch


def cup():
    """Café cup of black coffee on its saucer (palette 3: white, tan)."""
    g = blank()
    ellipse(g, 12, 27, 11, 3, "o")          # saucer
    rect(g, 5, 15, 18, 25, "o")             # cup body
    ellipse(g, 12, 25, 7, 2.5, "o")         # rounded bottom
    ellipse(g, 12, 15, 7, 2, "#")           # dark coffee seen from above
    ellipse(g, 12, 15, 5, 1, "s")           # light crema
    ellipse(g, 20, 20, 3, 3.5, "o")         # handle
    ellipse(g, 20, 20, 1.5, 2, ".")
    for y, x in ((6, 9), (8, 10), (10, 9), (4, 14), (6, 15), (8, 14)):
        g[y][x] = "s"                        # steam
    return outline(g)


def coltin():
    """The fort's coltin: yellow leather hat with very wide brims (palette 1)."""
    g = blank()
    ellipse(g, 12, 22, 11.5, 6, "s")        # wide brim
    ellipse(g, 12, 15, 6, 7, "s")           # crown
    rect(g, 6, 15, 17, 20, "s")
    hline(g, 6, 17, 19, "o")                # band
    hline(g, 6, 17, 20, "o")
    for x in (3, 7, 12, 17, 21):            # stitching on the brim
        g[24][x] = "#"
    return outline(g)


def bleu():
    """A petit bleu: folded blue pneumatic-post card (palette 1)."""
    g = blank()
    rect(g, 1, 9, 22, 26, "o")
    for i in range(11):                      # the flap's fold, a wide V
        g[9 + i // 2 + i % 2][1 + i] = "#"
        g[9 + i // 2 + i % 2][22 - i] = "#"
    hline(g, 4, 12, 21, "s")                 # address lines
    hline(g, 4, 15, 23, "s")
    rect(g, 17, 18, 20, 22, "s")             # stamp
    return outline(g)


def plate():
    """The fort's metal plate: arms of Paris (a ship) and his name (palette 1)."""
    g = blank()
    ellipse(g, 12, 17, 10, 11, "s")
    ellipse(g, 12, 17, 8, 9, "o")            # inner field
    rect(g, 7, 17, 17, 19, "s")              # the ship's hull
    ellipse(g, 12, 19, 5, 1.5, "s")
    vline(g, 12, 10, 16, "s")                # mast
    rect(g, 13, 11, 15, 15, "s")             # sail
    hline(g, 7, 17, 23, "#")                 # name, too small to read
    hline(g, 8, 16, 25, "#")
    rect(g, 11, 4, 13, 6, "s")               # ring to pin it on
    return outline(g)


def gratinee():
    """Onion soup gratinée in its bowl, cheese crust on top (palette 7)."""
    g = blank()
    ellipse(g, 12, 18, 11, 4, "o")           # browned crust
    ellipse(g, 12, 23, 10, 6, "s")           # bowl
    rect(g, 2, 18, 21, 22, "s")
    ellipse(g, 12, 17, 10, 3, "o")
    for y, x in ((16, 6), (17, 10), (16, 15), (18, 18), (18, 4), (17, 13)):
        g[y][x] = "#"                        # toasted spots
    ellipse(g, 0.5, 21, 1.5, 2, "s")         # lugs
    ellipse(g, 23.5, 21, 1.5, 2, "s")
    for y, x in ((9, 8), (11, 9), (13, 8), (8, 15), (10, 16), (12, 15)):
        g[y][x] = "s"                        # steam
    return outline(g)


def jeton():
    """A PTT telephone token (palette 3): copper-nickel disc with a deep
    groove down the middle (Numista 16650); drawn simplified."""
    g = blank()
    ellipse(g, 12, 17, 10, 10, "o")
    ellipse(g, 12, 17, 8, 8, "s")            # raised rim, then the face
    ellipse(g, 12, 17, 7, 7, "o")
    rect(g, 10, 10, 14, 24, "#")             # the deep square groove (P.T.T)
    rect(g, 11, 11, 13, 23, "s")
    return outline(g)


def ticket(pal_body="o", mark=None):
    """A métro ticket: small card, printed band, punch hole."""
    g = blank()
    rect(g, 3, 10, 20, 24, pal_body)
    hline(g, 5, 18, 13, "#")                 # printed lines
    hline(g, 5, 13, 16, "#")
    hline(g, 5, 15, 19, "#")
    if mark:
        mark(g)
    return g


def ticket_brown():
    """Second-class ticket, tan (palette 0); colour: see research claim."""
    return outline(ticket("o"))


def ticket_green():
    """First-class ticket, green, with the class number (palette 5)."""
    def one(g):
        vline(g, 17, 16, 21, "s")            # a big "1"
        g[17][16] = "s"
    return outline(ticket("o", one))


def roller():
    """A VéloSolex roller: ridged metal cylinder on its axle (palette 4)."""
    g = blank()
    rect(g, 6, 9, 17, 25, "o")
    ellipse(g, 12, 9, 6, 2, "o")             # top face
    ellipse(g, 12, 25, 6, 2, "o")
    for y in range(12, 25, 3):               # ridges
        hline(g, 6, 17, y, "#")
    vline(g, 12, 4, 8, "#")                  # axle
    vline(g, 12, 27, 30, "#")
    return outline(g)


def stub():
    """Exhibition ticket stub with a gold mask (palette 1). Design is fiction."""
    g = blank()
    rect(g, 2, 6, 21, 28, "s")
    for y in range(6, 29, 2):                # torn edge
        g[y][2] = "."
    rect(g, 7, 9, 16, 21, "o")               # striped headdress
    for y in range(10, 21, 2):
        hline(g, 7, 16, y, "s")
    ellipse(g, 11.5, 16, 3.5, 4, "s")        # golden face
    g[15][10] = g[15][13] = "#"
    rect(g, 11, 20, 12, 23, "o")             # beard
    hline(g, 5, 18, 25, "#")                 # PETIT PALAIS
    return outline(g)


# HUD icons (8 px wide), drawn by hand: same characters as the pictures.
# Centred in an 8x16 sprite by icon(). Each follows its picture's sources.
ICONS = {
    "CUP": [".s..s...",
            "........",
            "######..",
            "#oooo###",
            "#oooo#.#",
            "#oooo###",
            ".####...",
            "########"],
    "COLTIN": ["..####..",
               ".#ssss#.",
               ".#ssss#.",
               ".#oooo#.",
               "#ssssss#",
               ".######."],
    "BLEU": ["########",             # the petit bleu: a folded, sealed card
             "##oooo##",
             "#o#oo#o#",
             "#oo##oo#",
             "#oooooo#",
             "########"],
    "PLATE": [".######.",            # brass plate, the ship of Paris
              "#ssssss#",
              "#sss#ss#",
              "#ss##ss#",
              "#s####s#",
              "#ss##ss#",
              ".#ssss#.",
              "..####.."],
    "GRATINEE": [".######.",
                 "#oooooo#",
                 "#o#oo#o#",
                 "########",
                 "#ssssss#",
                 ".#ssss#.",
                 "..####.."],
    "JETON": ["..####..",            # PTT token: disc, deep groove down the middle
              ".#oooo#.",
              "#oo##oo#",
              "#oo##oo#",
              "#oo##oo#",
              "#oo##oo#",
              ".#oooo#.",
              "..####.."],
    "TICKET": ["########",
               "#oooooo#",
               "#o###oo#",
               "#oooooo#",
               "#o##ooo#",
               "########"],
    "GREEN": ["########",
              "#oooooo#",
              "#o##oos#",
              "#ooooos#",
              "#o##oos#",
              "########"],
    "ROLLER": [".######.",           # ridged roller on its axle
               "##o#o#o#",
               "##o#o#o#",
               "##o#o#o#",
               "##o#o#o#",
               ".######."],
    "STUB": ["########",
             "#ssssss#",
             "#s####s#",
             "#ssooss#",
             "#sossos#",
             "#ssooss#",
             "#ssssss#",
             "########"],
}


def icon(name):
    """The 8x16 HUD icon of an item, its drawing centred vertically."""
    g = ICONS[name]
    assert all(len(r) == 8 for r in g), name
    top = (16 - len(g)) // 2
    return ["........"] * top + g + ["........"] * (16 - len(g) - top)

# (enum name, grid, sprite palette)
ITEMS = [
    ("CUP", cup(), 3),
    ("COLTIN", coltin(), 1),
    ("BLEU", bleu(), 1),
    ("PLATE", plate(), 1),
    ("GRATINEE", gratinee(), 7),
    ("JETON", jeton(), 3),   # copper-nickel: silvery (Numista 16650)
    ("TICKET", ticket_brown(), 0),
    ("GREEN", ticket_green(), 5),
    ("ROLLER", roller(), 4),
    ("STUB", stub(), 1),
]


if __name__ == "__main__":
    import sys
    import art
    try:
        from PIL import Image
    except ImportError:
        sys.exit("needs Pillow")
    sc = 4
    img = Image.new("RGB", (len(ITEMS) * 28 * sc, 36 * sc), (60, 70, 80))
    for k, (_, g, p) in enumerate(ITEMS):
        pal = [tuple(c * 255 // 31 for c in col) for col in art.SPR_PALETTES[p]]
        for y, r in enumerate(g):
            for x, ch in enumerate(r):
                if ch == ".":
                    continue
                col = pal[".#os".index(ch)]
                for dy in range(sc):
                    for dx in range(sc):
                        img.putpixel(((k * 28 + 2 + x) * sc + dx, (2 + y) * sc + dy), col)
    out = sys.argv[1] if len(sys.argv) > 1 else "items_preview.png"
    img.save(out)
    print(out)
