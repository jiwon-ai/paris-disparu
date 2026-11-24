"""Episode 2 rooms painted in code ("scenes"), converted to CGB tiles.

Each scene is a 160x144 picture drawn with PIL in the colours of its own 8
background palettes, plus an 18x20 walk mask. build(name) converts it: each
8x8 tile takes the palette that reproduces it best, equal tiles are shared,
and if more than 128 remain the least used are replaced by their closest
neighbour (VRAM bank 1 holds 128 scenery tiles).

Palette 5 is the text palette (painted signs, dialogue box) and palette 3
the street-lamp palette whose colours 2-3 flicker at run time: both are the
same as Episode 1 in every scene. Colour 0 of palettes 0, 2, 4, 6 and 7 is
the night: it turns slowly blue as the night goes on (world.c).

Mask characters: '.' walk, '#' solid, 'p' walk and BG in front of sprites,
'P' solid and BG in front of sprites.

All drawings are original. Where a real place is drawn, its look is a
simplified reconstruction (sources in docs/episode2/RESEARCH.md); nothing is
traced from a photograph.
"""
import os

from PIL import Image, ImageDraw

W, H = 160, 144
TEXT_PAL = [(29, 28, 24), (29, 28, 24), (17, 15, 13), (3, 3, 6)]
LAMP_PAL = [(12, 12, 15), (3, 3, 6), (28, 23, 9), (31, 31, 22)]
BAYER = [[0, 8, 2, 10], [12, 4, 14, 6], [3, 11, 1, 9], [15, 7, 13, 5]]


def rgb8(c):
    return tuple(v * 8 + v // 4 for v in c)


class Painter:
    """Small drawing helpers in 5-bit CGB colours."""

    def __init__(self, bg):
        self.img = Image.new("RGB", (W, H), rgb8(bg))
        self.d = ImageDraw.Draw(self.img)
        self.px = self.img.load()

    def rect(self, x0, y0, x1, y1, c):
        self.d.rectangle([x0, y0, x1, y1], fill=rgb8(c))

    def box(self, x0, y0, x1, y1, c):
        self.d.rectangle([x0, y0, x1, y1], outline=rgb8(c))

    def line(self, pts, c, w=1):
        self.d.line(pts, fill=rgb8(c), width=w)

    def poly(self, pts, c):
        self.d.polygon(pts, fill=rgb8(c))

    def ellipse(self, x0, y0, x1, y1, c):
        self.d.ellipse([x0, y0, x1, y1], fill=rgb8(c))

    def dot(self, x, y, c):
        if 0 <= x < W and 0 <= y < H:
            self.px[x, y] = rgb8(c)

    def dither(self, x0, y0, x1, y1, top, bot):
        """Ordered-dither vertical gradient."""
        for y in range(y0, y1 + 1):
            t = (y - y0) / max(1, y1 - y0)
            for x in range(x0, x1 + 1):
                self.px[x, y] = rgb8(bot if t * 16 > BAYER[y % 4][x % 4] + 0.5 else top)

    def pattern(self, x0, y0, x1, y1, rows, cols):
        """Repeat a small pattern (list of strings of colour keys) over a box."""
        ph, pw = len(rows), len(rows[0])
        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                k = rows[y % ph][x % pw]
                if k != " ":
                    self.px[x, y] = rgb8(cols[k])


# ---------------------------------------------------------------------------
# Conversion
# ---------------------------------------------------------------------------
def _quantise(px, tx, ty, pal):
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
    return err, rows


def convert(img, palettes, limit=128):
    """Picture -> (tiles, tile map, palette map). Tiles are 8 strings of digits."""
    px = img.load()
    pals = [[rgb8(c) for c in p] for p in palettes]
    keys, tmap, amap = [], [], []
    for ty in range(H // 8):
        for tx in range(W // 8):
            best = None
            for pi, pal in enumerate(pals):
                if pi == 5:
                    continue
                err, rows = _quantise(px, tx, ty, pal)
                if best is None or err < best[0]:
                    best = (err, pi, rows)
            keys.append(tuple(best[2]))
            amap.append(best[1])
    uniq = []
    index = {}
    for k in keys:
        if k not in index:
            index[k] = len(uniq)
            uniq.append(k)
    tmap = [index[k] for k in keys]
    # Too many tiles: fold the least used tile into its nearest neighbour.
    alive = list(range(len(uniq)))
    remap = list(range(len(uniq)))
    while len(alive) > limit:
        use = {t: 0 for t in alive}
        for t in tmap:
            use[remap[t]] += 1
        victim = min(alive, key=lambda t: (use[t], t))
        def dist(a, b):
            return sum(ca != cb for ra, rb in zip(uniq[a], uniq[b]) for ca, cb in zip(ra, rb))
        target = min((t for t in alive if t != victim), key=lambda t: dist(victim, t))
        for i, r in enumerate(remap):
            if r == victim:
                remap[i] = target
        alive.remove(victim)
    order = {t: i for i, t in enumerate(alive)}
    tiles = [list(uniq[t]) for t in alive]
    tmap = [order[remap[t]] for t in tmap]
    return tiles, tmap, amap, len(uniq)


def preview(tiles, tmap, amap, palettes, path, scale=3):
    img = Image.new("RGB", (W, H))
    px = img.load()
    for i, (t, a) in enumerate(zip(tmap, amap)):
        tx, ty = i % 20, i // 20
        for y in range(8):
            for x in range(8):
                px[tx * 8 + x, ty * 8 + y] = rgb8(palettes[a][int(tiles[t][y][x])])
    img.resize((W * scale, H * scale), Image.NEAREST).save(path)


SCENES = {}


def scene(name):
    def deco(fn):
        SCENES[name] = fn
        return fn
    return deco


def build(name):
    """-> tiles, tmap, amap, solid, priority, palettes, unique count before folding."""
    palettes, img, mask = SCENES[name]()
    assert len(palettes) == 8 and palettes[5] == TEXT_PAL and palettes[3] == LAMP_PAL, name
    assert len(mask) == 18 and all(len(r) == 20 for r in mask), name
    tiles, tmap, amap, n = convert(img, palettes)
    solid = [1 if ch in "#P" else 0 for r in mask for ch in r]
    prio = [1 if ch in "pP" else 0 for r in mask for ch in r]
    return tiles, tmap, amap, solid, prio, palettes, n


import scenes_ep2  # noqa: E402,F401  (registers the scenes)

if __name__ == "__main__":
    import sys
    out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "docs", "art")
    os.makedirs(out, exist_ok=True)
    import scenes as S
    for nm in (sys.argv[1:] or list(S.SCENES)):
        tiles, tmap, amap, solid, prio, pals, n = S.build(nm)
        S.preview(tiles, tmap, amap, pals, os.path.join(out, "scene_%s.png" % nm))
        print("%s: %d unique tiles (%d kept)" % (nm, n, len(tiles)))
