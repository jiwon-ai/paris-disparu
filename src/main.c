/* PARIS DISPARU - main loop and game states. */
#include <gb/gb.h>
#include <gb/cgb.h>
#include <stdint.h>
#include "assets.h"
#include "actor.h"
#include "dialogue.h"
#include "events.h"
#include "game.h"
#include "input.h"
#include "sound.h"
#include "world.h"

enum { ST_WALK, ST_DIALOGUE, ST_PAUSE };

static uint8_t state;
static uint8_t cur_dialogue;
static uint8_t caption_timer;
static uint8_t ambience_timer;
static uint8_t rng = 0x5A;

/* Title screen colours: dark paper, cream ink (palette 5 is reused). */
static const palette_color_t title_palette[4] = {
    RGB(3, 3, 8), RGB(3, 3, 8), RGB(12, 10, 9), RGB(30, 27, 19)
};

static uint8_t row_buf[MAP_W];

static uint8_t next_random(void) {
    /* 8-bit Galois LFSR, deterministic from boot. */
    rng = (rng >> 1) ^ ((rng & 1) ? 0xB8 : 0);
    return rng;
}

static void bkg_fill_row(uint8_t y, uint8_t tile, uint8_t attr) {
    uint8_t x;
    for (x = 0; x < MAP_W; x++) row_buf[x] = attr;
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_tiles(0, y, MAP_W, 1, row_buf);
    VBK_REG = VBK_TILES;
    for (x = 0; x < MAP_W; x++) row_buf[x] = tile;
    set_bkg_tiles(0, y, MAP_W, 1, row_buf);
}

static uint8_t text_len(const uint8_t *t) {
    uint8_t n = 0;
    while (t[n]) n++;
    return n;
}

static void bkg_text_centered(uint8_t y, const uint8_t *t) {
    uint8_t n = text_len(t);
    set_bkg_tiles((MAP_W - n) / 2, y, n, 1, t);
}

static void load_graphics(void) {
    VBK_REG = VBK_TILES;
    set_sprite_data(0, SPRITE_TILE_COUNT, sprite_tiles);
    set_bkg_data(FONT_BASE, FONT_TILE_COUNT, font_tiles);
    VBK_REG = VBK_ATTRIBUTES;          /* bank 1: scenery tiles */
    set_bkg_data(0, ENV_TILE_COUNT, env_tiles);
    VBK_REG = VBK_TILES;
}

/* Shown if the cartridge is started on a monochrome Game Boy. */
static void dmg_screen(void) {
    uint8_t y;
    set_bkg_data(FONT_BASE, FONT_TILE_COUNT, font_tiles);
    for (y = 0; y < MAP_H; y++) {
        uint8_t x;
        for (x = 0; x < MAP_W; x++) row_buf[x] = GLYPH_SPACE;
        set_bkg_tiles(0, y, MAP_W, 1, row_buf);
    }
    bkg_text_centered(7, txt_dmg_1);
    bkg_text_centered(9, txt_dmg_2);
    SHOW_BKG;
    DISPLAY_ON;
    while (1) wait_vbl_done();
}

static void title_screen(void) {
    uint8_t y, t = 0;

    set_bkg_palette(0, 8, bg_palettes);
    set_bkg_palette(5, 1, title_palette);
    for (y = 0; y < 15; y++) bkg_fill_row(y, GLYPH_SPACE, 5);
    bkg_fill_row(15, TILE_SKY, 7 | 0x08);
    bkg_fill_row(16, TILE_ROOFTOP, 7 | 0x08);
    bkg_fill_row(17, TILE_ROOF, 7 | 0x08);
    bkg_text_centered(4, txt_title_1);
    bkg_text_centered(6, txt_title_2);
    bkg_text_centered(7, txt_title_3);
    bkg_text_centered(10, txt_title_4);

    while (1) {
        wait_vbl_done();
        input_update();
        if (joy_pressed & (J_START | J_A)) break;
        t++;
        if ((t & 31) == 0) {
            if (t & 32) bkg_text_centered(13, txt_title_5);
            else {
                for (y = 0; y < 5; y++) row_buf[y] = GLYPH_SPACE;
                set_bkg_tiles((MAP_W - 5) / 2, 13, 5, 1, row_buf);
            }
        }
        next_random();   /* time spent on the title varies the ambience */
    }
    sound_confirm();
}

