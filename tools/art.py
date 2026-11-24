"""Original pixel art for PARIS DISPARU, written as text so it can be diffed.

Background tiles are 8x8, digits are colour indices 0-3 within the tile's
CGB palette. Sprites are 16x16: '.' transparent, '#' = colour 1 (outline),
'o' = colour 2 (clothing), 's' = colour 3 (skin / light).
"""

# ---------------------------------------------------------------------------
# CGB palettes, RGB components 0-31.
# ---------------------------------------------------------------------------
STONE = (18, 16, 13)   # Paris limestone (cream-beige), seen at night

BG_PALETTES = [
    # 0 stone facade: night, shadow, stone, highlight
    [(4, 4, 9), (10, 9, 10), STONE, (24, 22, 17)],
    # 1 lit window: stone, frame, amber, warm light
    [STONE, (5, 5, 8), (29, 20, 7), (31, 28, 16)],
    # 2 street: gap, shade, setts/slabs, highlight
    [(4, 4, 8), (8, 8, 12), (12, 12, 15), (17, 16, 17)],
    # 3 street lamp: pavement, iron, glow, hot glow
    [(12, 12, 15), (3, 3, 6), (28, 23, 9), (31, 31, 22)],
    # 4 shopfront red: night, dark red, red, cream
    [(5, 5, 11), (12, 3, 4), (22, 6, 6), (29, 25, 20)],
    # 5 text box / painted signs: cream, cream, mid, ink
    [(29, 28, 24), (29, 28, 24), (17, 15, 13), (3, 3, 6)],
    # 6 produce and wood: dark, brown, green, orange wood
    [(4, 4, 8), (14, 8, 4), (8, 18, 6), (26, 17, 7)],
    # 7 zinc roofs and enamel plaques: sky, dark zinc / plaque blue, zinc,
    #   light zinc / plaque white
    [(4, 5, 11), (6, 8, 15), (11, 13, 18), (24, 25, 28)],
]

# Daybreak after the ending (about 8 a.m.): the same palettes in a pale,
# overcast November morning; lamps and shop lights off. Reconstruction.
BG_DAY = [
    [(25, 22, 22), (13, 12, 12), (24, 22, 17), (30, 28, 23)],
    [(24, 22, 17), (6, 6, 8), (13, 14, 17), (21, 23, 26)],
    [(10, 10, 12), (15, 15, 17), (20, 19, 19), (25, 24, 23)],
    [(20, 19, 19), (3, 3, 6), (17, 17, 15), (23, 23, 21)],
    [(25, 22, 22), (14, 4, 5), (25, 8, 7), (30, 27, 22)],
    [(29, 28, 24), (29, 28, 24), (17, 15, 13), (3, 3, 6)],
    [(14, 13, 14), (17, 10, 5), (11, 21, 8), (28, 19, 9)],
    [(25, 22, 23), (9, 12, 19), (15, 18, 23), (27, 28, 30)],
]

SPR_PALETTES = [
    # colour 0 is transparent for sprites
    [(0, 0, 0), (3, 3, 6), (20, 16, 9), (28, 21, 16)],   # 0 player: beige coat
    [(0, 0, 0), (3, 3, 6), (8, 11, 17), (27, 21, 9)],    # 1 fort: blue-grey blouse, yellow leather coltin
    [(0, 0, 0), (3, 3, 6), (18, 6, 6), (28, 21, 16)],    # 2 merchant
    [(0, 0, 0), (3, 3, 6), (28, 28, 27), (25, 18, 13)],  # 3 cafe: black waistcoat (outline colour), white apron
    [(0, 0, 0), (3, 3, 6), (15, 15, 16), (27, 20, 15)],  # 4 stranger: grey overcoat and hat (cool grey, apart from the beige player)
    [(0, 0, 0), (3, 3, 6), (8, 19, 6), (24, 16, 7)],     # 5 crate of cabbages
    [(0, 0, 0), (3, 3, 6), (23, 5, 5), (24, 16, 7)],     # 6 crate of apples
    [(0, 0, 0), (3, 3, 6), (30, 15, 3), (24, 16, 7)],    # 7 crate of carrots
]


def remap(tile, mapping):
    return ["".join(mapping[c] for c in row) for row in tile]


# ---------------------------------------------------------------------------
# Background tiles (stored in VRAM bank 1).
# ---------------------------------------------------------------------------
WIN_T = ["00000000", "01111110", "01332210", "01322210",
         "01222210", "01222210", "01111110", "01222210"]
WIN_B = ["01222210", "01222210", "01222210", "01222210",
         "01111110", "11111111", "00000000", "00000000"]
# Same window drawn with the stone palette, glass unlit.
_DARK = {"0": "2", "1": "1", "2": "0", "3": "1"}

# The river: two frames swapped at run time to make the water move.
WATER_FRAMES = [
    # mostly colour 0, so a sprite with the behind-BG flag (the barge) shows on
    # open water but disappears under the bridge, whose tiles use colours 1-3
    ["00000000", "02200000", "00000000", "00000022",
     "00000000", "00030000", "00000000", "22000000"],
    ["00000000", "00022000", "00000000", "22000000",
     "00000000", "00000300", "00000000", "00220000"],
]

