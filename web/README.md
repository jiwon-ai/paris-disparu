# Web player (browser version)

Plays the Game Boy Color ROM in a web page: no install, works on Windows,
macOS, Linux, iPhone/iPad and Android in a current browser.

    make                      # build/paris-disparu.gbc
    python3 web/build.py      # build/web/ and build/paris-disparu-web.zip
    node web/test_web.js [shot_dir]   # headless Chromium test (needs Playwright)

`build/paris-disparu-web.zip` has `index.html` at its root and is ready to
upload to itch.io as an HTML game (guide in Korean: `docs/ITCH_UPLOAD_KO.md`).
The unzipped folder also works by double-clicking `index.html`, because the
ROM and the emulator's wasm are embedded in `game-data.js` (no `fetch()`).

CI (`.github/workflows/web.yml`) builds the ROM, packages the zip, runs the
browser test and uploads the zip as the `paris-disparu-web` artifact.

## Files

| File | What |
|---|---|
| `index.html` | page, styles, on-screen pad, controls legend, © Jiwon |
| `player.js` | emulator loop, audio, keyboard / touch / gamepad input, save states |
| `build.py` | packages `build/web/` and the zip |
| `test_web.js` | Playwright test: desktop keyboard, phone touch, save/load, `file://` |
| `vendor/binjgb/` | emulator (prebuilt, unmodified) and its licence |

## Controls

Arrows or WASD: move. X / K / Space: A. Z / J: B. Enter: START.
Right Shift: SELECT. F6 / F9: save / load a state (kept in the browser's
localStorage; the cartridge has no save memory). Standard gamepads work.
On touch screens: the D-pad and buttons under the screen.

## Third-party code and licence

The emulator is **binjgb** by Ben Smith, MIT licence
(https://github.com/binji/binjgb). Files `binjgb.js` and `binjgb.wasm` are
copied unmodified from its `docs/` folder at commit
`6c04827f63d272f32afd86138a8e7262b3800344` (2026-11-16):

    sha256 85a2dd96d3664f380b068b38d0ef56b268fc1687b5cf3687bbd1f8503a012e84  binjgb.js
    sha256 53115a4b2915c71b188923238326816a48567f0a5d87a804ee266bb30602a3a6  binjgb.wasm

`player.js` follows the structure of binjgb's `docs/simple.js` (MIT, same
author). The MIT licence lets us ship it with a commercial, closed game; the
only obligation is to keep its copyright and licence notice, which the zip
does (`vendor/binjgb/LICENSE`, linked from the page footer). Copyleft players
(e.g. EmulatorJS, GPL-3.0) were not used, to keep licence questions out of a paid release.

The game itself (ROM, art, text) is © Jiwon, all rights reserved (see
`LICENSE-PENDING.md`). The ROM is embedded in the zip, so anyone with the
zip or page can extract it: share the page privately until release.
