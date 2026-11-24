"""Title key art for PARIS DISPARU: a night view of Les Halles, drawn in code.

Original art (no photo is traced): the silhouette of a large church with a
lantern tower standing behind an iron-and-glass market pavilion, gas lamps,
wet setts, and a market porter in his wide-brimmed hat carrying a crate.
The composition is inspired by the general look of the Les Halles district
(Saint-Eustache behind the Baltard pavilions); it is not a measured drawing.

build() returns the image, the tiles, the map and the palettes. Each 8x8
tile is given the CGB palette (of the 8 below) that reproduces it best.
Run directly to write docs/art/title_preview.png.
"""
import os

from PIL import Image, ImageDraw

import font

W, H = 160, 144

# Colours, CGB 5-bit RGB.
SKY0 = (2, 2, 7)
SKY1 = (4, 4, 11)
SKY2 = (7, 6, 15)
SKY3 = (12, 8, 16)
HAZE = (24, 13, 8)
DARK = (1, 1, 4)
IRON = (6, 8, 13)
AMBER = (29, 19, 6)
LIGHT = (31, 28, 16)
CREAM = (29, 27, 21)
AMBD = (14, 7, 4)
WETB = (5, 6, 11)
STAR = (24, 24, 28)

PALETTES = [
    [SKY0, SKY1, SKY2, STAR],      # 0 high sky and stars
    [SKY2, SKY3, HAZE, DARK],      # 1 low sky glow behind the roofs
    [SKY1, SKY2, DARK, AMBER],     # 2 silhouettes against the sky, lit windows
    [DARK, IRON, AMBER, LIGHT],    # 3 pavilion glass and iron, lamp heads
    [DARK, SKY3, HAZE, LIGHT],     # 4 lamp glow over the haze
    [DARK, DARK, AMBD, CREAM],     # 5 font text ("Press START"): paper 1, ink 3
    [SKY0, SKY1, AMBER, CREAM],    # 6 the logo
    [DARK, WETB, AMBD, AMBER],     # 7 wet setts and reflections
]

BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def rgb8(c):
    return tuple(v * 8 + v // 4 for v in c)


def mix(img, y0, y1, c_top, c_bot, x0=0, x1=W):
    """Ordered-dither vertical gradient between two colours."""
    px = img.load()
    for y in range(y0, y1):
        t = (y - y0) / max(1, y1 - y0 - 1)
        for x in range(x0, x1):
            px[x, y] = rgb8(c_bot if t * 16 > BAYER[y % 4][x % 4] + 0.5 else c_top)


def text(draw, x, y, s, colour, scale=1, advance=6):
    for ch in s:
        rows = font.GLYPHS.get(ch, [])
        for r, row in enumerate(rows):
            for c, v in enumerate(row):
                if v == "#":
                    draw.rectangle([x + c * scale, y + r * scale,
                                    x + c * scale + scale - 1, y + r * scale + scale - 1], fill=rgb8(colour))
        x += int(advance * scale)


def draw_scene():
    img = Image.new("RGB", (W, H), rgb8(DARK))
    d = ImageDraw.Draw(img)
    px = img.load()

    # Sky: deep indigo at the top, a warm glow from the market at the horizon.
    mix(img, 0, 40, SKY0, SKY1)
    mix(img, 40, 64, SKY1, SKY2)
    mix(img, 64, 84, SKY2, SKY3)
    mix(img, 84, 104, SKY3, HAZE)
    for (x, y) in [(9, 5), (47, 3), (98, 6), (139, 4), (4, 38),
                   (156, 22), (33, 46), (110, 50), (150, 52), (60, 52), (5, 58)]:
        px[x, y] = rgb8(STAR)

    # The church: long nave with a steep roof, buttresses, apse, lantern tower.
    nave_top, base = 70, 104
    d.polygon([(0, base), (0, 80), (60, nave_top - 6), (116, 80), (116, base)], fill=rgb8(DARK))
    d.rectangle([0, 80, 116, base], fill=rgb8(DARK))
    d.rectangle([0, 58, 14, 82], fill=rgb8(DARK))          # west facade tower
    d.polygon([(0, 58), (7, 52), (14, 58)], fill=rgb8(DARK))
    for bx in range(10, 112, 13):            # flying buttresses and pinnacles
        d.polygon([(bx - 4, 82), (bx, 72), (bx + 4, 82)], fill=rgb8(DARK))
        d.line([(bx, 66), (bx, 72)], fill=rgb8(DARK))
    d.rectangle([52, 46, 68, 66], fill=rgb8(DARK))        # crossing tower
    d.rectangle([55, 38, 65, 46], fill=rgb8(DARK))        # lantern
    d.polygon([(54, 38), (60, 30), (66, 38)], fill=rgb8(DARK))
    d.line([(60, 37), (60, 30)], fill=rgb8(DARK))
    d.ellipse([110, 78, 130, base + 6], fill=rgb8(DARK))  # apse
    for wx in range(14, 108, 13):            # a few lit lancet windows
        if wx in (40, 79):
            d.rectangle([wx, 86, wx + 1, 92], fill=rgb8(AMBER))
    d.ellipse([56, 50, 64, 58], fill=rgb8(AMBER))         # clock / rose window glow
    d.ellipse([58, 52, 62, 56], fill=rgb8(LIGHT))

    # Moonlight on the roof ridge and the tower edges, so the shapes read.
    d.line([(0, 80), (60, nave_top - 6), (116, 80)], fill=rgb8(SKY2))
    d.line([(52, 46), (52, 64)], fill=rgb8(SKY2))
    d.line([(55, 38), (55, 46)], fill=rgb8(SKY2))
    d.line([(0, 58), (7, 52)], fill=rgb8(SKY2))

    # The pavilion: iron and glass, two stepped roofs and a glowing lantern.
    p0, p1 = 64, 160
    d.rectangle([p0, 96, p1, 120], fill=rgb8(DARK))
    d.polygon([(p0 - 4, 98), (p0 + 10, 88), (p1, 88), (p1, 98)], fill=rgb8(IRON))
    d.rectangle([p0 + 18, 80, p1, 88], fill=rgb8(DARK))
    d.polygon([(p0 + 14, 82), (p0 + 24, 76), (p1, 76), (p1, 82)], fill=rgb8(IRON))
    for gx in range(p0 + 22, p1, 4):          # clerestory glass, lit from inside
        d.rectangle([gx, 83, gx + 2, 86], fill=rgb8(AMBER))
    for gx in range(p0 + 2, p1, 8):           # tall glass bays between columns
        d.rectangle([gx + 2, 101, gx + 6, 116], fill=rgb8(AMBER))
        d.rectangle([gx + 3, 102, gx + 5, 108], fill=rgb8(LIGHT))
        d.line([(gx, 99), (gx, 120)], fill=rgb8(IRON))

    # Street: wet setts with reflections of the lit bays.
    mix(img, 120, H, DARK, WETB)
    for gx in range(p0 + 4, p1, 8):
        for y in range(121, 138, 2):
            if (y + gx) % 4 < 2:
                d.line([(gx, y), (gx + 3, y)], fill=rgb8(AMBD if y > 128 else AMBER))

    # Gas lamps: iron posts, lantern heads, a warm pool of light below.
    for lx in (26, 148):
        d.line([(lx, 92), (lx, 132)], fill=rgb8(IRON))
        d.rectangle([lx - 1, 131, lx + 1, 133], fill=rgb8(IRON))
        d.polygon([(lx - 4, 86), (lx + 4, 86), (lx + 3, 93), (lx - 3, 93)], fill=rgb8(AMBER))
        d.rectangle([lx - 2, 87, lx + 2, 91], fill=rgb8(LIGHT))
        d.polygon([(lx - 5, 86), (lx, 82), (lx + 5, 86)], fill=rgb8(IRON))
        for y in range(134, 142, 2):
            d.line([(lx - 6 + (y - 134), y), (lx + 6 - (y - 134), y)], fill=rgb8(AMBD))

    # A fort des Halles: wide-brimmed coltin, a crate on his shoulder.
    fx, fy = 104, 100     # in front of the lit bays, so he reads as a silhouette
    d.ellipse([fx - 9, fy + 6, fx + 9, fy + 10], fill=rgb8(DARK))       # brim
    d.ellipse([fx - 4, fy + 2, fx + 4, fy + 9], fill=rgb8(DARK))        # crown
    d.rectangle([fx - 3, fy + 10, fx + 3, fy + 14], fill=rgb8(DARK))    # head
    d.polygon([(fx - 7, fy + 15), (fx + 7, fy + 15), (fx + 6, fy + 30), (fx - 6, fy + 30)], fill=rgb8(DARK))
    d.rectangle([fx - 5, fy + 30, fx - 2, fy + 37], fill=rgb8(DARK))    # legs
    d.rectangle([fx + 2, fy + 30, fx + 5, fy + 37], fill=rgb8(DARK))
    d.rectangle([fx + 2, fy + 1, fx + 16, fy + 9], fill=rgb8(AMBD))     # crate
    d.rectangle([fx + 3, fy + 2, fx + 15, fy + 8], outline=rgb8(DARK))

    # Logo: PARIS DISPARU, double-size letters with an amber shadow.
    title = "PARIS DISPARU"
    tw = len(title) * 11 - 1
    tx = (W - tw) // 2
    text(d, tx, 10, title, AMBER, 2, 5.5)
    text(d, tx, 8, title, CREAM, 2, 5.5)
    sub = "LES HALLES  1967"
    text(d, (W - len(sub) * 6) // 2, 28, sub, AMBER)
    d.ellipse([132, 40, 144, 52], fill=rgb8(CREAM))       # a crescent moon
    d.ellipse([128, 38, 140, 50], fill=rgb8(SKY1))

    return img


def build():
    img = draw_scene()
    px = img.load()
    pals = [[rgb8(c) for c in p] for p in PALETTES]
    tiles, index, tmap, amap = [], {}, [], []
    for ty in range(H // 8):
        for tx in range(W // 8):
            best = None
            for pi, pal in enumerate(pals):
                if pi == 5:
                    continue          # reserved for font text
                err, rows = 0, []
                for y in range(8):
                    row = ""
                    for x in range(8):
                        r, g, b = px[tx * 8 + x, ty * 8 + y]
                        ds = [(r - c[0]) ** 2 + (g - c[1]) ** 2 + (b - c[2]) ** 2 for c in pal]
                        k = ds.index(min(ds))
                        err += ds[k]
                        row += str(k)
                    rows.append(row)
                if best is None or err < best[0]:
                    best = (err, pi, rows)
            _, pi, rows = best
            key = tuple(rows)
            if key not in index:
                index[key] = len(tiles)
                tiles.append(rows)
            tmap.append(index[key])
            amap.append(pi)
    return img, tiles, tmap, amap, PALETTES


def preview(tiles, tmap, amap, path):
    img = Image.new("RGB", (W, H))
    px = img.load()
    for i, (t, a) in enumerate(zip(tmap, amap)):
        tx, ty = i % 20, i // 20
        for y in range(8):
            for x in range(8):
                px[tx * 8 + x, ty * 8 + y] = rgb8(PALETTES[a][int(tiles[t][y][x])])
    img.resize((W * 3, H * 3), Image.NEAREST).save(path)


if __name__ == "__main__":
    _, tiles, tmap, amap, _ = build()
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "docs", "art")
    os.makedirs(out, exist_ok=True)
    preview(tiles, tmap, amap, os.path.join(out, "title_preview.png"))
    print("title art: %d unique tiles" % len(tiles))
