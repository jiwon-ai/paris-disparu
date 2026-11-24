# Technical architecture

## Target and build

- GBDK-2020 4.3.0, C (SDCC), `make`.
- Cartridge header set by `makebin` flags in the `Makefile`: CGB only (`-Wm-yC`,
  0x143 = 0xC0), MBC5 (`-Wm-yt0x19`) so that ROM banking can be added without
  changing the cartridge type. Current ROM: 256 KiB (16 banks). Code is in bank 0, `src/assets.c` in bank 1 (always mapped), `src/dialogue_text.c` in bank 2; `dialogue.c` maps bank 2 with `SWITCH_ROM` only to copy one page into RAM.
- On a monochrome Game Boy the game shows a "Game Boy Color required" message
  (`_cpu != CGB_TYPE` check in `main.c`).

## Modules

| File | Responsibility |
|---|---|
| `src/main.c` | boot, title, state machine (walk / dialogue / pause), room changes, ambience |
| `src/input.c` | joypad state and newly pressed buttons |
| `src/actor.c` | player movement, collision, interaction probe, NPC idle animation, sprite drawing |
| `src/world.c` | loading a room into VRAM, solid tile and hotspot queries |
| `src/rooms.c` | room contents: NPCs, hotspots, exits (data) |
| `src/dialogue.c` | window-layer text box, paged typewriter dialogue |
| `src/sound.c` | sound effects on the APU registers |
| `src/events.c` | story logic: who says what, crates, tokens, phone numbers |
| `src/minigame.c` | the lift timing bar and the telephone dial |
| `src/assets.c` | generated: tiles, palettes, maps, collision, dialogue pages |

## VRAM layout (CGB)

Background tiles use the 0x8800 addressing mode (LCDC bit 4 = 0, set
explicitly at the start of `main()`; it was already GBDK's default, measured
LCDC = 0xE7). BG tile indices 0-127 are at 0x9000-0x97FF and 128-255 at
0x8800-0x8FFF; sprites always use 0x8000-0x8FFF. `set_bkg_data` follows the
LCDC mode. Measured in PyBoy: the scenery is in VRAM bank 1 at 0x9000.

- Bank 0, 0x8000-0x87FF (sprite tiles 0-127): character sprites (8x16 mode, 4 tiles per frame).
- Bank 0, 0x8800-0x8FFF (BG/sprite tiles 128-242): font and text box frame.
- Bank 0, 0x9000-0x97FF (BG tiles 0-127): free (reserved for streamed Korean text).
- Bank 1, 0x9000-0x97FF (BG tiles 0-127): scenery tiles (BG attribute bit 3);
  up to 128 tiles, checked by `tools/gen_assets.py`.
- Bank 1, sprite tiles 64-231: dialogue portraits (12 tiles each); 232-251: one-frame
  cameo sprites (`SPR1_*`). An NPC with `palette | S_BANK1` (OAM attribute bit 3)
  draws from bank 1 and has no idle animation. All are reloaded by `load_bank1()`.
- Window map: dialogue box at the bottom of the screen. Box tiles have the
  BG-priority attribute and use colour 1 as paper (never colour 0), so the box
  covers sprites on CGB.

## Korean option

- `lang` (0 English, 1 Korean) is set by the language menu after the title.
- `tools/ko_gen.py` (run by `gen_assets.py`) reads `assets/dialogue/ko.txt` and
  `assets/text/ui_ko.txt` (same ids and keys as the English files; a missing
  one falls back to English) and the Galmuri BDF fonts in `tools/fonts/galmuri/`.
- Dialogue: Galmuri11, word-wrapped to 144 px, 2 lines a page. `src/ko.c`
  (ROM bank 6) renders a page into a RAM bitmap and reveals it a tile column
  at a time into BG tiles 0-71 (VRAM bank 0, 0x9000); the box has 6 rows.
- Menus: every UI string goes through `box_print`; in Korean it looks the
  string up by its offset in `ui_text` and draws the Korean one. Hangul are
  Galmuri7 8x8 tiles loaded on demand into a cache (tiles 72-127 in small
  boxes, 0-127 in full-screen boxes); Latin letters and digits stay the
  English font. The generator checks cache sizes per screen.
- Banks: 6 = ko.c + tables + UI strings + Galmuri7 tiles; 8.. = Galmuri11
  glyphs and dialogue pages, read with `far_read()` (bank 0).
- Test: `tests/ko_test.py`.

## Data pipeline

All art and text are plain text so changes are reviewable:

- `tools/font.py` — original 8x8 font including French accents (é è ê ë à â ç î ï ô û ù œ É È À Ç « »).
- `tools/art.py` — palettes, background tiles, map legend, sprites.
- `assets/maps/*.txt` — 20x18 room maps, plus painted signs.
- `assets/dialogue/en.txt` — dialogue (English); word-wrapped to 18 columns, 3 lines per page. `fr.txt` is an unused French draft kept for a later localisation.
- `tools/gen_assets.py` — validates everything (map size, unknown characters,
  glyphs missing from the font, words longer than a line) and writes
  `src/assets.c` / `include/assets.h`. `--check` fails if the committed files
  are stale.

## Rooms and movement

- One room = one 160x144 screen (20x18 tiles). Rooms connect through screen
  edges (`exits[]` per direction); the code for edge transitions exists, and
  is exercised once the second room is added.
