/* French option: text lookups for the English engine (dialogue.c).
   Lives in FR_BANK with the tables; pages are in FR_PAGE_BANK.. (far_read). */
#pragma bank 11
#include <gb/gb.h>
#include "assets.h"
#include "fr.h"

void far_read(uint8_t bank, const uint8_t *src, uint8_t *dst, uint8_t n);

/* The French string of the UI key at offset `off` of ui_text, drawn at x, y. */
void fr_print(uint8_t x, uint8_t y, uint16_t off) BANKED {
    const uint8_t *t = fr_ui_text + fr_ui_at[off];
    uint8_t n = 0;
    while (t[n] && x + n < MAP_W) n++;
    set_win_tiles(x, y, n, 1, t);
}

void fr_pages_of(uint8_t id, uint16_t *first, uint8_t *count) BANKED {
    *first = fr_first_page[id];
    *count = fr_page_count[id];
}

void fr_load_page(uint16_t page, uint8_t *dst) BANKED {
    uint8_t k = page / FR_PAGES_PER_BANK;
    far_read(FR_PAGE_BANK + k, fr_page_base[k] + (page - k * FR_PAGES_PER_BANK) * PAGE_SIZE, dst, PAGE_SIZE);
}

void fr_choice(uint8_t pair, uint8_t *dst) BANKED {   /* pair = index + 1; 14 bytes */
    const uint8_t *t = fr_choice_text + (uint16_t)(pair - 1) * 14;
    uint8_t i;
    for (i = 0; i < 14; i++) dst[i] = t[i];
}
