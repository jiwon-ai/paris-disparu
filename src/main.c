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
#include "minigame.h"
#include "notebook.h"
#include "sound.h"
#include "world.h"

enum { ST_WALK, ST_DIALOGUE, ST_NOTEBOOK, ST_LIFT, ST_DIAL, ST_END };

static uint8_t state;
static uint8_t cur_dialogue;
#define CAPTION_FRAMES 900      /* the opening caption stays 15 s unless A/B closes it */
#define NO_BARK 0xFF
static uint16_t caption_timer;  /* opening caption: closes when it reaches 0 */
static uint8_t bark_from;       /* NPC index whose line is on screen, or NO_BARK */
static uint8_t bark_armed;      /* bit n: NPC n of this room may call out again */
static uint16_t ambience_timer;
static uint8_t rng = 0x5A;
static uint8_t anim_frame;
static uint8_t barge_x;

/* Title screen colours: dark paper, cream ink (palette 5 is reused). */
static const palette_color_t title_palette[4] = {
    RGB(4, 5, 11), RGB(4, 5, 11), RGB(12, 10, 9), RGB(30, 27, 19)
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
    bkg_text_centered(14, txt_title_6);

    while (1) {
        wait_vbl_done();
        input_update();
        if (joy_pressed) break;     /* any button starts: no guessing which one */
        t++;
        if ((t & 31) == 0) {
            if (t & 32) bkg_text_centered(12, txt_title_5);
            else {
                for (y = 0; y < MAP_W; y++) row_buf[y] = GLYPH_SPACE;
                set_bkg_tiles(0, 12, MAP_W, 1, row_buf);
            }
        }
        next_random();   /* time spent on the title varies the ambience */
    }
    sound_confirm();
}

static void close_caption(void) {
    if (caption_timer || bark_from != NO_BARK) box_close();
    caption_timer = 0;
    bark_from = NO_BARK;
}