TILES = {
    "sky":      ["00000000"] * 8,
    "rooftop":  ["00000000", "00000000", "00000000", "00000000",
                 "33333333", "22212221", "22212221", "22212221"],
    "roof":     ["22212221", "22212221", "22212221", "11111111",
                 "22212221", "22212221", "22212221", "22212221"],
    "dormer":   ["22111122", "21333312", "21322312", "21322312",
                 "21333312", "21111112", "22212221", "22212221"],
    "cornice":  ["33333333", "22222222", "11111111", "32323232",
                 "22222222", "22222222", "11111111", "11111111"],
    "wall":     ["22222221", "22222221", "22222221", "11111111",
                 "22212222", "22212222", "22212222", "11111111"],
    "win_t":    WIN_T,
    "win_b":    WIN_B,
    "dwin_t":   remap(WIN_T, _DARK),
    "dwin_b":   remap(WIN_B, _DARK),
    "door_t":   ["21111112", "21000012", "21000012", "21000012",
                 "21000012", "21111112", "21000012", "21000012"],
    "door_b":   ["21000012", "21000012", "21003012", "21000012",
                 "21000012", "21000012", "21111112", "11111111"],
    "awning":   ["11111111", "22332233", "22332233", "22332233",
                 "22332233", "22332233", "22332233", "01100110"],
    "shopglass": ["11111111", "13322221", "13222221", "12222221",
                  "12222221", "12222221", "12222221", "11111111"],
    "pave":     ["22222221", "22222221", "22222221", "22222221",
                 "22222221", "22222221", "22222221", "11111111"],
    "curb":     ["22222221", "22222221", "22222221", "22222221",
                 "33333333", "33333333", "11111111", "00000000"],
    "cobble":   ["03320332", "03210321", "02110211", "00000000",
                 "32033203", "21032103", "11021102", "00000000"],
    "lamp_head": ["00011000", "00111100", "01333310", "01323210",
                  "01333310", "00111100", "00011000", "00011000"],
    "lamp_post": ["22211221", "22211221", "22211221", "22211221",
                  "22211221", "22211221", "21111111", "11111111"],
    "crate":    ["12122121", "22212222", "11111111", "13333331",
                 "11111111", "13333331", "11111111", "10000001"],
    "plaque":   ["11111111", "13333331", "13111131", "13333331",
                 "13111131", "13333331", "11111111", "22222221"],
    # --- Les Halles pavilion (outside) ---
    "glass":    ["11111111", "13221322", "12221222", "12221222",
                 "11111111", "12221222", "12221222", "12221222"],
    "column":   ["01233210", "01233210", "01233210", "01233210",
                 "01233210", "01233210", "01233210", "01233210"],
    "arch":     ["11111111", "11100111", "11000011", "10000001",
                 "10000001", "00000000", "00000000", "00000000"],
    "opening":  ["00000000", "00000000", "00000000", "00010000",
                 "00000000", "00000000", "00000100", "00000000"],
    "cart":     ["12122121", "22212222", "11111111", "13333331",
                 "13333331", "11111111", "03000030", "01100110"],
    # --- Pavilion interior ---
    "floor":    ["33333332", "33333332", "33333332", "33333332",
                 "33333332", "33333332", "33333332", "22222222"],
    "cabbage":  ["01221220", "12222221", "22322222", "12222321",
                 "01221220", "12211221", "22222222", "11111111"],
    "apples":   ["12212212", "23223223", "12212212", "21221221",
                 "32232232", "21221221", "12212212", "11111111"],
    "carrots":  ["02020202", "23232323", "33333333", "13131313",
                 "33333333", "31313131", "13131313", "11111111"],
    "counter":  ["33333333", "11111111", "13331333", "13331333",
                 "13331333", "13331333", "11111111", "00000000"],
    # --- Cafe ---
    "panel":    ["11111111", "13333331", "13111131", "13111131",
                 "13111131", "13333331", "11111111", "00000000"],
    "shelf":    ["11111111", "10220330", "10220330", "10220330",
                 "11111111", "13033022", "13033022", "11111111"],
    "mirror":   ["11111111", "13333332", "13222222", "13222222",
                 "12222222", "12222222", "12222222", "11111111"],
    "phone":    ["11111111", "10000001", "10333001", "10000001",
                 "10030301", "10003001", "10000001", "11111111"],
    "zinc_top": ["33333333", "33333333", "22222222", "11111111",
                 "22222222", "22222222", "22222222", "22222222"],
    "zinc_front": ["11111111", "21212121", "12121212", "21212121",
                   "12121212", "21212121", "11111111", "00000000"],
    # Cafe floor: cream octagons with small dark cabochons where four tiles
    # meet (a common Paris cafe floor type; reconstruction, not documented
    # for this cafe).
    "checker":  ["22111122", "21111112", "11111111", "11111111",
                 "11111111", "11111111", "21111112", "22111122"],
    "table":    ["01111110", "13333331", "13333331", "11111111",
                 "00011000", "00011000", "00111100", "00000000"],
    # --- The Seine, quai and bridge ---
    "water":    WATER_FRAMES[0],
    "parapet":  ["33333333", "22222222", "22122212", "22222222",
                 "11111111", "21112111", "11111111", "00000000"],
    "box_l":    ["00000000", "02222222", "02111111", "02222222",
                 "02222222", "02222222", "02222222", "00000000"],
    "box_r":    ["00000000", "22222220", "11111120", "22222220",
                 "22322220", "22222220", "22222220", "00000000"],
    "bpar_l":   ["13322221", "13322221", "13322221", "11111111",
                 "13322221", "13322221", "13322221", "11111111"],
    "bpar_r":   ["12222331", "12222331", "12222331", "11111111",
                 "12222331", "12222331", "12222331", "11111111"],
    "gaslamp":  ["00111100", "01111110", "01333310", "01323210",
                 "01333310", "00111100", "00011000", "00111100"],
    "tree_tl":  ['00022222', '00222222', '02222322', '22223222', '22222222', '22232222', '22222222', '02222222'],
    "tree_tr":  ['22222000', '22222200', '22322220', '22232222', '22222222', '22223222', '22222222', '22222220'],
    "tree_bl":  ['22222222', '22232222', '02222222', '00222221', '00000001', '00000001', '00000011', '00000111'],
    "tree_br":  ['22222222', '22223222', '22222220', '12222200', '10000000', '10000000', '11000000', '11100000'],
    # Metro: white tiles (since 1900), iron gate, steps, street railing.
    "mtile":    ["00000002", "00000002", "00000002", "22222222",
                 "00020000", "00020000", "00020000", "22222222"],
    "gate":     ["11111111", "21212121", "21212121", "21212121",
                 "21212121", "21212121", "21212121", "11111111"],
    "stairs":   ["33333333", "22222222", "11111111", "00000000",
                 "33333333", "22222222", "11111111", "00000000"],
    "rail":     ["11111111", "10000001", "10100101", "10000001",
                 "10100101", "10000001", "11111111", "00000000"],
    "barrier":  ["00000000", "33223322", "33223322", "11111111",
                 "01000010", "01000010", "01000010", "00000000"],
    # Saint-Eustache, east end: pinnacle against the sky, buttressed stone,
    # pointed (lancet) windows. Original simplified drawing, palette 0.
    "pinnacle": ["00010000", "00010000", "00121000", "00121000",
                 "01222100", "01232100", "12222210", "12232210"],
    "buttress": ["12222321", "12222321", "11222321", "12222321",
                 "12222321", "11111111", "12232221", "12232221"],
    "lancet_t": ["22222222", "22211222", "22100122", "21000012",
                 "21000012", "21010012", "21010012", "21010012"],
    "lancet_b": ["21010012", "21010012", "21010012", "21010012",
                 "21111112", "22222222", "11111111", "22222222"],
    # Morris column (16x24): green dome, frieze, theatre posters. Wallace
    # fountain (8x16): dome on four caryatids over a basin, drawn in the lamp
    # palette's iron colour (real ones are dark green; 8-palette limit).
    "morris_tl": ['00000001', '00000012', '00001222', '00012222', '00122222', '01111111', '01222222', '01111111'],
    "morris_tr": ['10000000', '21000000', '22210000', '22221000', '22222100', '11111110', '22222210', '11111110'],
    "morris_ml": ['01333331', '01311131', '01333331', '01311331', '01333331', '01111111', '01333311', '01311311'],
    "morris_mr": ['13333310', '13111310', '13333310', '13311310', '13333310', '11111110', '33333310', '31111310'],
    "morris_bl": ['01333311', '01333311', '01333311', '01111111', '01222222', '01222222', '11111111', '00000000'],
    "morris_br": ['33333310', '31131310', '33333310', '11111110', '22222210', '22222210', '11111111', '00000000'],
    "wallace_t": ["00011000", "00111100", "01121110", "01111110",
                  "00100100", "01011010", "01011010", "01011010"],
    "wallace_b": ["01011010", "00111100", "01111110", "00011000",
                  "00011000", "00111100", "01111110", "00000000"],
    # Transport (2026-11-24): a market lorry (32x16, side view, cab left),
    # a VeloSolex (16x8, roller on the front wheel) and a bus-stop post.
    # Generic original shapes, no make or livery copied.
    "lorry_0": ["00000000", "00000000", "00011111", "00122222", "01223332", "01223332", "01222222", "01222222"],
    "lorry_1": ["00011111", "00133333", "10131131", "10133333", "10131131", "10133333", "10131131", "10133333"],
    "lorry_2": ["11111111", "33333333", "13113113", "33333333", "13113113", "33333333", "13113113", "33333333"],
    "lorry_3": ["11111110", "33333310", "11311310", "33333310", "11311310", "33333310", "11311310", "33333310"],
    "lorry_4": ["01222222", "01222222", "01111111", "00011111", "00110001", "00110301", "00011111", "00000000"],
    "lorry_5": ["11111111", "22222222", "11111111", "00000000", "10000000", "10000000", "00000000", "00000000"],
    "lorry_6": ["11111111", "22222222", "11111111", "00000001", "00000011", "00000011", "00000001", "00000000"],
    "lorry_7": ["11111110", "22222210", "11111110", "11110000", "00011000", "03011000", "11110000", "00000000"],
    "solex_0": ["00000000", "01110000", "00010000", "00011111", "00111000", "01000100", "01000100", "00111000"],
    "solex_1": ["00001100", "00000100", "00111112", "11111110", "00111100", "01000010", "01000010", "00111100"],
    "busstop_t": ["00000000", "01111110", "01333310", "01311310", "01333310", "01311310", "01333310", "01111110"],
    "busstop_b": ["00011000", "00011000", "00011000", "00011000", "00011000", "00011000", "00111100", "00000000"],
    # West end of Les Halles (2026-11-24): Bourse de commerce dome, Medici
    # column, a vespasienne (street urinal), a rat on the setts.
    "dome_ul": ['00000000', '00000000', '00000011', '00001122', '00112222', '01122322', '01223222', '12223222'],
    "dome_u": ['00000000', '11111111', '22222222', '23222322', '23222322', '23222322', '23222322', '23222322'],
    "dome_ur": ['00000000', '00000000', '11000000', '22110000', '22221100', '22322110', '22232210', '22232221'],
    "dome_l": ['00000012', '00000122', '00001232', '00012322', '00122322', '01223222', '01232222', '12232222'],
    "dome_m": ['23222322', '23222322', '23222322', '23222322', '23222322', '23222322', '23222322', '11111111'],
    "dome_r": ['21000000', '22100000', '23210000', '22321000', '22322100', '22232210', '22223210', '22223221'],
    "medici_t": ['00011000', '00111100', '01011010', '01011010', '01011010', '11111111', '00123100', '00123100'],
    "medici_s": ['00123100', '00123100', '00123100', '00111100', '00123100', '00123100', '00123100', '00111100'],
    "vesp_tl": ['00000003', '00000001', '00222222', '02111111', '00222222', '00020001', '00020001', '00222222'],
    "vesp_tr": ['30000000', '10000000', '22222200', '11111120', '22222200', '10002000', '10002000', '22222200'],
    "vesp_bl": ['00212121', '00222222', '00212121', '00222222', '00222222', '00000000', '00000110', '00000110'],
    "vesp_br": ['21212200', '22222200', '21212200', '22222200', '22222200', '00000000', '01100000', '01100000'],
    # A little grey car (32x16, nose left), round roof and fenders: a
    # simplified original drawing in the spirit of the deux-chevaux.
    "car_0": ['00000000', '00000000', '00000000', '00000001', '00000012', '00000012', '00011122', '00122222'],
    "car_1": ['00000000', '00111111', '11222222', '21111211', '11111211', '11111211', '22222222', '22222222'],
    "car_2": ['00000000', '11100000', '22211100', '11122210', '11112221', '11111222', '22222222', '22222222'],
    "car_3": ['00000000', '00000000', '00000000', '00000000', '10000000', '11000000', '22210000', '22210000'],
    "car_4": ['01232111', '01222122', '01221112', '00111001', '00010333', '00010333', '00001000', '00000111'],
    "car_5": ['22222222', '22222222', '12222222', '11111111', '01000000', '01000000', '10000000', '00000000'],
    "car_6": ['22222221', '22222222', '22222221', '11111110', '00000001', '00000001', '00000000', '00000000'],
    "car_7": ['22221000', '12221000', '11221000', '00111000', '03330100', '03330100', '10001000', '01110000'],
}

