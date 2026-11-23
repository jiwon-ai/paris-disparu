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

## Controls
D-pad walk · A talk/examine/confirm · B close · START pause.
