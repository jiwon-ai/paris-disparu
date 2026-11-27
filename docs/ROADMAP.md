# Roadmap

## Milestone 1 — first playable (done)
Bootable CGB ROM, street scene, 4-direction movement, collision, one NPC,
French dialogue, examinable objects, pause, build + tests + CI.

## Milestone 2 — Episode 1 vertical slice (first version done)
1. Four connected areas: rue Rambuteau → market entrance → market floor, plus a café.
2. Four NPCs: porter, merchant, café worker, wandering stranger.
3. Experience A: help the porter (short timing/positioning task).
4. Experience B: café scene with an unexpected moment.
5. Experience C: a historical object to examine and use without full explanation.
6. Scene-based ambient sound, a short original theme.
7. Playtest protocol (docs/PLAYTEST_PLAN.md).

Done: items 1-5 and scene-based ambience. Open: original theme (item 6),
playtesting with real players (item 7).

## Milestone 3 — make the slice worth 10-15 minutes
1. ROM banking (bank 0 is ~98% full) so content can grow.
2. Music: small original theme for the street and the market.
3. Denser world: more examinable details, one more encounter per area, NPC animation.
4. Smooth palette fades between rooms; dialogue box that avoids covering the player.
5. Verify research items before keeping them as facts in dialogue.
6. Run the playtest plan; test in SameBoy/BGB and on hardware.

## Series structure: Paris grows along métro line 4 (Jiwon, 2026-11-24)

Each episode cleared opens new stops; the player rides line 4 out of
Les Halles into new districts. Episode 1 already shows it: a line-4 map in
the ticket hall (only LES HALLES ringed, the other stops shown as closed),
the platform gate says "Fernand first", and the Episode 2 teaser ends
"+ new métro stops". Line 4 is the real line through Les Halles
(1908; Porte de Clignancourt - Porte d'Orléans, crossing under the Seine
between Châtelet and Saint-Michel, see HISTORICAL_RESEARCH).

| Episode | Date | Stops open | Status |
|---|---|---|---|
| 1 | 21 Nov 1967 (fiction date) | Les Halles | done |
| 2 | 3 Mar 1969 (Rungis opens) | Les Halles + one new stop | proposal; which stop is Jiwon's choice |
| later | to research | candidates on line 4: Châtelet / Cité (the Seine, Île de la Cité), Saint-Michel (Latin Quarter), Saint-Germain-des-Prés (the cafés of the writer cameos), Montparnasse, Gare du Nord / Barbès | each needs its own sourced research before any fact is used |

Engine needs: a save (to carry the notebook between episodes), a station
select screen on the platform, and one ROM bank of rooms per district.

## Later
Music driver, more neighbourhoods (Métro, pneumatique), English text,
hardware testing (GBC, ModRetro Chromatic), cartridge release preparation.
