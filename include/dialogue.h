#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>

/* Text box drawn on the window layer at the bottom of the screen. */
void box_open(uint8_t rows);
void box_close(void);
void box_print(uint8_t x, uint8_t y, const uint8_t *text); /* 0-terminated */

/* Paged dialogue with typewriter effect.
   A: finish page / next page. B: close at any time. */
void dialogue_start(uint8_t id);
uint8_t dialogue_update(void);  /* returns 1 while the dialogue is open */

#endif
