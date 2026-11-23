# Game design — Episode 1: Les Halles, 1967

Core loop: wander → notice → approach → interact → experience → keep wandering.
No markers, no checklist, no score. Everything is optional and nothing can be
missed permanently.

## Map (current build)

```
            [ Pavilion interior ]
                     ^ arch
[ Café ] <- door - [ rue Rambuteau ] -- east edge --> [ Pavilion entrance ]
```

The player starts in the quiet street at 3h10. Near the east edge a soft
knock (noise channel) is heard now and then, pulling the player towards the
market. All ambience is sparse and quiet: one sound every 1-4 seconds.

## The mystery (Episode 1 story)

*Fiction set on documented ground* (see docs/HISTORICAL_RESEARCH.md, "Story
elements"). 3 a.m., 1967. Fernand, a fort des Halles for thirty years, has
not come to work. The move of the market to Rungis has been announced and
is being built; Fernand's friend Marcel went to see it and is giving up.

| Clue (notebook line) | Where | Needs |
|---|---|---|
| Fernand is missing | the man with the notebook (street) | — |
| Coltin left behind | Simone, café: he hung his hat and left; she gives it to you | — |
| A petit bleu came | phone 4127: a courier brought a pneumatique to the café | a jeton |
| Bleu: Marcel quits | Simone shows the petit bleu | "A petit bleu came" |
| Plate sold to Roux | phone 7302 (card on a bouquiniste's box, quai) | a jeton |
| He fears Rungis | Germaine, after you carry Fernand's crates for Dédé | the crate task |

Fernand stands bareheaded on the bridge (no coltin, no plate on his
blouse). Until the player has the coltin, the petit bleu and the plate he
says what is still missing (his hat at Simone's; who wrote to him; the plate
sold "on the quai, by the books"), and the notebook goal names the next step
(phone 4127 / tell Simone / look by the river / phone 7302). Then he asks what the player would do:
**Stay** (spend the last winters under the glass) or **Go with** (follow
the market to Rungis). Both bring him back to the café. End screen: clues
found (n/6), 3 March 1969 move, pavilions gone in the 1970s, pneumatique
closed in 1984. B before the question cancels the scene.

## Characters

| NPC | Where | Role |
|---|---|---|
| The man with the notebook | street | first encounter; notes what the player has done since (market, crates, phone) — attention is noticed |
| Fernand, porter | pavilion entrance | offers Experience A |
| Germaine, merchant | interior, cabbage stall | prices, rhythm of the market; reacts to wrong deliveries |
| Simone, café worker | café, behind the counter | coffee choice, gives the telephone token |
| A porter at the counter | café | "sleeps standing up"; comments on the phone call |

## Experiences

- **A — Fernand's crates.** Accept with Oui/Non. Take crates one by one from
  his cart (cabbages, apples, carrots), carry them inside at half speed, and
  put each on the stall whose sign matches. Wrong stall: a reaction, no penalty.
  When all three are delivered Fernand sends the player to the café.
- **B — Café.** Workers who eat while the city sleeps; a coffee choice; the
  porter at the bar; Simone treats you if Fernand sent you.
- **C — The telephone.** Examining it shows a slot "too small for a coin".
  The game does not explain; asking Simone gets a token. Using it dials a
  wrong number (the butcher's), and the token does not come back.

## Opening (first minute)
Title, then a 3-page skippable intro (setting, tonight's goal, controls; A next,
B/START skip). The player starts next to the café with a one-line goal on
screen ("Find Fernand, the missing porter!"). A cat sits nearby: walk up and it
runs ahead in short dashes to the café door, and the game says "Follow!".

## Controls
D-pad walk · A (or START) talk/examine/confirm/next page · B close · START notebook.
