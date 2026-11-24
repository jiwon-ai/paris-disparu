#include <gb/gb.h>
#include <gb/cgb.h>
#include "assets.h"
#include "game.h"
#include "world.h"

uint8_t cur_room;

static const palette_color_t black[32] = { 0 };
static palette_color_t fade[64];

/* The night goes on: each quest done moves the clock an hour (03:00 to
   07:00) and the darkest colour of the outdoor palettes edges towards a
   deep pre-dawn blue. Sunrise in Paris in late November is after 08:00,
   so it never gets light: only the first hint of blue at the end. */
uint8_t dawn_stage;
static palette_color_t bg_live[32];
static const uint8_t dawn_mix[5] = { 0, 0, 2, 4, 6 };   /* eighths towards the blue */
static const uint8_t dawn_pals[5] = { 0, 2, 4, 6, 7 };   /* palettes whose colour 0 is the night */

static void build_live(void) {
    uint8_t i, m = dawn_mix[dawn_stage > 4 ? 4 : dawn_stage], save = _current_bank;
    SWITCH_ROM(ASSET_BANK);
    for (i = 0; i < 32; i++) bg_live[i] = bg_palettes[i];
    SWITCH_ROM(save);
    for (i = 0; i < 5; i++) {
        uint16_t c = bg_live[dawn_pals[i] * 4];
        uint8_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        r += (uint8_t)(((10 - r) * m) >> 3);
        g += (uint8_t)(((11 - g) * m) >> 3);
        b += (uint8_t)(((20 - b) * m) >> 3);
        bg_live[dawn_pals[i] * 4] = (uint16_t)r | ((uint16_t)g << 5) | ((uint16_t)b << 10);
    }
}

void world_dawn(uint8_t stage) {
    dawn_stage = stage;
    build_live();
    set_bkg_palette(0, 8, bg_live);
}

/* All palettes at level/4 of their brightness (bank ASSET_BANK mapped). */
static void fade_to(uint8_t level) {
    uint8_t i;
    for (i = 0; i < 64; i++) {
        uint16_t c = i < 32 ? bg_live[i] : spr_palettes[i - 32];
        uint8_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        fade[i] = (uint16_t)((r * level) >> 2) | ((uint16_t)((g * level) >> 2) << 5)
                  | ((uint16_t)((b * level) >> 2) << 10);
    }
    wait_vbl_done();
    set_bkg_palette(0, 8, fade);
    set_sprite_palette(0, 8, fade + 32);
    wait_vbl_done();
}

void world_load(uint8_t room) {
    uint8_t l;
    cur_room = room;
    build_live();
    /* Fade out, rewrite the map while invisible, then fade back in. */
    for (l = 3; l; l--) fade_to(l);
    wait_vbl_done();
    set_bkg_palette(0, 8, black);
    set_sprite_palette(0, 8, black);
    l = _current_bank;
    SWITCH_ROM(ROOM_BANK);
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_attrs[room]);
    VBK_REG = VBK_TILES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_tiles[room]);
    SWITCH_ROM(l);
    for (l = 1; l < 4; l++) fade_to(l);
    wait_vbl_done();
    set_bkg_palette(0, 8, bg_live);
    set_sprite_palette(0, 8, spr_palettes);
}

uint8_t world_solid(int16_t px, int16_t py) {
    uint8_t s, save = _current_bank;     /* callable from banked code too */
    if (px < 0 || py < 0 || px >= SCREEN_W || py >= SCREEN_H) return 1;
    SWITCH_ROM(ROOM_BANK);
    s = room_solid[cur_room][(uint16_t)(py >> 3) * MAP_W + (uint8_t)(px >> 3)];
    SWITCH_ROM(save);
    return s;
}

uint8_t world_hotspot(int16_t px, int16_t py) {
    const room_def_t *r = &rooms[cur_room];
    uint8_t tx, ty, i;
    if (px < 0 || py < 0 || px >= SCREEN_W || py >= SCREEN_H) return NO_DIALOGUE;
    tx = (uint8_t)(px >> 3);
    ty = (uint8_t)(py >> 3);
    for (i = 0; i < r->hotspot_count; i++) {
        const hotspot_t *h = &r->hotspots[i];
        if (tx >= h->x && tx < h->x + h->w && ty >= h->y && ty < h->y + h->h)
            return h->id;
    }
    return NO_DIALOGUE;
}

uint8_t world_door(int16_t px, int16_t py) {
    const room_def_t *r = &rooms[cur_room];
    uint8_t tx, ty, i;
    if (px < 0 || py < 0 || px >= SCREEN_W || py >= SCREEN_H) return NO_EXIT;
    tx = (uint8_t)(px >> 3);
    ty = (uint8_t)(py >> 3);
    for (i = 0; i < r->door_count; i++) {
        const door_t *d = &r->doors[i];
        if (tx >= d->x && tx < d->x + d->w && ty >= d->y && ty < d->y + d->h)
            return i;
    }
    return NO_EXIT;
}
