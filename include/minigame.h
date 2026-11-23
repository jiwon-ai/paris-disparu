#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdint.h>

#define MG_RUNNING 0
#define MG_SUCCESS 1
#define MG_FAIL    2   /* lift: missed / dial: cancelled with B */

/* Lifting a crate: a cursor sweeps a bar, press A inside the zone.
   level 0-2 makes the zone smaller and the cursor faster. */
void lift_start(uint8_t level);
uint8_t lift_update(void);

/* Rotary-style dialling: Up/Down changes a digit, Left/Right moves, A calls. */
void dial_start(void);
uint8_t dial_update(void);
uint16_t dial_number(void);   /* the 4 digits as a decimal number */

#endif
