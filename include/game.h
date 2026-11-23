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

/* Persistent story flags (game_flags bits) */
#define FLAG_MET_PORTER 0x01

extern uint8_t game_flags;

#endif