# Map legend: character -> (tile name, bg palette, solid[, priority over sprites])
LEGEND = {
    " ": ("sky", 7, True),
    "^": ("rooftop", 7, True),
    "R": ("roof", 7, True),
    "M": ("dormer", 7, True),
    "=": ("cornice", 0, True),
    "#": ("wall", 0, True),
    "W": ("win_t", 1, True),
    "w": ("win_b", 1, True),
    "K": ("dwin_t", 0, True),
    "k": ("dwin_b", 0, True),
    "D": ("door_t", 0, True),
    "d": ("door_b", 0, True),
    "A": ("awning", 4, True),
    "G": ("shopglass", 1, True),
    ".": ("pave", 2, False),
    "_": ("curb", 2, False),
    ",": ("cobble", 2, False),
    "L": ("lamp_head", 3, True),
    "|": ("lamp_post", 2, True),
    "X": ("crate", 6, True),
    "P": ("plaque", 0, True),
    "E": ("door_b", 0, False),          # door you can walk into
    "g": ("glass", 1, True),
    "I": ("column", 7, True),
    "O": ("arch", 0, True),
    "o": ("opening", 0, True),
    "H": ("cart", 6, True),
    "f": ("floor", 2, False),
    "v": ("cabbage", 6, True),
    "a": ("apples", 4, True),
    "n": ("carrots", 6, True),
    "T": ("counter", 6, True),
    "Y": ("panel", 6, True),
    "S": ("shelf", 6, True),
    "m": ("mirror", 7, True),
    "Q": ("phone", 6, True),
    "Z": ("zinc_top", 7, True, True),   # drawn over sprites (counter hides legs)
    "z": ("zinc_front", 7, True),
    "F": ("checker", 5, False),
    "t": ("table", 6, True),
    "~": ("water", 7, True),
    "-": ("parapet", 0, True),
    "[": ("box_l", 6, True),
    "]": ("box_r", 6, True),
    "b": ("floor", 2, False),          # bridge deck
    "<": ("bpar_l", 0, True),
    ">": ("bpar_r", 0, True),
    "l": ("gaslamp", 3, True),
    "{": ("tree_tl", 6, True),
    "}": ("tree_tr", 6, True),
    "(": ("tree_bl", 6, True),
    ")": ("tree_br", 6, True),
    "%": ("barrier", 4, True),
    "B": ("mtile", 5, True),            # metro: white tiled wall
    "J": ("gate", 7, True),             # metro: gate to the platform
    "U": ("stairs", 2, False),          # metro steps
    "V": ("rail", 3, True),             # railing round a metro entrance
    "+": ("pinnacle", 0, True),         # Saint-Eustache
    "C": ("buttress", 0, True),
    "N": ("lancet_t", 0, True),
    "u": ("lancet_b", 0, True),
    "1": ("morris_tl", 6, True),        # Morris column (reconstruction)
    "2": ("morris_tr", 6, True),
    "3": ("morris_ml", 6, True),
    "4": ("morris_mr", 6, True),
    "5": ("morris_bl", 6, True),
    "6": ("morris_br", 6, True),
    "7": ("wallace_t", 3, True),        # Wallace fountain (reconstruction)
    "8": ("wallace_b", 3, True),
    "c": ("lorry_0", 6, True),
    "e": ("lorry_1", 6, True),
    "h": ("lorry_2", 6, True),
    "i": ("lorry_3", 6, True),
    "j": ("lorry_4", 6, True),
    "r": ("lorry_5", 6, True),
    "s": ("lorry_6", 6, True),
    "x": ("lorry_7", 6, True),
    "y": ("solex_0", 3, True),
    "0": ("solex_1", 3, True),
    "9": ("busstop_t", 3, True),
    "@": ("busstop_b", 3, True),
    "Ø": ("opening", 0, False),         # covered street you can walk through
    "Γ": ("dome_ul", 7, True),          # Bourse de commerce: round, iron dome
    "Δ": ("dome_u", 7, True),
    "Θ": ("dome_ur", 7, True),
    "Λ": ("dome_l", 7, True),
    "Ξ": ("dome_m", 7, True),
    "Π": ("dome_r", 7, True),
    "Σ": ("medici_t", 0, True),         # Medici column: caged top
    "Φ": ("medici_s", 0, True),
    "Ψ": ("vesp_tl", 6, True),          # vespasienne, bottle green
    "Ω": ("vesp_tr", 6, True),
    "Б": ("vesp_bl", 6, True),
    "Д": ("vesp_br", 6, True),
    "Ж": ("car_0", 7, True),            # a little grey car (street)
    "З": ("car_1", 7, True),
    "И": ("car_2", 7, True),
    "Й": ("car_3", 7, True),
    "К": ("car_4", 7, True),
    "Л": ("car_5", 7, True),
    "М": ("car_6", 7, True),
    "Н": ("car_7", 7, True),
}

