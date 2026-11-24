#ifndef KO_H
#define KO_H

#include <gb/gb.h>
#include <stdint.h>

/* 0 = English, 1 = Korean (chosen after the title). */
extern uint8_t lang;

/* Bank 0 helper: copy n bytes from another ROM bank, then map the caller's
   bank back. */
void far_read(uint8_t bank, const uint8_t *src, uint8_t *dst, uint8_t n);

/* src/ko.c, ROM bank 6. */
void ko_start(uint8_t id) BANKED;       /* first page of a dialogue, in RAM */
uint8_t ko_last(void) BANKED;           /* 1 on the last page */
void ko_next(void) BANKED;
uint8_t ko_reveal(void) BANKED;         /* one more column; 0 when done */
uint8_t ko_done(void) BANKED;
void ko_reveal_all(void) BANKED;
void ko_clear(void) BANKED;             /* blank the 4 text rows */
void ko_ui_reset(uint8_t full) BANKED;  /* new box: empty syllable cache */
uint8_t ko_print(uint8_t x, uint8_t y, uint16_t off) BANKED; /* UI string at ui_text+off */
void ko_draw_choice(uint8_t sel) BANKED;
void ko_draw_name(uint8_t x, uint8_t y) BANKED;

#endif
