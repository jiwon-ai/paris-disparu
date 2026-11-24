/* Item pictures: what the player just got, drawn above the caption box. */
#pragma bank 4
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "items.h"
#include "game.h"
#include "world.h"   /* S_BANK1 */
#include "actor.h"
#include "events.h"

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
   walking or talking (Jiwon, 2026-11-24). Up to HUD_MAX icons, in VRAM bank 1
   sprite tiles 240-247 (free after the portraits), on OAM 22-23 and 32-33. */
#define HUD_MAX 4
#define HUD_TILE 240
static const uint8_t hud_oam[HUD_MAX] = { 22, 23, 32, 33 };
static const uint8_t hud_order[] = {        /* most useful first */
    ITEM_PLATE, ITEM_TICKET, ITEM_JETON, ITEM_MUTE, ITEM_VIOLET
};
static uint16_t hud_now = 0xFFFF;           /* bit ITEM_*: drawn now */

static uint16_t held(void) {
    uint16_t m = 0;
    if ((clue_flags & K_PLATE) && !(game_flags & F_SOLVED)) m |= 1 << ITEM_PLATE;
    if (game_flags & F_TICKET) m |= 1 << ITEM_TICKET;
    if (game_flags & F_JETON) m |= 1 << ITEM_JETON;
    if ((game_flags & (F_MUTE | F_INSIDE)) == F_MUTE) m |= 1 << ITEM_MUTE;
    if (game_flags & F_VIOLET) m |= 1 << ITEM_VIOLET;
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
        VBK_REG = 1;
        set_sprite_data(HUD_TILE + k * 2, 2, item_icon + (uint16_t)(it - 1) * 32);
        VBK_REG = 0;
        set_sprite_tile(hud_oam[k], HUD_TILE + k * 2);
        set_sprite_prop(hud_oam[k], item_pal[it - 1] | S_BANK1);
        move_sprite(hud_oam[k], 10 + k * 10, 18);
        k++;
    }
    for (; k < HUD_MAX; k++) move_sprite(hud_oam[k], 0, 0);
}

/* A random encounter (events.c sets encounter): a tramp or a pickpocket
   walks up to the poet from the far side of the screen. */
#define ENC_OAM 36
static int16_t enc_x, enc_y;

void encounter_place(void) BANKED {
    enc_x = player.x < 80 ? 168 : -16;
    enc_y = player.y;
}

uint8_t encounter_update(uint8_t on) BANKED {
    uint8_t who;
    if (!encounter || !on) {
        move_sprite(ENC_OAM, 0, 0);
        move_sprite(ENC_OAM + 1, 0, 0);
        return 0;
    }
    if (enc_x < player.x - 14) enc_x += 2;
    else if (enc_x > player.x + 14) enc_x -= 2;
    if (enc_y < player.y) enc_y++;
    else if (enc_y > player.y) enc_y--;
    if (encounter == 2)                  /* the thief's sprite stays loaded */
        sprite16(ENC_OAM, enc_x, enc_y, SPR1_THIEF_0, 4 | S_BANK1 | (enc_x > player.x ? S_FLIPX : 0));
    else                                 /* a tramp in an old grey coat */
        sprite16(ENC_OAM, enc_x, enc_y, SPR_STRANGER_0, 4 | (enc_x > player.x ? S_FLIPX : 0));
    if (enc_x < player.x - 14 || enc_x > player.x + 14 || enc_y != player.y) return 0;
    who = encounter;
    encounter = 0;
    move_sprite(ENC_OAM, 0, 0);
    move_sprite(ENC_OAM + 1, 0, 0);
    return who;
}

/* Coins, top right: the coin glyph and the count. Font glyphs are copied
   into VRAM bank 1 sprite tiles 236-239 (free after the portraits) as white
   ink with a dark shadow and a transparent lower half (8x16 sprites). */
#define MONEY_OAM 18
#define MONEY_TILE 236u
static uint8_t money_shown = 0xFF;

static void glyph_sprite(uint8_t dst, uint8_t glyph) {
    uint8_t t[32], i, ink, prev = 0;
    VBK_REG = 0;
    get_sprite_data(glyph, 1, t);
    for (i = 0; i < 8; i++) {
        ink = t[i * 2] & t[i * 2 + 1];                         /* font ink: colour 3 */
        t[i * 2] = (uint8_t)(((ink >> 1) | prev | (prev >> 1)) & ~ink);   /* shadow: 1 */
        t[i * 2 + 1] = ink;                                    /* ink: colour 2 */
        prev = ink;
    }
    for (i = 16; i < 32; i++) t[i] = 0;
    VBK_REG = 1;
    set_sprite_data(dst, 2, t);
    VBK_REG = 0;
}

void money_hud(uint8_t on) BANKED {
    if (!on) {
        move_sprite(MONEY_OAM, 0, 0);
        move_sprite(MONEY_OAM + 1, 0, 0);
        return;
    }
    if (money_shown != money) {
        if (money_shown == 0xFF) glyph_sprite(MONEY_TILE, GLYPH_COIN);
        glyph_sprite((uint8_t)(MONEY_TILE + 2), (uint8_t)(GLYPH_DIGIT0 + money));
        money_shown = money;
        set_sprite_tile(MONEY_OAM, MONEY_TILE);
        set_sprite_tile(MONEY_OAM + 1, (uint8_t)(MONEY_TILE + 2));
        set_sprite_prop(MONEY_OAM, 3 | S_BANK1);
        set_sprite_prop(MONEY_OAM + 1, 3 | S_BANK1);
    }
    move_sprite(MONEY_OAM, 148, 20);
    move_sprite(MONEY_OAM + 1, 156, 20);
}
