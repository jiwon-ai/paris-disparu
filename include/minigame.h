#ifndef MINIGAME_H
#define MINIGAME_H

#include <gb/gb.h>
#include <stdint.h>

#define MG_RUNNING 0
#define MG_SUCCESS 1
#define MG_FAIL    2   /* lift: missed / dial: cancelled with B */

/* Lifting a crate: a cursor sweeps a bar, press A inside the zone.
   level 0-2 makes the zone smaller and the cursor faster. */
void lift_start(uint8_t level);
void timing_start(uint8_t level, const uint8_t *line1, const uint8_t *line2);
uint8_t lift_update(void);

/* Rotary-style dialling: Up/Down changes a digit, Left/Right moves, A calls. */
void dial_start(void);
uint8_t dial_update(void);
uint16_t dial_number(void);   /* the 4 digits as a decimal number */

/* Quest mini-games (games.c). */
void serve_start(void) BANKED;
uint8_t serve_update(void) BANKED;
void cups_start(void) BANKED;
uint8_t cups_update(void) BANKED;
uint8_t cups_answer(void) BANKED;
void catch_start(void) BANKED;
uint8_t catch_update(void) BANKED;

/* The thief on the quai (games.c): catch him before the time runs out. */
extern int16_t thief_x, thief_y;
void chase_start(void) BANKED;
uint8_t chase_update(void) BANKED;

#endif
