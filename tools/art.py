"""Original pixel art for PARIS DISPARU, written as text so it can be diffed.

Background tiles are 8x8, digits are colour indices 0-3 within the tile's
CGB palette. Sprites are 16x16: '.' transparent, '#' = colour 1 (outline),
'o' = colour 2 (clothing), 's' = colour 3 (skin / light).
"""

# ---------------------------------------------------------------------------
# CGB palettes, RGB components 0-31.
# ---------------------------------------------------------------------------
STONE = (15, 14, 16)

BG_PALETTES = [
    # 0 stone facade: night, shadow, stone, highlight
    [(5, 5, 11), (9, 9, 14), STONE, (21, 20, 20)],
    # 1 lit window: stone, frame, amber, warm light
    [STONE, (5, 5, 10), (29, 20, 7), (31, 29, 17)],
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
    # 7 zinc roofs: sky, dark zinc, zinc, light zinc
    [(3, 3, 8), (7, 8, 13), (11, 13, 18), (17, 19, 23)],
]

SPR_PALETTES = [
    # colour 0 is transparent for sprites
    [(0, 0, 0), (3, 3, 6), (20, 16, 9), (28, 21, 16)],   # 0 player: beige coat
    [(0, 0, 0), (3, 3, 6), (7, 11, 22), (28, 26, 22)],   # 1 porter: blue work clothes
    [(0, 0, 0), (3, 3, 6), (18, 6, 6), (28, 21, 16)],    # 2 merchant
    [(0, 0, 0), (3, 3, 6), (26, 26, 26), (25, 18, 13)],  # 3 cafe worker
    [(0, 0, 0), (3, 3, 6), (10, 14, 10), (27, 20, 15)],  # 4 stranger
    [(0, 0, 0), (3, 3, 6), (16, 10, 5), (28, 18, 8)],    # 5 objects
    [(0, 0, 0), (3, 3, 6), (12, 12, 15), (24, 24, 26)],  # 6 spare
    [(0, 0, 0), (3, 3, 6), (12, 12, 15), (24, 24, 26)],  # 7 spare
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
}

# Map legend: character -> (tile name, bg palette, solid)
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
}

# ---------------------------------------------------------------------------
# Sprites: 16x16, built from a 12-row upper body and 4-row legs.
# ---------------------------------------------------------------------------
P_DOWN = [
    "......####......",
    ".....######.....",
    "....########....",
    "....#ssssss#....",
    "....s#ssss#s....",
    ".....ssssss.....",
    "......#ss#......",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#oo#oo#oo#...",
    "...so#oooo#os...",
    "....#oooooo#....",
]
P_UP = [
    "......####......",
    ".....######.....",
    "....########....",
    "....########....",
    "....########....",
    ".....######.....",
    "......#ss#......",
    "....#oooooo#....",
    "...#oooooooo#...",
    "...#oooooooo#...",
    "...so#oooo#os...",
    "....#oooooo#....",
]
P_SIDE = [
    "......####......",
    ".....######.....",
    ".....#######....",
    ".....##sssss....",
    ".....#sss#ss....",
    "......sssss.....",
    ".......#ss......",
    "......#ooo#.....",
    ".....#oooooo#...",
    ".....#ooo#oo#...",
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
    ".....######.....",
    "....#ssssss#....",
    "..#ssssssssss#..",
    "..############..",
    "....#ssssss#....",
    "....#s#ss#s#....",
    ".....#ssss#.....",
    "...##oooooo##...",
    "..#oooooooooo#..",
    "..#ooo#oo#ooo#..",
    "..so#oooooo#os..",
    "...#oooooooo#...",
    "...#ooo##ooo#...",
    "....#oo##oo#....",
    "....###..###....",
    "...####..####...",
]
# Idle frame: head and shoulders bob down 1px (breathing).
PORTER_IDLE = ["................"] + PORTER[0:7] + PORTER[8:]

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
]
