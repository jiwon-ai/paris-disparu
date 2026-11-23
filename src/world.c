#include <gb/gb.h>
#include <gb/cgb.h>
#include "assets.h"
#include "game.h"
#include "world.h"

uint8_t cur_room;

static const palette_color_t black[32] = { 0 };

void world_load(uint8_t room) {
    cur_room = room;
    /* Black out, rewrite the map while invisible, then restore colours. */
    wait_vbl_done();
    set_bkg_palette(0, 8, black);
    set_sprite_palette(0, 8, black);
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_attrs[room]);
    VBK_REG = VBK_TILES;
    set_bkg_tiles(0, 0, MAP_W, MAP_H, room_tiles[room]);
    wait_vbl_done();
    set_bkg_palette(0, 8, bg_palettes);
    set_sprite_palette(0, 8, spr_palettes);
}

uint8_t world_solid(int16_t px, int16_t py) {
    if (px < 0 || py < 0 || px >= SCREEN_W || py >= SCREEN_H) return 1;
    return room_solid[cur_room][(uint16_t)(py >> 3) * MAP_W + (uint8_t)(px >> 3)];
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
            return h->dlg;
    }
    return NO_DIALOGUE;
}
