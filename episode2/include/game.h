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

/* Characters (Episode 2, night of 27-28 February 1969) */
#define NPC_FERNAND     0
#define NPC_ROUX        1
#define NPC_PORTER      2   /* Dédé: the cup game, for francs */
#define NPC_MUSICIAN    3   /* the accordion on rue Baltard */
#define NPC_DANCER      4
#define NPC_CAFE        5   /* Simone: jetons */
#define NPC_FLOWER      6   /* flower seller, Cité */
#define NPC_PETANQUE    7   /* old man, Place Dauphine: pétanque */
#define NPC_PETANQUE2   8
#define NPC_CLOCHARDE   9   /* old woman with the pigeons, Notre-Dame */
#define NPC_STUDENT     10  /* Saint-Michel */
#define NPC_DOORMAN     11  /* the jazz cellar door */
#define NPC_TRUMPET     12  /* inside: the rhythm game */
#define NPC_OLDPOET     13  /* Saint-Germain terrace */
#define NPC_WAITER      14
#define NPC_PHILOSOPHER 15  /* cameo */
#define NPC_LULU        16  /* the boy in the red scarf, under the bridge */
#define NPC_SLEEPER     17
#define NPC_TICKETS     18  /* the métro ticket window (hotspot) */
#define NPC_TRAMP       19  /* random encounter */

/* Story flags (game_flags) */
#define F_MET_FERNAND  0x0001
#define F_TICKET       0x0002   /* holds a métro ticket */
#define F_JETON        0x0004   /* holds a telephone jeton */
#define F_VIOLET       0x0008   /* bought a violet */
#define F_MUTE         0x0010   /* found the trumpet mute in the fountain */
#define F_INSIDE       0x0020   /* the doorman let you in */
#define F_AZALEA       0x0040
#define F_PAVE         0x0080
#define F_NUMBER       0x0100   /* the paper from Lulu's jacket (ROQ 2271) */
#define F_CARD         0x0200   /* the bookshop card (ODÉ 6120) */
#define F_MATCHES      0x0400   /* the cellar's matchbook (MED 3301) */
#define F_POET1        0x0800   /* answered the old poet's first line */
#define F_SOLVED       0x1000   /* plate given back: the dawn plays */
#define F_LULU_HOME    0x2000   /* told Lulu his mother called */
#define F_PHONE_SEEN   0x4000
#define F_SHARD        0x8000   /* the blue shard (also a secret) */
extern uint16_t game_flags;

/* What the poet has found out (clue_flags), one bit per quest. */
#define K_TRAIL  0x01   /* Mme Roux: a boy in a red scarf, gone by métro */
#define K_NAME   0x02   /* pétanque: Lucien, "the Sparrow", a fort's son */
#define K_HIDE   0x04   /* jazz cellar: he sleeps under the bridge */
#define K_HEART  0x08   /* the old poet: why he took it */
#define K_PLATE  0x10   /* Lulu gave the plate back */
#define K_READY  (K_NAME | K_HIDE | K_HEART)
#define CLUE_COUNT 5
extern uint8_t clue_flags;

/* Side goals (extra_flags) */
#define X_MOTHER    0x01   /* phoned Lulu's mother, then told him */
#define X_TRAMP     0x02   /* gave a franc to a tramp */
#define X_THIEF     0x04   /* caught a pickpocket */
#define X_VIOLET    0x08   /* gave the violet to the old woman */
#define X_CALLED    0x10   /* (step) phoned his mother */
#define X_POET_SING 0x20   /* (not a goal) "the city sings" */
#define X_POET_YOUNG 0x40  /* (not a goal) "the night is young" */
#define X_COUNT     4
#define X_ALL       (X_MOTHER | X_TRAMP | X_THIEF | X_VIOLET)
extern uint8_t extra_flags;
extern uint16_t cameo_met;   /* bit k: NPC k talked to once */
extern uint8_t new_extra;
uint8_t extra_total(void) BANKED;

#define SECRET_COUNT 10
#define SECRET_BELLS 8     /* examine the bell tower 3 times */
#define SECRET_INITIALS 9  /* (hotspot) */
extern uint16_t secret_flags;
extern uint8_t new_secret;
uint8_t secret_total(void) BANKED;

/* Francs in the poet's pocket. */
#define MONEY_MAX 9
extern uint8_t money;
extern uint8_t new_money;      /* +1 just earned: show it */

/* Album of vanished Paris: bit i = found (see notebook.c). */
#define ALBUM_COUNT 6
#define AL_TICKET 0x01
#define AL_JETON  0x02
#define AL_AZALEA 0x04
#define AL_PAVE   0x08
#define AL_SHARD  0x10
#define AL_PLATE  0x20
extern uint8_t album_flags;
extern uint8_t new_album;      /* 1: something for the album */
uint8_t album_total(void) BANKED;

/* Quests: Q0 the trail, Q1 his name, Q2 where he sleeps, Q3 why, Q4 the
   plate, Q5 dawn. Each of Q0-Q4 is done by its K_ bit. */
#define QUEST_COUNT 6
extern uint8_t quest_flags;     /* unused bits, kept for the save */
extern uint8_t new_quest;       /* 1-5: that quest was just completed */
uint8_t quest_done(uint8_t q) BANKED;
extern uint8_t new_clue;
uint8_t clue_total(void) BANKED;

/* Métro: the stops of line 4 open tonight. */
#define STOP_HALLES   0
#define STOP_CITE     1
#define STOP_STMICHEL 2
#define STOP_STGERMAIN 3
#define STOP_COUNT    4

#endif
