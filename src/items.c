/* Item pictures: what the player just got, drawn above the caption box. */
#pragma bank 4
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "items.h"

uint8_t new_item;

static const uint8_t * const item_name[] = {
    txt_it_0, txt_it_1, txt_it_2, txt_it_3, txt_it_4,
    txt_it_5, txt_it_6, txt_it_7, txt_it_8, txt_it_9,
};

void item_show(uint8_t it, uint8_t x, uint8_t y) BANKED {
    uint8_t i, n;
    it--;
    VBK_REG = 1;
    set_sprite_data(ITEM_TILE, 12, item_tiles + (uint16_t)it * 192);
    VBK_REG = 0;
    for (i = 0; i < 6; i++) {
        n = 24 + i;                         /* PORTRAIT_OAM */
        set_sprite_tile(n, ITEM_TILE + i * 2);
        set_sprite_prop(n, item_pal[it] | 0x08);
        move_sprite(n, x + (i >> 1) * 8, y + (i & 1) * 16);
    }
}

uint8_t item_note(void) BANKED {
    uint8_t it = new_item, own = 0;
    if (!it) return 0;
    new_item = NO_ITEM;
    if (!(LCDC_REG & LCDCF_WINON)) {        /* no caption up: a "You got" one */
        box_open(4);
        box_print(1, 1, txt_got);
        box_print(1, 2, item_name[it - 1]);
        own = 1;
    }
    item_show(it, 136, (uint8_t)(WY_REG - 16));   /* right side, standing on the box */
    return own;
}