# ---------------------------------------------------------------------------
# Sprites: 16x16, built from a 12-row upper body and 4-row legs.
# ---------------------------------------------------------------------------
P_DOWN = [   # the young poet: longish hair, a long dark scarf
    "......####......",
    ".....######.....",
    "....########....",
    "...##ssssss##...",
    "...#s#ssss#s#...",
    "...#.ssssss.#...",
    "......#ss#......",
    "....########....",
    "...#o##ooooo#...",
    "...#o##o#oo#....",
    "...so##ooo#os...",
    "....#oooooo#....",
]
P_UP = [   # the young poet: longish hair, a long dark scarf
    "......####......",
    ".....######.....",
    "....########....",
    "...##########...",
    "...##########...",
    "...#.######.#...",
    "......#ss#......",
    "....########....",
    "...#ooo##ooo#...",
    "...#ooo##ooo#...",
    "...so#o##o#os...",
    "....#oooooo#....",
]
P_SIDE = [   # the young poet: longish hair, a long dark scarf
    "......####......",
    ".....######.....",
    "....########....",
    "....###sssss....",
    "....##sss#ss....",
    "....#.sssss.....",
    ".......#ss......",
    "....########....",
    "...##oooooo#....",
    "...#.#ooo#oo#...",
    ".....#ooso#o#...",
    "......#oooo#....",
]
LEGS_FRONT_STAND = [
    "....#oo##oo#....",
    ".....##..##.....",
    ".....##..##.....",
    "....###..###....",
]
LEGS_FRONT_STEP = [
    "....#oo##oo#....",
    ".....##..##.....",
    ".....##...##....",
    "....###...###...",
]
LEGS_SIDE_STAND = [
    "......#oooo#....",
    ".......####.....",
    ".......#..#.....",
    "......##..##....",
]
LEGS_SIDE_STEP = [
    "......#oooo#....",
    "......##..##....",
    ".....##....##...",
    "....###....###..",
]


def mirror(rows):
    return [r[::-1] for r in rows]


