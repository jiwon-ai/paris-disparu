# Paris Disparu: Episode 2 design ("La dernière nuit")

Status: draft 2026-11-24, for Jiwon to react to. Facts are marked **Read**
(with source), **Inference**, or **Fiction**.

## 1. Why Episode 1 is "okay but not very fun" (our diagnosis)

| Problem in Ep1 | Evidence | Fix in Ep2 |
|---|---|---|
| One fixed chain of quests | Quests 1-5 must be done in order (GAME_DESIGN.md) | Three districts open at once, any order |
| Lots of reading before playing | NPC scenes of 4-8 pages | Max 3 pages per talk; action first |
| Mini-games hard to understand | Jiwon's feedback; all five are timing bars | Every mini-game: 1 button, a 3-second "watch me" demo, a free practice try, and after 3 misses it becomes easier (never a dead end) |
| Little reward feeling | Rewards are text ("New clue!") | Francs you earn and spend, item pictures pop up, a poem line pops up, album and rank |
| No surprises | Everything is placed and fixed | Random street encounters, sparkles, phone puzzles, secrets |
| Small world | 10 single-screen rooms | About 13 rooms in 4 districts + métro map |

## 2. Setting and date

- **Night of Thursday 27 to Friday 28 February 1969, 1:00 to 6:00.**
- **Read:** "In the early hours of Friday February 28, 1969, Parisians headed to
  the centuries-old fresh food market ... in the early hours of its last day";
  flowers "tossed into the complex's central square"; a "bed of azaleas, roses
  and tulips" outside Saint-Eustache, "lit up for the occasion"; people "danced
  around pyramids of vegetable crates as a band played popular songs"; "the
  heavy atmosphere of melancholy". The move was "carried out overnight March
  2-3" with "1,500 lorries"; "On Monday March 3 Rungis opened". (AFP via
  [Malay Mail, 2019](https://www.malaymail.com/news/life/2019/02/27/50-years-since-iconic-paris-market-uprooted/1727367);
  3 March opening also on [paris.fr](https://www.paris.fr/pages/rungis-et-paris-une-histoire-commune-6744)).
- **Inference:** "the last night" = night of 27-28 February.
- Ep1's teaser says "3 March 1969. The last night of the vegetables". 3 March
  is when Rungis opened, not the last night. Proposal: Ep2 is the last night
  (28 Feb); 3 March (the move, 1,500 lorries) becomes the Episode 3 teaser.
  Ep1's teaser line needs one change ("28 Feb 1969") - Jiwon decides.

## 3. One-sentence goal (always on screen)

**"Get Fernand's brass plate back before dawn (6:00)."**

**Fiction:** On the market's last night Fernand wants to wear his plate one
last time. It has vanished from Mme Roux's box (Ep1 teaser). A young thief
the player half-recognises from Ep1 is selling things along métro line 4.
Three witnesses in three districts each hold one piece of the trail. With all
three, the player corners the thief and returns to Les Halles for the dawn.

## 4. Core loop (the fun)

```
ride the métro -> new district -> ask around -> mini-game / trade / phone
      ^                                              |
      |   francs, items, album pictures, poem line   v
      +------------- trail piece found <------------+
```

- **Francs (new):** start with 5 F in your pocket (HUD shows coins).
  Earn from mini-games, sparkles, odd jobs. Spend on métro tickets, phone
  jetons, a coffee, a flower, a tip to a witness. Lose some to a pickpocket
  (chase him to get it back). Prices: to research, or kept vague (no invented
  price stated as fact).
- **Clock:** each trail piece moves the night on one hour (1:00 to 6:00), the
  sky lightens. No fail state, no real-time timer.
- **Métro map:** choose a stop on line 4. Arrival animation. Stops not yet
  needed show "Not tonight".

## 5. Places (13 rooms)

Line 4 order south of Les Halles: Les Halles, Châtelet, Cité, Saint-Michel,
Odéon, Saint-Germain-des-Prés (**Read**, HISTORICAL_RESEARCH "Line 4 in 1967";
each station's 1969 look to be checked before drawing).

| District | Rooms | Feeling |
|---|---|---|
| Les Halles (start + finale) | rue Baltard (crate pyramids, band), Saint-Eustache square (lit flower bed), Mme Roux's quai, métro hall | festive but sad |
| Cité | métro exit, flower market alley (closed stalls), Notre-Dame square, Place Dauphine | quiet, stone, river |
| Saint-Michel | the fountain square, a narrow street with a jazz cellar, the cellar inside | students, music |
| Saint-Germain-des-Prés | church square, café terrace (no real café name), a night bookshop/club door | writers, night owls |

New tiles per district (engine change: load a tileset per district instead of
one shared 128-tile set, which Ep1 has filled to 124).

## 6. Mini-games (6, all one button, all different)

| Mini-game | Where | Skill | How it stays easy |
|---|---|---|---|
| Find the face | Les Halles crowd | look / search: move a hand, A on the person with the glint | the glint blinks more often after 20 s |
| Pétanque | Place Dauphine | aim: one press stops a power bar | 3 balls, closest one counts |
| Jazz beat | jazz cellar | rhythm: A when the note reaches the line | 8 notes, 5 to win, slower each retry |
| Flower toss | Les Halles finale | aim left/right, A to throw into the crowd | no losing |
| Stop, thief! | anywhere (encounter) | chase (Ep1, kept) | longer time after a miss |
| Phone dial | payphones | puzzle: numbers from posters, matchbooks, a laundry ticket | numbers go to the notebook automatically |

Every mini-game starts with a 3-second demo showing the button picture,
then "Try!" (practice not counted).

## 7. Random encounters (on entering a street, about 1 in 5)

- A tramp ("clochard") asks for a coin: give and he tells a secret or the
  thief's direction; refuse, no penalty.
- A pickpocket bumps into you: coins gone, chase him.
- A flower seller, a lost dog, a student with a guitar (small gags, poem lines).
All original characters (**Fiction**), period dress researched per type.

## 8. Phone puzzles (period format)

Paris numbers then: three exchange letters + four digits, e.g. ODÉ 8400 (the
speaking clock, already in Ep1). Ep2: 4 new numbers hidden in the world
(a jazz-cellar matchbook, a poster, a note in the thief's dropped jacket, a
bookshop card). Real exchange names only after checking a source; numbers and
who answers are **Fiction**.

## 9. Collectibles (album, hand-drawn 24x32 pictures)

8 new era objects, one per district at least, each found as "???" and named by
the one person who knows (Ep1 "curiosity" system). Candidates, each to verify
with a source before use: a loose paving stone in the Latin Quarter, a flower
thrown on the last night, a carnet of métro tickets, a jazz-cellar matchbook,
a student's mimeographed tract (shown, not quoted), a 45 rpm record (no label),
a brass fort plate, a café jeton. Nothing branded.

## 10. Writing

- Characters reveal things; nobody explains history ("Same flowers they
  threw at my wedding").
- The poet sometimes says a short poetic line when looking at something
  (Jiwon 2026-11-24, e.g. swans). Sparingly, about one per room, EN/FR/KO.
- Each district earns poem lines; the ending is dawn on 28 Feb outside
  Saint-Eustache: flowers, the band, Fernand with his plate; the poem is
  completed.
- Cameos: period figures as hint-givers, unnamed, no likeness claims
  (same legal flags as Ep1).

## 11. Secrets and replay

10 secrets, 8 album objects, 4 phone numbers, 3 endings variants (what you
did with the plate money / who you helped), end rank, Episode 3 teaser:
"2-3 March 1969: the move of the century".

## 12. Build plan

1. `episode2/` folder in the repo (copy of the Ep1 engine), so the Episode 1
   work on `dev` is never touched; own ROM `paris-disparu-2.gbc`.
2. Engine: per-district tilesets, francs + HUD, métro map, encounter system,
   demo/practice wrapper for mini-games.
3. Content district by district (Les Halles, Cité, Saint-Michel, Saint-Germain),
   each playable before the next. EN first, then FR/KO.
4. Tests (PyBoy play-through), ROM + web zip to /mnt/project-files.
