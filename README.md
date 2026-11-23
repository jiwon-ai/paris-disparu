# PARIS DISPARU — La vie qui n'existe plus

An original exploration game for the **Game Boy Color**, set in Paris around
1967. You wander the streets around the old Les Halles market before dawn,
meet the people who work there, and take part in everyday life that no longer
exists. No combat, no quest markers, no collectibles: the historical
experiences are the reward.

![Title](docs/screenshots/01_title.png) ![Street](docs/screenshots/02_street.png)
![Dialogue](docs/screenshots/03_dialogue.png) ![Café](docs/screenshots/05_cafe.png)
![Entrance](docs/screenshots/07_entrance.png) ![Market](docs/screenshots/08_market.png)
![Quai](docs/screenshots/10_quai.png) ![Bridge](docs/screenshots/12_barge.png) ![Telephone](docs/screenshots/09_dial.png)

Screenshots are captured automatically by `tests/play_test.py` in the PyBoy
emulator.

## Status

**Episode 1 vertical slice (Les Halles, 1967) — first complete version.**
Everything below is in the ROM and exercised by the automated play-through
(`tests/play_test.py`), unless marked otherwise.

| Feature | State |
|---|---|
| CGB-only ROM with valid header (MBC5, CGB flag 0xC0) | done, checked by `tests/check_rom.py` |
| Title screen, location caption, START pause | done, tested |
| 6 connected areas: rue Rambuteau, pavilion entrance, pavilion interior, café, quai of the Seine, a bridge | done, tested (screen edges + doors) |
| Seine: animated water, a barge that passes under the bridge, gas lamps, bouquinistes' boxes, plane trees | done, tested (barge movement checked in RAM) |
| Player: 4-direction movement, walk animation, tile + NPC collision | done, tested |
| 6 NPCs with French dialogue that changes with what you have done (incl. a painter on the bridge) | done, tested |
| Experience A: carry Fernand's three crates to the right stalls (read the signs; wrong stall gets a reaction; half speed while carrying) | done, tested |
| Experience B: café scene (Simone, a porter asleep on his feet, Oui/Non choice) | done, tested |
| Experience C: the café wall telephone. Get a token (jeton) from Simone, then dial a number with the D-pad. Four numbers are hidden in the world (the café wall, the man with the notebook, a bouquiniste's box, the painter); each call is a different scene | done, tested — historical details **to research** |
| Two-option choices in dialogue | done, tested |
| Scene-based ambient sound (market noise louder towards the market, café cups) | done, **not verified by ear** |
| Music | not started |
| Message on a monochrome Game Boy | written, **not tested** |
| SameBoy / BGB / real hardware / ModRetro Chromatic | **not tested yet** |

A first-time play-through is currently short (a few minutes, not yet the
10-15 minute target); see [ROADMAP](docs/ROADMAP.md).

## Controls

| Button | Action |
|---|---|
| D-pad | Walk |
| A | Talk, examine, next page |
| B | Close dialogue |
| START | Start game / pause |

## Building

Requirements:

- [GBDK-2020](https://github.com/gbdk-2020/gbdk-2020/releases) 4.3.0 (tested; other 4.x versions likely work)
- GNU make
- Python 3 (only to regenerate assets and run the checks)

```sh
# Linux example
curl -sSLO https://github.com/gbdk-2020/gbdk-2020/releases/download/4.3.0/gbdk-linux64.tar.gz
tar xzf gbdk-linux64.tar.gz -C ~
make GBDK_HOME=~/gbdk
```

Output: `build/paris-disparu.gbc` (64 KiB, MBC5: code in bank 0, graphics/maps in bank 1, dialogue in bank 2). The generated asset files
`src/assets.c` and `include/assets.h` are committed, so Python is not needed
for a plain build.

Other targets:

```sh
make assets      # regenerate assets from tools/*.py and assets/ (python3)
make check       # asset freshness + ROM header validation
python3 tests/play_test.py build/paris-disparu.gbc [screenshot_dir]   # needs: pip install pyboy
```

## Running

Open `build/paris-disparu.gbc` in a Game Boy Color emulator such as
[SameBoy](https://sameboy.github.io/), [BGB](https://bgb.bircd.org/) or
[mGBA](https://mgba.io/), with the emulated model set to Game Boy Color.
The automated test uses PyBoy; the game has not yet been checked in SameBoy,
BGB or on hardware.

## Repository layout

```
src/            game code (C)            include/   headers
assets/maps/    room maps as text        assets/dialogue/fr.txt   French dialogue
assets/text/    UI strings               tools/     font, art and asset generator
tests/          ROM header check, emulator gameplay test
docs/           design, architecture, research, roadmap
```

## Historical research

The game separates verified facts, plausible reconstructions and fiction.
All characters are fictional. Claims and their sources, with confidence
levels, are tracked in [docs/HISTORICAL_RESEARCH.md](docs/HISTORICAL_RESEARCH.md).

## Licence

No licence has been chosen; all rights reserved. See
[LICENSE-PENDING.md](LICENSE-PENDING.md). Game Boy and Game Boy Color are
trademarks of Nintendo; this project is not affiliated with or endorsed by
Nintendo.