- Player moves 1 px per frame, one axis at a time. Collision uses a feet box
  (x+3..x+12, y+10..y+15) against the room's solid map and NPC boxes, so the
  player can overlap facades visually.
- Sprites are assigned OAM slots by y every frame so the lower character is
  drawn in front.
- The barge uses the sprite "behind background" flag. Water tiles are mostly
  colour 0 and the bridge tiles use colours 1-3, so the barge is visible on the
  river and disappears under the bridge. The café counter uses the BG priority
  attribute instead, to hide Simone's legs.

## Music

`tools/music.py` holds the songs (original compositions) as melody tokens
plus a chord list, and writes `src/music_data.c` (bank 4) and
`include/music_songs.h`. The accompaniment is generated from the chords
(waltz: bass, chord, chord; polka: bass, chord, fifth, chord). The driver
(`src/music.c`, bank 0) runs from the VBL interrupt: one step per `tempo`
frames, melody on channel 3 (a one-cycle reedy wave at 50%), accompaniment
on channel 1 (25% duty, volume 3, short decay). It saves and restores the
ROM bank itself. Effects keep channels 2 and 4; `sound_confirm` and
`sound_note` borrow channel 1 by setting `music_ch1_hold`. Songs: title and
end (TITLE), street, market (rue Baltard and the pavilion), café, river
(quai and bridge), métro, mini-games (GAME). `music_play` is a no-op while
the same song plays, so the walk loop calls it every frame with the room's
song.

## Save

Cartridge type 0x1B (MBC5+RAM+BATTERY), RAM size code 0x02 (8 KiB,
`-Wl-ya1`). One slot at 0xA000, 25 bytes: magic "PDS1", room, player x/y
(int16), direction, `howto_seen`, 13 bytes of story state from
`event_pack` (events.c: game/clue/extra/quest/secret flags, cameos met,
carrying, crates taken/delivered, tickets punched) and a checksum. Written
by `save_write` (main.c) after each room change and each dialogue that
ends back in walking; read once after the language menu; erased (magic
byte cleared) when the end screen opens. Blank or random RAM fails the
magic/checksum test and reads as no save.

## Tests

- `tests/check_rom.py`: header logo, CGB flag, cartridge type, size code, header and global checksums.
- `tests/play_test.py`: drives PyBoy (CGB mode, headless) and checks player
  coordinates in RAM (`_player` from the `.noi` symbol file) and the window
  tile map for dialogue text.
- `.github/workflows/build.yml` runs build, checks and the gameplay test.


## Title key art and portraits

- `tools/title_art.py` draws the 160x144 title illustration in code and
  converts it to CGB tiles: each 8x8 tile takes the best of 8 palettes
  (palette 5 is reserved for the blinking "Press START" font text). The
  generator writes `src/title_art.c` into ROM bank 3 (`TITLE_BANK`). During
  the title its tiles use VRAM bank 1 indices 0..; `load_bank1()` puts the
  scenery tiles and portraits back afterwards. The title fades in from black
  (palettes scaled in 4 steps). Preview: `python3 tools/title_art.py` writes
  `docs/art/title_preview.png`.
- `tools/portraits.py` draws six 24x32 speaker busts. They are sprite tiles
  in VRAM bank 1 from `PORTRAIT_BASE` (64), shown as six 8x16 sprites (OAM
  24-29) on top of the dialogue box, with the speaker's sprite palette. The
  portrait of each dialogue is the name before ':' on its first page
  (`dlg_portrait[]`, generated).
- Street-lamp glow (BG palette 3, colours 2-3) moves between three close
  levels at random every 8 frames.
- ROM banks: 0 code, 1 assets, 2 dialogue, 3 title art.


## Dialogue banks

Dialogue pages (54 bytes each) fill ROM bank 2 (`dlg_pages`, DLG_PAGES_LO
pages); the overflow is stored in bank 3 after the title art
(`dlg_pages_hi`). `dialogue.c` maps the right bank per page.

## ROM layout (256 KiB, MBC5, 16 banks)

| Bank | Content |
|---|---|
| 0 | code |
| 1 | assets, always mapped (tiles, maps, palettes, UI text, dialogue index); nearly full |
| 2 | dialogue pages (first DLG_PAGES_LO) |
| 3 | title art + dialogue overflow + `dlg_first_page` (read with far_read) |
| 4 | portrait tiles and low sprites (copied to VRAM bank 1 by load_bank1); music data (`music_data.c`, MUSIC_BANK) |
| 5 | story logic (`events.c`) |
| 6 | Korean code, tables, UI strings, Galmuri7 tiles |
| 7 | mini-games (`games.c`, BANKED functions) |
| 8-10 | Galmuri11 glyphs and Korean dialogue pages |
| 11-15 | free |

Code outside bank 0/1 cannot read bank-1 data directly: `box_print` and
`world_solid` save the current bank, map ASSET_BANK and restore it, so they
are callable from any bank; `far_read()` copies from another bank.
VRAM bank 1 sprite tiles 0-63 hold "low" sprites (`BANK1_LOW_SPRITES` in
tools/art.py, e.g. the pickpocket, `SPR1_THIEF_*`).

Dialogue ids: up to 0xF6 (0xF7-0xFE are EV_* events, EV_FIRST = EV_CHASE; 0xFF none). Hotspot
dialogues must stay below 0x80 (TALK flag); gen_assets.py checks rooms.c.
Bank 0 is full (about 60 bytes free after the cameos): new code must go to banked functions (`#pragma bank`) first.