static void enter_room(uint8_t room) {
    close_caption();             /* nothing from the previous room stays behind */
    actors_hide();
    world_load(room);
    actors_init_room();
    player_unstick();
    event_enter_room(room);
    bark_armed = 0xFF;
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

/* Start whatever an interaction returned: text, or an interactive sequence. */
static void open_dialogue(uint8_t d) {
    caption_timer = 0;           /* the dialogue box replaces any caption */
    bark_from = NO_BARK;
    if (d == EV_LIFT) {
        lift_start(event_lift_level());
        state = ST_LIFT;
    } else if (d == EV_DIAL) {
        dial_start();
        state = ST_DIAL;
    } else if (d == EV_END) {
        end_show();
        sound_confirm();
        state = ST_END;
    } else if (d == NO_DIALOGUE) {
        state = ST_WALK;
        if (new_clue) {          /* tell the player the notebook changed */
            new_clue = 0;
            box_open(3);
            box_print(1, 1, txt_new_clue);
            caption_timer = 240;
            sound_confirm();
        }
    } else {
        cur_dialogue = d;
        dialogue_start(d);
        state = ST_DIALOGUE;
    }
}

/* Moving water and the barge that passes under the bridge. */
static void animate_river(void) {
    uint8_t has_water = cur_room == ROOM_QUAI || cur_room == ROOM_PONT;
    anim_frame++;
    if (has_water && (anim_frame & 31) == 0) {
        VBK_REG = VBK_ATTRIBUTES;     /* scenery tiles are in VRAM bank 1 */
        set_bkg_data(TILE_WATER, 1, water_frames + ((anim_frame >> 5) & 1) * 16);
        VBK_REG = VBK_TILES;
    }
    if (cur_room == ROOM_PONT) {
        uint8_t i;
        if (anim_frame & 1) barge_x++;
        for (i = 0; i < 4; i++) {
            set_sprite_tile(36 + i, (i < 2 ? SPR_BARGE_L : SPR_BARGE_R) + (i & 1) * 2);
            set_sprite_prop(36 + i, 1 | S_PRIORITY);   /* behind the bridge */
            move_sprite(36 + i, barge_x + i * 8, 16 + 100);
        }
    } else {
        uint8_t i;
        for (i = 36; i < 40; i++) move_sprite(i, 0, 0);
    }
}

static uint8_t absdiff(int16_t a, int16_t b) {
    int16_t d = a > b ? a - b : b - a;
    return d > 255 ? 255 : (uint8_t)d;
}

/* NPCs call out a short line when the player comes close, once per approach.
   The line stays on screen until the player walks away or presses A/B. */
static void update_barks(void) {
    const room_def_t *r = &rooms[cur_room];
    uint8_t i;
    for (i = 0; i < r->npc_count; i++) {
        const npc_def_t *n = &r->npcs[i];
        uint8_t dx = absdiff(player.x, n->x), dy = absdiff(player.y, n->y);
        uint8_t bit = 1 << i;
        if (dx < 28 && dy < 28) {
            if ((bark_armed & bit) && bark_from == NO_BARK) {
                bark_armed &= ~bit;
                caption_timer = 0;      /* a call replaces the opening caption */
                bark_show(event_bark(n->npc));
                sound_blip();
                bark_from = i;
                return;
            }
        } else if (dx > 48 || dy > 48) {
            bark_armed |= bit;      /* walked away: may call out again next time */
            if (bark_from == i) close_caption();
        }
    }
}

/* Scene-based ambience: rare, quiet sounds at irregular intervals
   (one every 1-4 s), never a continuous noise. */
static void update_ambience(void) {
    uint8_t r;
    if (ambience_timer) {
        ambience_timer--;
        return;
    }
    r = next_random();
    ambience_timer = 90 + (r & 127);
    switch (rooms[cur_room].ambience) {
    case AMBIENCE_STREET:   /* only near the east end, where the market is */
        if (player.x > 96) sound_market(1);
        break;
    case AMBIENCE_MARKET:
        sound_market(2 + (r & 1));
        ambience_timer = 60 + (r & 127);
        break;
    case AMBIENCE_HALL:
        sound_market(2);
        break;
    case AMBIENCE_RIVER:
        sound_water();
        ambience_timer = 70 + (r & 63);
        break;
    case AMBIENCE_CAFE:
        sound_clink();
        ambience_timer = 180 + r;
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
    box_open(4);
    box_print(1, 1, txt_caption);
    box_print(1, 2, txt_caption_2);
    caption_timer = CAPTION_FRAMES;
    bark_from = NO_BARK;
    state = ST_WALK;

    while (1) {
        wait_vbl_done();
        input_update();

        switch (state) {
        case ST_WALK:
            if (joy_pressed & J_START) {
                caption_timer = 0;
                bark_from = NO_BARK;
                notebook_show();
                sound_confirm();
                state = ST_NOTEBOOK;
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
            if (joy_pressed & (J_A | J_B)) close_caption();   /* read it: dismiss */
            {
                uint8_t exit_dir = player_update();
                if (exit_dir != 0xFF) change_room(exit_dir);
            }
            if (caption_timer && --caption_timer == 0) box_close();
            update_barks();
            update_ambience();
            break;

        case ST_DIALOGUE:
            if (!dialogue_update()) {
                open_dialogue(event_after(cur_dialogue, dialogue_choice));
            }
            break;

        case ST_LIFT: {
            uint8_t r = lift_update();
            if (r != MG_RUNNING) open_dialogue(event_lift_result(r == MG_SUCCESS));
            break;
        }

        case ST_DIAL: {
            uint8_t r = dial_update();
            if (r == MG_SUCCESS) open_dialogue(event_dial_result(dial_number()));
            else if (r == MG_FAIL) state = ST_WALK;   /* hung up: token kept */
            break;
        }

        case ST_NOTEBOOK:
            if (joy_pressed & (J_START | J_B)) {
                box_close();
                state = ST_WALK;
            }
            break;

        case ST_END:
            if (joy_pressed & J_START) reset();   /* back to the title */
            break;
        }
        actors_draw();
        animate_river();
    }
}
