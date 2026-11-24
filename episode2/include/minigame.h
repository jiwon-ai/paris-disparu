#ifndef MINIGAME_H
#define MINIGAME_H

#include <gb/gb.h>
#include <stdint.h>

#define MG_RUNNING 0
#define MG_SUCCESS 1
#define MG_FAIL    2   /* lift: missed / dial: cancelled with B */

/* Lifting a crate: a cursor sweeps a bar, press A inside the zone.
   level 0-2 makes the zone smaller and the cursor faster. */
void lift_start(uint8_t level) BANKED;
void timing_start(uint8_t level, const uint8_t *line1, const uint8_t *line2) BANKED;
uint8_t lift_update(void) BANKED;

/* Rotary-style dialling: Up/Down changes a digit, Left/Right moves, A calls. */
void dial_start(void) BANKED;
uint8_t dial_update(void) BANKED;
uint16_t dial_number(void) BANKED;   /* the 4 digits as a decimal number */
extern uint8_t dial_exch;     /* the exchange dialled (EX_*, assets.h) */

/* Quest mini-games (games.c). */
void cups_start(void) BANKED;
uint8_t cups_update(void) BANKED;
uint8_t cups_answer(void) BANKED;

/* The thief on the quai (games.c): catch him before the time runs out. */
extern int16_t thief_x, thief_y;
void chase_start(void) BANKED;
uint8_t chase_update(void) BANKED;

/* Episode 2 mini-games (games2.c). Each begins with a short demo where
   the game plays itself ("Watch!"), then the player's turn. */
void petanque_start(void) BANKED;
uint8_t petanque_update(void) BANKED;
void jazz_start(void) BANKED;
uint8_t jazz_update(void) BANKED;
void find_start(void) BANKED;
uint8_t find_update(void) BANKED;
void toss_start(void) BANKED;
uint8_t toss_update(void) BANKED;
uint8_t toss_score(void) BANKED;
void metro_start(void) BANKED;          /* the line-4 map */
uint8_t metro_update(void) BANKED;      /* MG_SUCCESS: metro_choice is the stop */
extern uint8_t metro_choice;

#endif
