# PARIS DISPARU 2: The Last Night (Les Halles, 27-28 February 1969)

The second episode is a separate ROM, built from a copy of the Episode 1
engine in this folder, so that Episode 1 on `dev` stays untouched.

You are a young poet on the last night of the Les Halles market. Fernand's
brass porter's plate has been stolen. Ride métro line 4 between four
districts (Les Halles, Cité, Saint-Michel, Saint-Germain-des-Prés) and
answer three questions in any order: who took it (pétanque in place
Dauphine), where he sleeps (keep the beat in a jazz cellar), and why (finish
an old poet's line). Then find him under the bridge. At dawn, the flowers fly.
An epilogue shows 3 March 1969, the lorries leaving for Rungis.

- Design: [docs/episode2/DESIGN.md](../docs/episode2/DESIGN.md)
- Sources and what is fiction: [docs/episode2/RESEARCH.md](../docs/episode2/RESEARCH.md)
- English, French and Korean (chosen after the title).

## Build and test

```sh
cd episode2
make assets   # text, maps, scenes, music -> src/*.c (python3 + Pillow)
make          # build/paris-disparu.gbc (GBDK-2020 4.3.0 in /opt/gbdk)
python3 tests/story_test.py build/paris-disparu.gbc   # title to THE END
python3 web/pack.py EP1_WEB_ZIP build/paris-disparu-2-web.zip
```

`tests/story_test.py` plays the whole story in PyBoy the way a player
would. It talks to people, rides the métro and plays the mini-games by
reading the screen, and checks 37 steps. The other tests in `tests/` are
still Episode 1's and are not updated for Episode 2.

## What is new compared with Episode 1

| Feature | State |
|---|---|
| 14 rooms: 5 in Les Halles (Episode 1 maps), 9 painted scenes (`tools/scenes_ep2.py`, own tile set per room) | done, all reached in the story test |
| Métro line-4 map, tickets bought with coins (a free ride when broke, so you never get stuck) | done, tested |
| Coins (top right), carried items (top left) | done |
| Mini-games: pétanque, keep the beat, find the red scarf, throw the flowers, plus Episode 1's cups and chase. Each starts with a rules card the first time. The two timing games play themselves once first ("Watch"). | done, the first four tested |
| Random encounters: a tramp or a pickpocket (who leads to a chase) | done, not in the story test |
| Phone numbers in the 1969 format (ODÉ 8400...) | done |
| 10 secrets, 4 extras, 6 album objects, a rank at the end | done, not all in the story test |
| Epilogue (3 March 1969) and an Episode 3 teaser | done, reached in the story test |
| French and Korean | done. A native speaker review is advisable. |
