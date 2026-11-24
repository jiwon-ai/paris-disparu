#ifndef FR_H
#define FR_H

#include <stdint.h>
#include <gb/gb.h>

/* French text for the English engine (src/fr.c, data from tools/fr_gen.py). */
void fr_print(uint8_t x, uint8_t y, uint16_t off) BANKED;   /* off: in ui_text */
void fr_pages_of(uint8_t id, uint16_t *first, uint8_t *count) BANKED;
void fr_load_page(uint16_t page, uint8_t *dst) BANKED;
void fr_choice(uint8_t pair, uint8_t *dst) BANKED;

#endif
