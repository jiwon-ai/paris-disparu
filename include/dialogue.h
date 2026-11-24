#ifndef DIALOGUE_H
#define DIALOGUE_H

#include <stdint.h>

/* Text box drawn on the window layer at the bottom of the screen. */
void box_open(uint8_t rows);
void box_close(void);
void box_print(uint8_t x, uint8_t y, const uint8_t *text); /* 0-terminated */
void box_tile(uint8_t x, uint8_t y, uint8_t t);   /* one tile in the box (Korean: laid out) */
void box_clear(uint8_t y);   /* blank one row of the box (Korean: both of its rows) */
void box_flush(void);    /* end of a full-screen box: Korean lays it out */
uint8_t box_has_next(void);   /* Korean long box: not on its last page */
uint8_t box_more(void);  /* Korean long box: next page (1), else 0 */
uint8_t digit_glyph(uint8_t n);   /* font tile of digit n, callable from any bank */

/* Paged dialogue with typewriter effect.
   A: finish page / next page. B: close at any time. */
void dialogue_start(uint8_t id);
uint8_t dialogue_update(void);  /* returns 1 while the dialogue is open */
void bark_show(uint8_t id);      /* non-blocking one-page line */
extern uint8_t lang_fr;         /* 1: French text (the English engine) */
extern uint8_t dialogue_choice; /* 0 = first option, 1 = second or B on the options,
                                   2 = closed with B before the options */

#endif
