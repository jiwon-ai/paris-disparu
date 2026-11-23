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

/* Persistent story flags (game_flags bits) */
#define FLAG_MET_STRANGER   0x0001
#define FLAG_MET_PORTER     0x0002
#define FLAG_TASK_ACTIVE    0x0004
#define FLAG_TASK_DONE      0x0008
#define FLAG_VISITED_MARKET 0x0010
#define FLAG_MET_MERCHANT   0x0020
#define FLAG_MET_CAFE       0x0040
#define FLAG_SAW_PHONE      0x0080
#define FLAG_HAS_JETON      0x0100
#define FLAG_PHONE_DONE     0x0200
#define FLAG_FERNAND_TOLD   0x0400  /* Simone heard Fernand sent you */
#define FLAG_MET_WORKER     0x0800

extern uint16_t game_flags;

#endif
