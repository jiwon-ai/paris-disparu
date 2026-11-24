/* Item pictures: what the player just got, drawn above the caption box. */
#pragma bank 4
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "items.h"
#include "game.h"

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

/* What the player carries, as small icons in the top-left corner while
   walking or talking (Jiwon, 2026-11-24). Up to HUD_MAX icons, in VRAM bank 0
   sprite tiles 246-253 (free above the font), on OAM 22-23 and 32-33. */
#define HUD_MAX 4
#define HUD_TILE 246
static const uint8_t hud_oam[HUD_MAX] = { 22, 23, 32, 33 };
static const uint8_t hud_order[] = {        /* most useful first */
    ITEM_JETON, ITEM_GRATINEE, ITEM_PLATE, ITEM_COLTIN, ITEM_BLEU, ITEM_TICKET,
    ITEM_GREEN, ITEM_ROLLER, ITEM_STUB
};
static uint16_t hud_now = 0xFFFF;           /* bit ITEM_*: drawn now */

static uint16_t held(void) {
    uint16_t m = 0;
    uint8_t i;
    if (!(game_flags & FLAG_SOLVED)) {      /* the clues go back to Fernand at the end */
        if (clue_flags & CLUE_COLTIN) m |= 1 << ITEM_COLTIN;
        if (clue_flags & CLUE_BLEU) m |= 1 << ITEM_BLEU;
        if (clue_flags & CLUE_PLATE) m |= 1 << ITEM_PLATE;
    }
    if (quest_flags & QF_SOUP) m |= 1 << ITEM_GRATINEE;
    if (game_flags & FLAG_HAS_JETON) m |= 1 << ITEM_JETON;
    if (game_flags & FLAG_TICKET) m |= 1 << ITEM_TICKET;
    for (i = 0; i < 3; i++)                 /* curiosities until explained */
        if ((curio_flags & (0x11 << i)) == (1 << i)) m |= 1 << (ITEM_GREEN + i);
    return m;
}

void hud_update(uint8_t on) BANKED {
    uint16_t m = on ? held() : 0;
    uint8_t i, k = 0, it;
    if (m == hud_now) return;
    hud_now = m;
    for (i = 0; i < sizeof hud_order && k < HUD_MAX; i++) {
        it = hud_order[i];
        if (!(m & (1 << it))) continue;
        set_sprite_data(HUD_TILE + k * 2, 2, item_icon + (uint16_t)(it - 1) * 32);
        set_sprite_tile(hud_oam[k], HUD_TILE + k * 2);
        set_sprite_prop(hud_oam[k], item_pal[it - 1]);
        move_sprite(hud_oam[k], 10 + k * 10, 18);
        k++;
    }
    for (; k < HUD_MAX; k++) move_sprite(hud_oam[k], 0, 0);
}
