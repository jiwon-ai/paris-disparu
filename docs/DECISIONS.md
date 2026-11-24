# Decisions and feedback log

What the creative director (Jiwon) asked for or decided, and what was done.
Newest first. Dates are 2026.

| Date | Feedback / decision | What changed |
|---|---|---|
| 11-24 | Dawn mugger encounter (non-combat) | After Mme Roux gives the brass plate, a pickpocket in a flat cap bumps into the player ("Pardon!") and runs off with it. New mini-game "Stop, thief!" on the quai: walk into him within 30 s; he runs away from the player and around obstacles. Fail: retry; win: he drops the plate (clue). Rules card like the other games. Code moved to ROM bank 7 (`games.c`) |
| 11-24 | Full Korean localization | assets/dialogue/ko.txt: all 152 dialogue entries translated (417 pages); Korean pages break between sentences. Screenshots: docs/screenshots/ko/ |
| 11-24 | Korean font: public pixel font (Galmuri, OFL 1.1) | Galmuri 2.40.3 from npm, unchanged BDFs in tools/fonts/galmuri with licence and source. Language menu after the title (English / 한국어). Dialogue in Galmuri11 (variable width, 2 lines of 144 px, 6-row box); menus and notebook in Galmuri7 (one tile per syllable). Code in ROM bank 6, glyphs and pages in banks 8+; ROM now 256 KiB. Missing translations fall back to English. Note for legal review: the package says Galmuri is based on the Nintendo DS font design |
| 11-24 | Paris details (metro, plaques, Morris column, Wallace fountain) | Drawn from the sourced facts in HISTORICAL_RESEARCH "Paris design details": blue LES HALLES plaque, blue street plaque, Morris column (replaces the crates in rue Baltard), Wallace fountain on the quai; both examinable. Palette 7 dark colour made bluer for enamel |
| 11-23 | Korean option needs VRAM room (engine groundwork) | Measured: background tiles already use 0x8800 mode (LCDC 0xE7), so scenery sits at 0x9000 in VRAM bank 1. Real limit is 128 scenery tiles, not 64 (docs corrected, generator check raised). VRAM bank 0 0x9000-0x97FF (128 tiles) is free for Korean text. Mode now set explicitly in `main()` |
| 11-23 | I don't know how to dial or how to play: build it from the player's side | docs/UX_PLAYTHROUGH.md: rules cards before each mini-game, dial prompts + START picks notebook numbers, jeton goal, "!" over who to talk to, "A" bubble, idle goal reminder. Story logic moved to ROM bank 5 to free bank 0 |
| 11-23 | Two famous people is too few; fix the engine to raise sprite capacity | Cameo sprites and portraits now also live in VRAM bank 1. 5 more cameos (7 in all): actor (Belmondo), chanteuse (Gréco), philosopher (Sartre), filmmaker (Tati), writer (Queneau); each a first talk + a hint (secrets or quest); sources and legal flags in HISTORICAL_RESEARCH Cameos |
| 11-23 | Text is hard to read | Bold font (2-px vertical strokes, tighter letter spacing); dialogue shows 2 lines per page with a blank row between; barks shortened to 2 lines |
| 11-23 | Hidden things to find by poking around (cute) | 9 secrets: 7 spots (coin, cat's eyes, carved heart, the cat asleep in the café, pigeon, Pont Neuf faces, metro mouse) + 2 gags (knock 3x on the hotel door, stare 5x at a lamp); "Secret found! n/9" with a two-note ding; count in the notebook and on the end screen |
| 11-23 | Record everything on GitHub | Every change is committed to `dev` with docs; this log |
| 11-23 | Show the real Les Halles of the 1960s | Research in docs/VANISHED_PARIS_DATABASE.md (PR #2). Maps now follow it: rue Rambuteau (north, Saint-Eustache's east end) → rue Baltard between the pavilion groups → south to the quai and the Pont Neuf |
| 11-23 | Cuter graphics, more game features (inventory, money, save, run, map, album) | Planned after the quest rework |
| 11-23 | Credit the creator: "© Jiwon", no year | Title screen bottom row, new © glyph |
| 11-23 | Play like a real game: quests, mini-games, fetch/trade, vanished objects | Quest chain with 3 new mini-games (serve, cups, catch) and the Mme Roux trade; QUEST COMPLETE box and fanfare |
| 11-23 | Famous people of the era as hint-givers (recognisable, unnamed) | Photographer (Doisneau-inspired), singer (Gainsbourg-inspired); Cameos table with legal flags |
| 11-23 | Era culture: trends, literature, poetry, songs, theatre, photography | Scenery references with rights status; only public-domain text quoted |
| 11-23 | Show the Paris that no longer exists (e.g. métro ticket punchers) | Métro ticket hall with poinçonneur and punch mini-game |
| 11-23 | More goals and a hook to the next episode | Side goals (notebook page 2), bonus scene, Episode 2 teaser |
| 11-23 | A short explanation at the start; a fun, clear opening | 3-page skippable intro, goal caption, the cat |
| 11-23 | Player invisible on the quai (bug) | Fixed in player_unstick; visibility test in CI |
| 11-23 | Fernand found but nothing happens | He and the notebook say what is missing |
| 11-23 | START should advance dialogue | A or START |
| 11-23 | All game text in English | Overrides the French-first brief |
| earlier | Story only possible in 1967 | Fernand / Rungis mystery |
| earlier | No fabricated history; sources for every claim | docs/HISTORICAL_RESEARCH.md: verified / reconstruction / fiction |