PORTER = [
    # The "coltin": a yellow leather hat with very wide brims (see docs/HISTORICAL_RESEARCH.md)
    "......####......",
    ".....#ssss#.....",
    "#ssssssssssssss#",
    ".##############.",
    "....#ssssss#....",
    "....#s#ss#s#....",
    ".....#ssss#.....",
    "...##oooooo##...",
    "..#oooooooooo#..",
    "..#oo#ss#oooo#..",   # metal badge on the blouse
    "..so#oooooo#os..",
    "...#oooooooo#...",
    "...#oooooooo#...",
    "....#ooooooo#...",
    "....###..###....",
    "...####..####...",
]
# Idle frame: head and shoulders bob down 1px (breathing).
PORTER_IDLE = ["................"] + PORTER[0:7] + PORTER[8:]
# Fernand tonight: no coltin (left at the cafe), no plate on the blouse (sold).
FERNAND = ["................", "................", "................",
           ".....######.....", "....#ssssss#...."] + PORTER[5:9] + \
          ["..#oooooooooo#.."] + PORTER[10:]
FERNAND_IDLE = ["................"] + FERNAND[0:7] + FERNAND[8:]


MERCHANT = [
    "......####......",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#o#ssss#o#...",
    "....#s#ss#s#....",
    ".....#ssss#.....",
    "......#ss#......",
    "....##ssss##....",
    "...#ssoooooss#..",
    "...#s#oooooo#...",
    "...ss#oooooo#ss.",
    "....#oooooooo#..",
    "....#oooooooo#..",
    ".....########...",
    ".....##....##...",
    "....###....###..",
]
CAFE = [
    # Black waistcoat over a white shirt, long white apron
    "......####......",
    ".....######.....",
    "....########....",
    "....#ssssss#....",
    "....s#ssss#s....",
    ".....ssssss.....",
    "......#oo#......",
    "....o######o....",
    "...oo######oo...",
    "...o########o...",
    "...s#oooooo#s...",
    "....#oooooo#....",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....#oooo#.....",
    ".....##..##.....",
]
STRANGER = [
    "................",
    ".....######.....",
    "....#######.....",
    "..###########...",
    "....#ssssss#....",
    "....#s#ss#s#....",
    ".....#ssss#.....",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#ooo#ssssss#.",
    "...#ooo#s####s#.",
    "...#oooo#ssss#..",
    "...#oooooooo#...",
    "....#ooooooo#...",
    ".....##...##....",
    "....###...###...",
]
# A photographer who comes to the market at dawn (cameo: see
# docs/HISTORICAL_RESEARCH.md, "Cameos"). Flat cap, camera at the chest.
PHOTOGRAPHER = [
    "................",
    "................",
    ".....######.....",
    "....#########...",
    "....#ssssss#....",
    "....#s#ss#s#....",
    ".....#ssss#.....",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#o######o#...",
    "...#o#ssss#o#...",
    "...#o######o#...",
    "...#oooooooo#...",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
# A metro ticket puncher (poinconneur): peaked cap, uniform, punch in hand.
PUNCHER = [
    "................",
    ".....######.....",
    "....########....",
    "...##########...",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....ssssss.....",
    "......#ss#......",
    "....#oooooo#....",
    "...#oo#oo#oo#...",
    "...#oooooooo#...",
    "...so#oooo#o##..",
    "....#oooooo#s#..",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
# A singer out at night by the river (cameo: see docs/HISTORICAL_RESEARCH.md,
# "Cameos"). Tousled hair, white shirt, dark jacket, hands in pockets.
SINGER = [
    "................",
    "....#.####......",
    "...#########....",
    "...##########...",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....ssssss.....",
    "......####......",
    "....#oossoo#....",
    "...#ooossooo#...",
    "...#ooossooo#...",
    "...#oo#ss#oo#...",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
# The cafe cat (side view, facing left): sitting and running.
CAT_SIT = ["................"] * 5 + [
    "..#...#.........",
    ".#o#.#o#........",
    ".#ooooo#........",
    ".#sosos#........",
    ".#ooooo#.....#..",
    "..#ooo#.....#o#.",
    "..#ooooo#...#o#.",
    ".#ooooooo#.#o#..",
    ".#ooooooo##o#...",
    ".#oo#oo#ooo#....",
    "..##.##.###.....",
]
CAT_RUN = ["................"] * 6 + [
    "..#...#.........",
    ".#o#.#o#........",
    ".#ooooo#######..",
    ".#sosooooooooo##",
    "..#oooooooooo#..",
    "...#oo##oo##o#..",
    "..#o#..#o#.#o#..",
    "..##...##...##..",
    "................",
    "................",
]
CRATE = [
    "................",
    "................",
    "...#.##.#.##....",
    "..#oo#oo#oo#o#..",
    "..#oooooooooo#..",
    "..############..",
    "..#ssssssssss#..",
    "..############..",
    "..#ssssssssss#..",
    "..############..",
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
]


def bob(rows):
    """Idle frame: upper body moves down 1px."""
    return ["................"] + rows[0:7] + rows[8:]

BARGE = [
    "................................",
    "................................",
    "......................######....",
    "......................#ssss#....",
    "......................#s##s#....",
    "......................#ssss#....",
    "...###########################..",
    "..#sssssssssssssssssssssssssss#.",
    ".#ooooooooooooooooooooooooooooo#",
    "#oooooooooooooooooooooooooooooo#",
    "#oooooooooooooooooooooooooooooo#",
    ".#oooooooooooooooooooooooooooo#.",
    "..############################..",
    "................................",
    "................................",
    "................................",
]
PAINTER = [
    "................",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....ssssss.....",
    "......#ss#......",
    "....#oooooo#..#.",
    "...#oooooooo##..",
    "...#oooooooo#...",
    "...so#oooo#os...",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....######.....",
    ".....##..##.....",
    "....###..###....",
]
PAINTER_1 = PAINTER[:7] + ["....#oooooo#.#..", "...#oooooooo#...", "...#ooooooo##..."] + PAINTER[10:]

# Sprite frames in VRAM order. Each frame uses 4 tiles (8x16 mode: TL, BL, TR, BR).
SPRITE_FRAMES = [
    ("PLAYER_DOWN_0", P_DOWN + LEGS_FRONT_STAND),
    ("PLAYER_DOWN_1", P_DOWN + LEGS_FRONT_STEP),
    ("PLAYER_DOWN_2", P_DOWN + mirror(LEGS_FRONT_STEP)),
    ("PLAYER_UP_0", P_UP + LEGS_FRONT_STAND),
    ("PLAYER_UP_1", P_UP + LEGS_FRONT_STEP),
    ("PLAYER_UP_2", P_UP + mirror(LEGS_FRONT_STEP)),
    ("PLAYER_SIDE_0", P_SIDE + LEGS_SIDE_STAND),
    ("PLAYER_SIDE_1", P_SIDE + LEGS_SIDE_STEP),
    ("PLAYER_SIDE_2", P_SIDE + LEGS_SIDE_STEP),
    ("PORTER_0", PORTER),
    ("PORTER_1", PORTER_IDLE),
    ("MERCHANT_0", MERCHANT),
    ("MERCHANT_1", bob(MERCHANT)),
    ("CAFE_0", CAFE),
    ("CAFE_1", bob(CAFE)),
    ("STRANGER_0", STRANGER),
    ("STRANGER_1", [r for r in STRANGER[:9]] + [STRANGER[9].replace("ssssss", "ss#sss")] + STRANGER[10:]),
    ("CRATE", CRATE),
    ("BARGE_L", [r[:16] for r in BARGE]),
    ("BARGE_R", [r[16:] for r in BARGE]),
    ("PAINTER_0", PAINTER),
    ("PAINTER_1", PAINTER_1),
    ("FERNAND_0", FERNAND),
    ("FERNAND_1", FERNAND_IDLE),
    ("PHOTO_0", PHOTOGRAPHER),
    ("PHOTO_1", bob(PHOTOGRAPHER)),
    ("CAT_SIT", CAT_SIT),
    ("CAT_RUN", CAT_RUN),
    ("PUNCHER_0", PUNCHER),
    ("PUNCHER_1", bob(PUNCHER)),
    ("SINGER_0", SINGER),
    ("SINGER_1", bob(SINGER)),
]


# ---------------------------------------------------------------------------
# Cameo sprites kept in VRAM bank 1 (after the portraits), one frame each:
# bank 0 has no free sprite tiles left. Unnamed figures (docs/HISTORICAL_RESEARCH.md).
# ---------------------------------------------------------------------------
ACTOR = [            # open shirt, jacket over the shoulder, tousled hair
    "................",
    "....##.###......",
    "...#########....",
    "...#########....",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....ssssss.....",
    "......#ss#......",
    "....#ossssoo#...",
    "...#oosssooo#...",
    "...#ooossooo#...",
    "...so#ooooo#s...",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
CHANTEUSE = [        # long dark hair, dark dress
    "................",
    ".....######.....",
    "....########....",
    "...###ssss###...",
    "...##s#ss#s##...",
    "...##ssssss##...",
    "...###s##s###...",
    "...###ssss###...",
    "....#######.....",
    "...#########....",
    "...s#######s....",
    "....#######.....",
    "...#########....",
    "...#########....",
    ".....s...s......",
    "....##...##.....",
]
PHILOSOPHER = [      # round glasses, jacket, a book under the arm
    "................",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "...##s##s##.....",
    "....#ssssss.....",
    ".....#s##s#.....",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#oooooo#ss##.",
    "...#oooooo#s#s#.",
    "...#oooooo#ss##.",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
FILMMAKER = [        # very tall, hat, holding a viewfinder up
    ".....######.....",
    "...##########...",
    ".....#ssss#.....",
    ".....#s#s##.....",
    ".....#ssss#.....",
    "......#ss#...##.",
    ".....#oooo#.#s#.",
    "....#oooooo#ss..",
    "....#oooooooo#..",
    "....#oooooo#....",
    "....#oooooo#....",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....#o##o#.....",
    ".....#....#.....",
    "....##....##....",
]
WRITER = [           # glasses, bow tie, a notebook open in the hand
    "................",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "...##s##s##.....",
    "....#ssssss.....",
    ".....#ssss#.....",
    "....#o#ss#o#....",
    "...#oooooooo#...",
    "...#ooooo#sssss#",
    "...#ooooo#s#s#s#",
    "...#ooooo#sssss#",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
RAT = [              # a market rat, side view facing right (palette 4: grey, pink)
    "................",
    "................",
    "................",
    "................",
    "................",
    "................",
    "..........###...",
    ".........#sss#..",
    "......#####ss#..",
    "....##ooooo##o#.",
    "...#oooooooo#o##",
    "..#ooooooooooo#s",
    ".#oooooooooo####",
    "s#ooooooooo#....",
    "s.#o##o###o#....",
    ".ss#..#...#.....",
]
BANK1_SPRITES = []   # cameos moved to the per-room pool (BANK1_LOW_SPRITES)


# UI icons in VRAM bank 1, 8x16 sprites (palette 3: '#' black, 'o' white):
# "!" over the person to talk to next, "A" over the player when something
# in front of them can be talked to or looked at.
ICON_MARK = [
    "..####..", ".#oooo#.", ".#o##o#.", ".#o##o#.", ".#o##o#.", ".#o##o#.",
    ".#oooo#.", ".#o##o#.", ".#oooo#.", "..####..", "...##...", "....#...",
    "........", "........", "........", "........",
]
ICON_PRESS = [
    "..####..", ".#oooo#.", "#oo##oo#", "#o#oo#o#", "#o####o#", "#o#oo#o#",
    "#o#oo#o#", ".#oooo#.", "..####..", "...##...", "....#...", "........",
    "........", "........", "........", "........",
]
BANK1_ICONS = [("MARK", ICON_MARK), ("PRESS", ICON_PRESS)]

# Speech bubble over an NPC who calls out (VRAM bank 0 sprite tiles 254-255,
# palette 3): the line itself opens only when the player presses A.
BUBBLE = [
    ".######.", "#oooooo#", "#o#o#oo#", "#oooooo#", ".##o###.", "..#o#...",
    "..##....", "........", "........", "........", "........", "........",
    "........", "........", "........", "........",
]

# Sprites loaded at tile 0.. of VRAM bank 1 (0x8000-0x83FF is free: the
# scenery is at 0x9000). Two-frame run, facing right; mirrored in code.
THIEF_0 = [          # flat cap, turned-up collar, dark jacket, running
    "................",
    "....######......",
    "...########.....",
    "......#ssss#....",
    "......#ss#s#....",
    ".....#sssss.....",
    "....###sss#.....",
    "...#ooo###oo#...",
    "..#oooooooooo#..",
    "..s#ooooooo#.s..",
    "....#oooooo#....",
    "....#oooooo#....",
    "....##o##o##....",
    "...##.....##....",
    "..##.......##...",
    ".##.........##..",
]
THIEF_1 = THIEF_0[:12] + [
    ".....##oo##.....",
    ".....##..##.....",
    ".....##..##.....",
    "....###..###....",
]
# Writer cameos (2026-11-24), unnamed: see docs/HISTORICAL_RESEARCH.md
# "Cameos". Generic figures, no likeness attempted. Low tiles of VRAM bank 1.
NOVELIST = [         # short dark hair, coat, a notebook held to the chest
    "................",
    ".....#####......",
    "....#######.....",
    "....##ssss##....",
    "....#s#ss#s#....",
    "....#ssssss#....",
    ".....#s##s#.....",
    "......#ss#......",
    "....#oooooo#....",
    "...#oo#sss#o#...",
    "...so#s#s#os....",
    "....#o#sss#.....",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....s....s.....",
    "....##....##....",
]
PLAYWRIGHT = [       # very tall and thin, short brushed-up hair, turtleneck
    ".....#.#.#......",
    ".....######.....",
    ".....#ssss#.....",
    ".....#s#s##.....",
    ".....#ssss#.....",
    "......#ss#......",
    ".....#oooo#.....",
    "....#oooooo#....",
    "....#oooooo#....",
    "....so#oo#os....",
    ".....#oooo#.....",
    ".....#oooo#.....",
    ".....#o##o#.....",
    ".....#o#.#o#....",
    ".....#.#..#.....",
    "....##....##....",
]
CRITIC = [           # neat side parting, scarf, jacket
    "................",
    ".....#####......",
    "....###.###.....",
    "....#ssssss#....",
    "....#s#ss#s#....",
    "....#ssssss#....",
    ".....#s##s#.....",
    "....#ssssss#....",
    "...#oooooooo#...",
    "...#o######o#...",
    "...so#oooo#os...",
    "....#oooooo#....",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
YOUNG_MAN = [        # lanky student, longish hair, books under the arm
    "................",
    "....#######.....",
    "...#########....",
    "...##ssssss#....",
    "...#s#ss#s#.....",
    "....#ssssss.....",
    ".....#ssss#.....",
    "....#oooooo#....",
    "...#oooooo#ss##.",
    "...#oooooo#s#s#.",
    "...#oooooo#ss##.",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....#o##o#.....",
    ".....#o..o#.....",
    "....##....##....",
]
# People of the night, each with their own silhouette (2026-11-24, after
# "everyone dresses alike"). Coltin: read (HISTORICAL_RESEARCH Clothing);
# the rest are drawing choices, no claim.
ROUX = [             # bouquiniste: hair in a bun, scarf, long coat, gloved hands
    "......###.......",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....ssssss.....",
    "....##s##s##....",
    "...#oo####oo#...",
    "...#oooooooo#...",
    "..s#oooooooo#s..",
    "...#oooooooo#...",
    "...#oooooooo#...",
    "..#oooooooooo#..",
    "..#oooooooooo#..",
    "...##########...",
    "....##....##....",
]
COOK = [             # moustache, white double-breasted jacket, a basket on the arm
    "................",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....#####......",
    "......ssss......",
    "....#oooooo#....",
    "...#o#oo#oo#....",
    "...#oooooo#s###.",
    "...#oooooo#####.",
    "...#oooooo#s#s#.",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
FORT2 = [            # a fort with a sack on his shoulder (coltin)
    "......####......",
    ".....#ssss#.....",
    "#ssssssssssssss#",
    ".##############.",
    "....#ssssss#####",
    "....#s#ss#s#sss#",
    ".....#ssss#ssss#",
    "...##oooooo#sss#",
    "..#oooooooo####.",
    "..#oo#ss#ooo#...",
    "..so#oooooo#os..",
    "...#oooooooo#...",
    "...#oooooooo#...",
    "....#ooooooo#...",
    "....###..###....",
    "...####..####...",
]
SLEEPER = [          # flat cap, dark scarf, eyes shut, hands in pockets
    "................",
    "................",
    ".....######.....",
    "....#########...",
    "....#ssssss#....",
    "....s##s##s.....",
    ".....ssssss.....",
    "....########....",
    "...#oooooooo#...",
    "...#oooooooo#...",
    "...#o#oooo#o#...",
    "...#oooooooo#...",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
WORKER_CAFE = [      # a fort dozing at the counter: coltin, eyes shut, a cup
    "......####......",
    ".....#ssss#.....",
    "#ssssssssssssss#",
    ".##############.",
    "....#ssssss#....",
    "....s##s##s.....",
    ".....#ssss#.....",
    "...##oooooo##...",
    "..#oooooooooo#..",
    "..#oo#ss#oooo#..",
    "..so#oooooo#s##.",
    "...#oooooooo#s#.",
    "...#oooooooo#...",
    "....#ooooooo#...",
    "....###..###....",
    "...####..####...",
]


def _sparkle():
    """16x16 four-point star with short diagonals: the "got it!" burst.
    Sprite palette 7 (orange 'o', gold 's')."""
    rows = []
    for y in range(16):
        r = ""
        for x in range(16):
            dx, dy = abs(x - 7.5), abs(y - 7.5)
            axis = (dx < 1 and dy < 7) or (dy < 1 and dx < 7)
            diag = abs(dx - dy) < 0.6 and 1.5 < dx < 4.5
            if axis or diag:
                r += "s" if max(dx, dy) < 3 else "o"
            else:
                r += "."
        rows.append(r)
    return rows


SPARKLE = _sparkle()

WAITER = [           # brasserie waiter: moustache, bow tie, waistcoat, long apron
    "................",
    ".....#####......",
    "....#######.....",
    "....#ssssss#....",
    "....s#ss#ss.....",
    ".....#####......",
    "......#ss#......",
    "....o######o....",
    "...oo##oo##oo...",
    "...o###oo###o...",
    "...s#oooooo#s...",
    "....#oooooo#....",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
ARCHIVIST = [        # lots of wavy hair, sideburns, a beard but no moustache
    "...#.##.##.#....",
    "..###########...",
    "..############..",
    "..###ssssss###..",
    "..##s#ss#sss##..",
    "..##ssssssss##..",
    "...##ssssss##...",
    "....##s##s##....",
    "....########....",
    "...#oo####oo#...",
    "...#oooooooo#...",
    "...so#oooo#os...",
    "....#oooooo#....",
    "....#oooooo#....",
    ".....##..##.....",
    "....###..###....",
]
BANK1_LOW_SPRITES = [      # the first LOW_FIXED (3) stay loaded; the rest are
    ("THIEF_0", THIEF_0),  # copied per room into a pool (pool_load, notebook.c)
    ("THIEF_1", THIEF_1),
    ("SPARKLE", SPARKLE),
    ("NOVELIST", NOVELIST),
    ("PLAYWRIGHT", PLAYWRIGHT),
    ("CRITIC", CRITIC),
    ("YOUNG_MAN", YOUNG_MAN),
    ("ROUX", ROUX),
    ("COOK", COOK),
    ("FORT2", FORT2),
    ("SLEEPER", SLEEPER),
    ("WORKER_CAFE", WORKER_CAFE),
    ("WAITER", WAITER),
    ("ACTOR", ACTOR),
    ("CHANTEUSE", CHANTEUSE),
    ("PHILOSOPHER", PHILOSOPHER),
    ("FILMMAKER", FILMMAKER),
    ("WRITER", WRITER),
    ("ARCHIVIST", ARCHIVIST),
]


# ---------------------------------------------------------------------------
# "DEFENSE D'AFFICHER" painted on a facade (2026-11-24): 3x5 capitals on a
# 4 px pitch, dark ink (colour 0) on stone (2), wall palette 0. 9 tiles.
# ---------------------------------------------------------------------------
_MICRO = {
    "D": ["110", "101", "101", "101", "110"], "E": ["111", "100", "110", "100", "111"],
    "F": ["111", "100", "110", "100", "100"], "N": ["110", "101", "101", "101", "101"],
    "S": ["011", "100", "010", "001", "110"], "A": ["010", "101", "111", "101", "101"],
    "I": ["111", "010", "010", "010", "111"], "C": ["011", "100", "100", "100", "011"],
    "H": ["101", "101", "111", "101", "101"], "R": ["110", "101", "110", "101", "101"],
    "'": ["010", "010", "000", "000", "000"], " ": ["000"] * 5,
}


def _inscription(text, accents=(1,)):
    w = 4 * len(text)
    w += -w % 8
    rows = [["2"] * w for _ in range(8)]
    for i, ch in enumerate(text):
        for r, bits in enumerate(_MICRO[ch]):
            for c, b in enumerate(bits):
                if b == "1":
                    rows[2 + r][4 * i + c] = "0"
    for i in accents:                       # the accent of DÉFENSE
        rows[1][4 * i + 1] = "0"
    rows[7] = ["1"] * w                     # mortar line, as on the wall tile
    return ["".join(r) for r in rows]


_INSCR = _inscription("DEFENSE D'AFFICHER")
INSCRIPTION_CHARS = "$&*/:?`'\""
for _k in range(len(_INSCR[0]) // 8):
    TILES["inscr_%d" % _k] = [r[8 * _k:8 * _k + 8] for r in _INSCR]
    LEGEND[INSCRIPTION_CHARS[_k]] = ("inscr_%d" % _k, 0, True)


def _line_map():
    """Line 4 map on the ticket-hall wall, 32x16, palette 7 (blue enamel
    board, white line, stations as dots; LES HALLES as a ring). Drawing
    is a reconstruction: no photo of the 1967 board was found."""
    rows = [["1"] * 32 for _ in range(16)]
    for x in range(32):
        rows[0][x] = rows[15][x] = "0"
    for y in range(16):
        rows[y][0] = rows[y][31] = "0"
    for x in range(3, 29):                  # the line
        rows[8][x] = rows[9][x] = "3"
    for x in (4, 8, 12, 20, 24, 28):        # other stops: closed for now
        for y in (7, 10):
            rows[y][x - 1] = rows[y][x] = "2"
    for y in range(5, 13):                  # LES HALLES: a ring
        for x in range(13, 19):
            edge = y in (5, 12) or x in (13, 18)
            if edge and not ((y in (5, 12)) and (x in (13, 18))):
                rows[y][x] = "3"
    for x in (15, 16):                      # arrow: you are here
        rows[2][x] = rows[3][x] = "3"
    rows[3][14] = rows[3][17] = "3"
    return ["".join(r) for r in rows]


_MAP4 = _line_map()
for _k, _ch in enumerate("αβγδ"):
    TILES["map4_%d" % _k] = [r[8 * _k:8 * _k + 8] for r in _MAP4[:8]]
for _k, _ch in enumerate("εζηθ"):
    TILES["map4_%d" % (4 + _k)] = [r[8 * _k:8 * _k + 8] for r in _MAP4[8:]]
for _k, _ch in enumerate("αβγδεζηθ"):
    LEGEND[_ch] = ("map4_%d" % _k, 7, True)


# ---------------------------------------------------------------------------
# Furniture to fill the rooms (2026-11-24, generic, no claim beyond "a
# market had baskets, a café chairs, a station benches and posters").
# ---------------------------------------------------------------------------
TILES["basket"] = ["02020200", "22222222", "13131313", "31313131",
                   "13131313", "31313131", "11111111", "00000000"]
# Chair drawn over the café floor tile (palette 5): dark wood (3) on the
# checker pattern, so it does not sit in a dark box.
_CHAIR = ["..33....", ".3..3...", ".3..3...", ".333333.",
          ".3....3.", ".3....3.", ".3....3.", "........"]
TILES["chair"] = ["".join(c if c != "." else f for c, f in zip(cr, fr))
                  for cr, fr in zip(_CHAIR, ["22111122", "21111112", "11111111", "11111111",
                                             "11111111", "11111111", "21111112", "22111122"])]
TILES["bench_l"] = ["11111111", "33333333", "11111111", "00000000",
                    "33333333", "11111111", "01000000", "01000000"]
TILES["bench_r"] = ["11111111", "33333333", "11111111", "00000000",
                    "33333333", "11111111", "00000010", "00000010"]
_POSTER = [
    "3333333333333333", "3222222222222223", "3222333333332223", "3223322222233223",
    "3223222222223223", "3223222112223223", "3223221111223223", "3223221111223223",
    "3223222112223223", "3223222222223223", "3223322222233223", "3222333333332223",
    "3222222222222223", "3211111111111123", "3222222222222223", "3333333333333333",
]
for _k, (_x, _y) in enumerate(((0, 0), (8, 0), (0, 8), (8, 8))):
    TILES["poster_%d" % _k] = [r[_x:_x + 8] for r in _POSTER[_y:_y + 8]]
LEGEND["ι"] = ("basket", 6, True)
LEGEND["π"] = ("chair", 5, True)
LEGEND["κ"] = ("bench_l", 6, True)
LEGEND["λ"] = ("bench_r", 6, True)
for _k, _ch in enumerate("μνξο"):
    LEGEND[_ch] = ("poster_%d" % _k, 4, True)
