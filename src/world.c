#include <gb/gb.h>
#include <gb/cgb.h>
#include "assets.h"
#include "game.h"
#include "world.h"

uint8_t cur_room;

static const palette_color_t black[32] = { 0 };
static palette_color_t fade[64];

/* All palettes at level/4 of their brightness (bank ASSET_BANK mapped). */
static void fade_to(uint8_t level) {
    uint8_t i;
    for (i = 0; i < 64; i++) {
        uint16_t c = i < 32 ? bg_palettes[i] : spr_palettes[i - 32];
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
    /* Fade out, rewrite the map while invisible, then fade back in. */
    for (l = 3; l; l--) fade_to(l);
    wait_vbl_done();
    set_bkg_palette(0, 8, black);
    set_sprite_palette(0, 8, black);
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_attrs[room]);
    VBK_REG = VBK_TILES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_tiles[room]);
    for (l = 1; l < 4; l++) fade_to(l);
    wait_vbl_done();
    set_bkg_palette(0, 8, bg_palettes);
    set_sprite_palette(0, 8, spr_palettes);
}

uint8_t world_solid(int16_t px, int16_t py) {
    uint8_t s, save = _current_bank;     /* callable from banked code too */
    if (px < 0 || py < 0 || px >= SCREEN_W || py >= SCREEN_H) return 1;
    SWITCH_ROM(ASSET_BANK);
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
