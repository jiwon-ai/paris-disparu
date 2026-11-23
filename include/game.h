#ifndef GAME_H
#define GAME_H

#include <stdint.h>

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
#define FLAG_FERNAND_TOLD   0x0400  /* Simone heard Fernand sent you */
#define FLAG_MET_WORKER     0x0800
#define FLAG_SOLVED         0x1000  /* Fernand found and answered: the ending plays */
#define FLAG_MET_PHOTO      0x4000
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
extern uint8_t new_clue;   /* set when a clue was just found; main shows a note */
uint8_t clue_total(void);

#endif
