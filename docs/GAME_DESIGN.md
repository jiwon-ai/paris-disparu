# Game design — Episode 1: Les Halles, 1967

Core loop: wander → notice → approach → interact → experience → keep wandering.
No markers, no checklist, no score. Everything is optional and nothing can be
missed permanently.

## Map (current build)

Follows the real 1960s geography (sources: docs/HISTORICAL_RESEARCH.md,
"Real geography of the maps"). North is up.

```
[ Café ] <- door - [ rue Rambuteau ]  (Saint-Eustache's east end, north side)
                         | south
[ Pavilion interior ] <- arch - [ rue Baltard ] -- métro steps --> [ Métro ticket hall ]
                         | south (towards rue du Pont-Neuf)
                     [ Quai ] -- bridge deck --> [ Pont Neuf ]
```

The player starts in the quiet street at 3h10. Near the south side a soft
knock (noise channel) is heard now and then, pulling the player towards the
market. All ambience is sparse and quiet: one sound every 1-4 seconds.

## The mystery (Episode 1 story)

*Fiction set on documented ground* (see docs/HISTORICAL_RESEARCH.md, "Story
elements"). 3 a.m., 1967. Fernand, a fort des Halles for thirty years, has
not come to work. The move of the market to Rungis has been announced and
is being built; Fernand's friend Marcel went to see it and is giving up.

Since 2026-11-23 the main path is a chain of **quests**, each finished by a
mini-game or a trade (user feedback: "make it play like a real game, with
mini-games and quests"). The notebook (START) lists them; finishing one
shows a "QUEST COMPLETE!" box with a four-note fanfare.

| Quest | Giver / where | How it is done | Reward (clue) |
|---|---|---|---|
| 1 Coffee rush | Simone, café | **Serve**: 5 orders, pick COFFEE/SOUP/BREAD before the bar empties, 4 right to win | Fernand's coltin |
| 2 Who wrote him? | phone in the café | win a jeton at the porter's **cup game**, **dial** 4127 (courier), then Simone shows the petit bleu | the petit bleu (Marcel quits) |
| 3 Dédé's crates | Dédé, entrance; Germaine, pavilion | **lift** 3 crates (timing), carry each to the right stall, then **catch** 6 of Germaine's cabbages (3 misses allowed) | Germaine tells about Rungis |
| 4 The brass plate | Mme Roux, quai | trade: she wants a hot gratinée; Simone gives one; bring it to her; a pickpocket snatches the plate, catch him on the quai ("Stop, thief!", 30 s) | his brass plate |
| 5 Find Fernand | the bridge | give back coltin, plate and the bleu, choose Stay / Go with | the ending |

Optional: métro ticket punching, phone the butcher, the painter's wife
(notebook page 2, see "Side goals"). Jetons are won again at the cups for
each call.

Fernand stands bareheaded on the bridge (no coltin, no plate on his
blouse). Until the player has the coltin, the petit bleu and the plate he
says what is still missing (his hat at Simone's; who wrote to him; the plate
sold "on the quai, by the books"), and the notebook goal names the next step
(help Simone / phone 4127 / tell Simone / look by the river / bring Mme Roux
a gratinée). Then he asks what the player would do:
**Stay** (spend the last winters under the glass) or **Go with** (follow
the market to Rungis). Both bring him back to the café. End screen: clues
found (n/6), 3 March 1969 move, pavilions gone in the 1970s, pneumatique
closed in 1984. B before the question cancels the scene.

## Characters

| NPC | Where | Role |
|---|---|---|
| The man with the notebook | street | first encounter; notes what the player has done since (market, crates, phone) — attention is noticed |
| Fernand, porter | rue Baltard | offers Experience A |
| Germaine, merchant | interior, cabbage stall | prices, rhythm of the market; reacts to wrong deliveries |
| Simone, café worker | café, behind the counter | coffee choice, gives the telephone token |
| A porter at the counter | café | "sleeps standing up"; comments on the phone call |

## Experiences

- **Métro ticket hall (vanished Paris).** Steps down from the pavilion
  entrance. The ticket puncher won't let you through without a ticket; buy a
  second-class ticket at the window, he punches it ("Clac!") and lets you try
  his job: punch three passengers' tickets with a timing bar. Reward: he saw
  a big fort with no hat go off towards the river (a hint for the bridge).
  Optional. Sources and status in HISTORICAL_RESEARCH, "Vanished Paris".

- **A — Fernand's crates.** Accept with Oui/Non. Take crates one by one from
  his cart (cabbages, apples, carrots), carry them inside at half speed, and
  put each on the stall whose sign matches. Wrong stall: a reaction, no penalty.
  When all three are delivered Fernand sends the player to the café.
- **B — Café.** Workers who eat while the city sleeps; a coffee choice; the
  porter at the bar; Simone treats you if Fernand sent you.
- **C — The telephone.** Examining it shows a slot "too small for a coin".
  The game does not explain; asking Simone gets a token. Using it dials a
  wrong number (the butcher's), and the token does not come back.

## Side goals (notebook page 2, LEFT/RIGHT)

| Extra | How | Payoff |
|---|---|---|
| Punch tickets | métro: buy a ticket, punch 3 passengers' tickets | the puncher saw Fernand head for the river |
| Phone the early butcher | café phone, 0614 (pencilled on the wall) | a gag; counts for the bonus |
| A message for the painter | phone 2508 (his wife), then tell him | he packs up and goes home |

A note "Extra done! n/3" appears each time. All three: a bonus scene with
the photographer before the end screen. The end screen shows clues n/6 and
extras n/3.

## Hook to Episode 2

After the end screen, START shows "NEXT: EPISODE 2": 3 March 1969, the last
night of the fruit and vegetable trade at Les Halles (Rungis opened that day,
see HISTORICAL_RESEARCH), and an open thread from Episode 1: Fernand's brass
plate is gone from Mme Roux's box. Who took it? (fiction). The teaser ends
"+ new métro stops": each episode opens new stops on line 4 (ROADMAP,
"Series structure"). In the ticket hall, a line-4 map (examinable) rings
LES HALLES and shows the other stops as closed for now.

## Opening (first minute)
Title, then a 3-page skippable intro (setting, tonight's goal, controls; A next,
B/START skip). The player starts next to the café with a one-line goal on
screen ("Find Fernand, the missing porter!"). A cat sits nearby: walk up and it
runs ahead in short dashes to the café door, and the game says "Follow!".

## Controls
D-pad walk · A (or START) talk/examine/confirm/next page · B close · START notebook.


## Secrets (9)

Found by examining things (A) or by insisting. Each shows "Secret found! n/9";
the notebook (page 2) and the end screen count them. All are fiction except
the Pont Neuf mascarons (carved stone faces on the cornice: Musée Carnavalet,
"Mascaron du Pont Neuf"; Atlas Obscura).

| # | Where | How |
|---|---|---|
| 1 | rue Rambuteau | the gutter (curb, x7): a coin |
| 2 | rue Rambuteau | examine a street lamp 5 times in a row: a moth |
| 3 | rue Rambuteau | examine the hotel door 3 times in a row: someone shouts |
| 4 | rue Baltard | the dark covered street (west): a cat's eyes |
| 5 | pavilion | the right-hand column: a carved heart "J + M" |
| 6 | café | the street cat, asleep in the corner (after it ran in) |
| 7 | quai | the left-most tree: a pigeon |
| 8 | Pont Neuf | the west parapet near the top: carved faces |
| 9 | métro | the bottom-left wall: a mouse without a ticket |