static void enter_room(uint8_t room) {
    actors_hide();
    world_load(room);
    actors_init_room();
    player_unstick();
    event_enter_room(room);
    ambience_timer = 20;
    actors_draw();
}

/* exit_code from player_update(): a screen edge DIR_* or PLAYER_DOOR | index. */
static void change_room(uint8_t exit_code) {
    uint8_t dest;
    if (exit_code & PLAYER_DOOR) {
        const door_t *d = &rooms[cur_room].doors[exit_code & 0x3F];
        dest = d->room;
        player_place(d->to_x, d->to_y, d->to_dir);
    } else {
        dest = rooms[cur_room].exits[exit_code];
        switch (exit_code) {
        case DIR_LEFT:  player.x = SCREEN_W - 13 - 1; break;
        case DIR_RIGHT: player.x = -2; break;
        case DIR_UP:    player.y = SCREEN_H - 16 - 1; break;
        default:        player.y = -9; break;
        }
    }
    enter_room(dest);
}

static void open_dialogue(uint8_t d) {
    caption_timer = 0;
    cur_dialogue = d;
    dialogue_start(d);
    state = ST_DIALOGUE;
}

/* Scene-based ambience: short noise bursts at irregular intervals. */
static void update_ambience(void) {
    uint8_t r;
    if (ambience_timer) {
        ambience_timer--;
        return;
    }
    r = next_random();
    switch (rooms[cur_room].ambience) {
    case AMBIENCE_STREET:   /* louder towards the east end, where the market is */
        sound_market(1 + (uint8_t)(player.x >> 5));
        ambience_timer = 40 + (r & 63);
        break;
    case AMBIENCE_MARKET:
        sound_market(5 + (r & 1));
        ambience_timer = 12 + (r & 31);
        break;
    case AMBIENCE_HALL:
        sound_market(3 + (r & 1));
        ambience_timer = 16 + (r & 31);
        break;
    case AMBIENCE_CAFE:
        sound_blip();       /* a cup on a saucer */
        ambience_timer = 90 + r;
        break;
    default:
        ambience_timer = 255;
    }
}

void main(void) {
    DISPLAY_OFF;
    if (_cpu != CGB_TYPE) dmg_screen();

    sound_init();
    load_graphics();
    SPRITES_8x16;
    SHOW_BKG;
    SHOW_SPRITES;
    HIDE_WIN;
    DISPLAY_ON;

    title_screen();

    player_place(28, 72, DIR_RIGHT);
    enter_room(ROOM_STREET);
    box_open(3);
    box_print(1, 1, txt_caption);
    caption_timer = 150;
    state = ST_WALK;

    while (1) {
        wait_vbl_done();
        input_update();

        switch (state) {
        case ST_WALK:
            if (joy_pressed & J_START) {
                caption_timer = 0;
                box_open(4);
                box_print(7, 1, txt_pause_1);
                box_print(2, 2, txt_pause_2);
                sound_confirm();
                state = ST_PAUSE;
                break;
            }
            if (joy_pressed & J_A) {
                uint8_t id = player_interact();
                if (id != NO_DIALOGUE) {
                    uint8_t d = event_examine(id);
                    if (d != NO_DIALOGUE) {
                        open_dialogue(d);
                        break;
                    }
                }
            }
            {
                uint8_t exit_dir = player_update();
                if (exit_dir != 0xFF) change_room(exit_dir);
            }
            if (caption_timer && --caption_timer == 0) box_close();
            update_ambience();
            break;

        case ST_DIALOGUE:
            if (!dialogue_update()) {
                uint8_t next = event_after(cur_dialogue, dialogue_choice);
                if (next != NO_DIALOGUE) open_dialogue(next);
                else state = ST_WALK;
            }
            break;

        case ST_PAUSE:
            if (joy_pressed & (J_START | J_B)) {
                box_close();
                state = ST_WALK;
            }
            break;
        }
        actors_draw();
    }
}
