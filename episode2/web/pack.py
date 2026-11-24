#!/usr/bin/env python3
"""Package Episode 2 for the browser: reuses the Episode 1 web player
(branch web-player, web/) and swaps in this ROM, the page text and its own
save key. Usage: python3 web/pack.py EP1_WEB_ZIP OUT_ZIP"""
import base64
import re
import sys
import zipfile

src, out = sys.argv[1], sys.argv[2]
rom = open("build/paris-disparu.gbc", "rb").read()
zin = zipfile.ZipFile(src)
zout = zipfile.ZipFile(out, "w", zipfile.ZIP_DEFLATED)
for item in zin.infolist():
    data = zin.read(item.filename)
    if item.filename == "game-data.js":
        t = data.decode()
        t = re.sub(r'window\.GAME_ROM_B64="[^"]*"',
                   'window.GAME_ROM_B64="%s"' % base64.b64encode(rom).decode(), t)
        data = t.encode()
    elif item.filename == "player.js":
        data = data.decode().replace('"paris-disparu-state-1"', '"paris-disparu-2-state-1"').encode()
    elif item.filename == "index.html":
        t = data.decode()
        t = t.replace("<title>Paris Disparu</title>", "<title>Paris Disparu 2</title>")
        t = re.sub(r'content="Paris Disparu:[^"]*"',
                   'content="Paris Disparu 2: an original Game Boy Color game set on the last '
                   'night of the Les Halles market, Paris, 1969."', t)
        t = re.sub(r"<p>Paris, 1967, before dawn\..*?</p>",
                   "<p>Paris, 27 February 1969: the last night of the Les Halles market. "
                   "Fernand's brass porter's plate has been stolen. Ride the métro, play, "
                   "talk and look, and get it back before dawn.</p>", t, flags=re.S)
        t = t.replace("<h1>PARIS DISPARU</h1>", "<h1>PARIS DISPARU 2</h1>")
        t = t.replace("Les Halles, 1967</p>", "The last night · Les Halles, 1969</p>")
        data = t.encode()
    zout.writestr(item, data)
zout.close()
print("wrote", out)
