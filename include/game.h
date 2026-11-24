#ifndef GAME_H
#define GAME_H

#include <stdint.h>
#include <gb/gb.h>

/* Screen and sprite geometry */
#define SCREEN_W 160
#define SCREEN_H 144
#define ACTOR_WIDTH 16
#define ACTOR_HEIGHT 16

/* Facing directions */
#define DIR_DOWN  0
#define DIR_UP    1
#define DIR_LEFT  2
#define DIR_RIGHT 3

/* Characters */
#define NPC_STRANGER 0
#define NPC_PORTER   1
#define NPC_MERCHANT 2
#define NPC_CAFE     3
#define NPC_WORKER   4
#define NPC_PAINTER  5
#define NPC_FERNAND  6
#define NPC_PHOTO    7   /* cameo: a photographer at dawn */
#define NPC_PUNCHER  8   /* metro ticket puncher (poinconneur) */
#define NPC_SINGER   9   /* cameo: a singer out at night */
#define NPC_ROUX     10  /* Mme Roux, bouquiniste, at her box on the quai */
/* Cameos 11-15, in this order: dialogue DLG_ACTOR_HELLO + 2k (first talk)
   and + 2k + 1 (a hint) for k = npc - NPC_ACTOR. */
#define NPC_ACTOR       11
#define NPC_CHANTEUSE   12
#define NPC_PHILOSOPHER 13
#define NPC_FILMMAKER   14
#define NPC_WRITER      15
#define NPC_NOVELIST    16  /* writer cameos added 2026-11-24 */
#define NPC_PLAYWRIGHT  17
#define NPC_CRITIC      18
#define NPC_YOUNG_MAN   19
#define NPC_FORT2       20  /* people of the night, 2026-11-24: a porter, */
#define NPC_COOK        21  /* a bistro cook buying, */
#define NPC_SLEEPER     22  /* a night worker waiting in the métro */
#define NPC_WAITER      23  /* west end, 2026-11-24: the brasserie waiter, */
#define NPC_NIGHTPORTER 24  /* the hotel's night porter, */
#define NPC_SOUPER      25  /* a fort over his soup */
#define NPC_ARCHIVIST   26  /* cameo, brasserie: big hair and a beard (Perec, unnamed) */

/* Persistent story flags (game_flags bits) */
#define FLAG_MET_STRANGER   0x0001
#define FLAG_MET_PORTER     0x0002
#define FLAG_TASK_ACTIVE    0x0004
#define FLAG_TASK_DONE      0x0008
#define FLAG_KNOW_7302      0x0010  /* read Mme Roux's card on the quai */
#define FLAG_MET_MERCHANT   0x0020
#define FLAG_MET_CAFE       0x0040
#define FLAG_SAW_PHONE      0x0080
#define FLAG_HAS_JETON      0x0100
#define FLAG_PHONE_DONE     0x0200
#define FLAG_TICKET         0x0400  /* bought a metro ticket */
#define FLAG_MET_WORKER     0x0800
#define FLAG_SOLVED         0x1000  /* Fernand found and answered: the ending plays */
#define FLAG_MET_PHOTO      0x4000
#define FLAG_CAT_GONE       0x8000  /* the cat has led the player to the cafe */
#define FLAG_MET_PAINTER    0x2000

extern uint16_t game_flags;

/* The mystery: Fernand, a fort des Halles for thirty years, has not come
   to work tonight. Where is he, and why? (Fiction; see HISTORICAL_RESEARCH.)
   One bit per clue, in notebook order. */
#define CLUE_CASE    0x01   /* the man with the notebook: no yellow hat tonight */
#define CLUE_COLTIN  0x02   /* Simone: he left his coltin on the hook */
#define CLUE_COURIER 0x04   /* call 4127: a petit bleu came for him yesterday */
#define CLUE_BLEU    0x08   /* Simone shows the petit bleu: Marcel is stopping */
#define CLUE_PLATE   0x10   /* call 7302: he sold his fort's plate to Mme Roux */
#define CLUE_RUNGIS  0x20   /* Germaine, after the crates: he fears Rungis */
#define CLUE_READY   (CLUE_COLTIN | CLUE_BLEU | CLUE_PLATE)  /* enough to talk to him */
#define CLUE_COUNT 6
extern uint8_t clue_flags;

/* Optional side goals (extra_flags bits), shown on the notebook's second page. */
#define X_TICKETS   0x01   /* punched three passengers' tickets in the metro */
#define X_BUTCHER   0x02   /* phoned the butcher (0614) */
#define X_PAINTER   0x04   /* phoned the painter's wife, then told him */
#define X_WIFE      0x08   /* (step) phoned the painter's wife */
#define X_MET_SINGER 0x10  /* (not a goal) heard the singer's first lines */
#define X_COUNT     3
#define X_ALL       (X_TICKETS | X_BUTCHER | X_PAINTER)
extern uint8_t extra_flags;
extern uint8_t new_extra;      /* an extra was just completed: show a note */
uint8_t extra_total(void) BANKED;

/* Hidden secrets (secret_flags bits 0-8): seven spots to examine
   (DLG_SECRET_COIN.., bit = id - DLG_SECRET_COIN), knocking three times on
   the hotel door, staring five times at a street lamp. */
#define SECRET_COUNT 9
#define SECRET_KNOCK 7
#define SECRET_MOTH  8
extern uint16_t secret_flags;
extern uint8_t new_secret;     /* a secret was just found: show a note */
uint8_t secret_total(void) BANKED;
/* Curiosities: vanished objects found as "???", then shown to someone who
   knows what they are. Bit i = found, bit 4+i = identified. */
#define CURIO_COUNT 3
extern uint8_t curio_flags;
extern uint8_t new_curio;      /* 1: a curiosity found, 2: one identified */
uint8_t curio_total(void) BANKED;   /* identified */

/* Main quests: each is completed by the clue it rewards. */
#define QUEST_COUNT 5               /* rush, who wrote, crates, plate, Fernand */
#define QF_ROUX     0x01            /* Mme Roux asked for a gratinee */
#define QF_SOUP     0x02            /* carrying the gratinee for her */
extern uint8_t quest_flags;
extern uint8_t new_quest;           /* 1-4: that quest was just completed */
uint8_t quest_done(uint8_t q) BANKED;      /* q = 0..QUEST_COUNT-1 */
extern uint8_t new_clue;   /* set when a clue was just found; main shows a note */
uint8_t clue_total(void) BANKED;

#endif
