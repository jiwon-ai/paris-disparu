#ifndef KO_H
#define KO_H

#include <gb/gb.h>
#include <stdint.h>

/* 0 = English, 1 = Korean (chosen after the title). */
extern uint8_t lang;

/* Bank 0 helper: copy n bytes from another ROM bank, then map the caller's
   bank back. */
void far_read(uint8_t bank, const uint8_t *src, uint8_t *dst, uint8_t n);
void far_bkg_data(uint8_t bank, uint8_t first, uint8_t n, const uint8_t *src);

/* src/ko.c, ROM bank 6. */
void ko_start(uint8_t id) BANKED;       /* first page of a dialogue, in RAM */
uint8_t ko_last(void) BANKED;           /* 1 on the last page */
void ko_next(void) BANKED;
uint8_t ko_reveal(void) BANKED;         /* one more column; 0 when done */
uint8_t ko_done(void) BANKED;
void ko_reveal_all(void) BANKED;
void ko_clear(void) BANKED;             /* blank the 4 text rows */
/* Boxes: see src/ko.c. mode 1 small box, 2 full screen, 4 inside a dialogue. */
extern uint8_t ko_mode, ko_borrowed;
void ko_ui_reset(uint8_t mode) BANKED;
uint8_t ko_print(uint8_t x, uint8_t y, uint16_t off, const uint8_t *en) BANKED;
void ko_tile(uint8_t x, uint8_t y, uint8_t t) BANKED;
uint8_t ko_small_rows(uint8_t rows) BANKED;
void ko_clear_row(uint8_t y) BANKED;
void ko_flush(void) BANKED;             /* lay out a full-screen box */
uint8_t ko_more(void) BANKED;           /* next page of a long full box; 0 = single page */
uint8_t ko_has_next(void) BANKED;       /* not on its last page yet */
void ko_draw_choice(uint8_t sel) BANKED;
void ko_draw_name(uint8_t x, uint8_t y) BANKED;

#endif
