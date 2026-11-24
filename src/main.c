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
#include "items.h"
#include "minigame.h"
#include "notebook.h"
#include "sound.h"
#include "music.h"
#include "ko.h"
#include "world.h"

enum { ST_WALK, ST_DIALOGUE, ST_NOTEBOOK, ST_LIFT, ST_DIAL, ST_END, ST_GAME, ST_HOWTO, ST_DAWN };

static uint8_t state;
static uint8_t cur_dialogue;
static uint8_t caption_note;    /* the caption is a quest/clue/secret note */
#define CAPTION_FRAMES 900      /* the opening caption stays 15 s unless A/B closes it */
#define NO_BARK 0xFF
static uint16_t caption_timer;  /* opening caption: closes when it reaches 0 */
static uint8_t bark_from;       /* NPC index whose line is on screen, or NO_BARK */
static uint8_t bark_armed;      /* bit n: NPC n of this room may call out again */
static uint16_t ambience_timer;

/* The cafe cat: sits in the street at the start and, when the player comes
   near, darts away along the pavement to the cafe door and slips inside.
   A first thing to do that needs no reading, and it shows the way. */
#define CAT_OAM 12
static const int16_t cat_path[][2] = { { 56, 92 }, { 44, 92 }, { 36, 84 }, { 30, 76 }, { 30, 62 } };
#define CAT_STEPS 5
int16_t cat_x = 56, cat_y = 92;
static void close_caption(void);
static uint8_t lift_punch;
static uint8_t teaser_seen;
static uint8_t game_ev;          /* EV_SERVE / EV_CUPS / EV_CATCH while in ST_GAME */
static uint8_t fanfare;          /* frames left of the quest-complete jingle */
static uint8_t howto_seen;       /* bit (EV_* - EV_FIRST): rules already shown */
static uint8_t sparkle;          /* frames left of the "got it!" star over the player */
static uint8_t shake;            /* frames left of a small screen shake */
#define SPARKLE_OAM 20           /* free: games use 16-17, portraits 24+ */
#define SPARKLE_FRAMES 96
#define COIN_OAM 30              /* the glint of a coin on the ground */

/* Juice: the star rises and blinks over the player's head; the screen
   shakes by a pixel or two. Both are cosmetic and stop by themselves. */
static uint8_t anim_frame;
static void update_juice(void) {
    if (sparkle) {
        sparkle--;
        if (state != ST_WALK || !sparkle || (sparkle & 4)) {
            move_sprite(SPARKLE_OAM, 0, 0);
            move_sprite(SPARKLE_OAM + 1, 0, 0);
            if (state != ST_WALK) sparkle = 0;
        } else {
            sprite16(SPARKLE_OAM, player.x, player.y - 16 - ((SPARKLE_FRAMES - sparkle) >> 4),
                     SPR1_SPARKLE, 7 | S_BANK1);
        }
    }
    {   /* a coin on the ground glints now and then */
        uint8_t c = state == ST_WALK ? coin_here() : 0xFF;
        if (c != 0xFF && (anim_frame & 63) < 10)
            sprite16(COIN_OAM, coin_x(c) * 8 - 4, coin_y(c) * 8 - 4, SPR1_SPARKLE, 7 | S_BANK1);
        else {
            move_sprite(COIN_OAM, 0, 0);
            move_sprite(COIN_OAM + 1, 0, 0);
        }
    }
    if (shake) {
        shake--;
        SCX_REG = (shake & 2) ? 2 : 0;
        SCY_REG = (shake & 4) ? 1 : 0;
    }
}
static const uint16_t fanfare_x[4] = { 1985, 1964, 1949, 1923 };   /* C7 G6 E6 C6, played backwards */

static const uint8_t * const quest_name[QUEST_COUNT - 1] = {
    txt_q_0, txt_q_1, txt_q_2, txt_q_3
};
static const uint8_t * const quest_got[QUEST_COUNT - 1] = {
    txt_q_0r, txt_q_1r, txt_q_2r, txt_q_3r
};      /* the timing bar is the metro punch, not a crate */
static uint8_t cat_wp, cat_run;

static void update_cat(void) {
    int16_t tx, ty;
    if (cur_room == ROOM_CAFE && (game_flags & FLAG_CAT_GONE)) {   /* asleep in the corner */
        sprite16(CAT_OAM, 4, 118, SPR_CAT_SIT, 7);
        return;
    }
    if (cur_room != ROOM_STREET || (game_flags & FLAG_CAT_GONE)) {
        move_sprite(CAT_OAM, 0, 0);
        move_sprite(CAT_OAM + 1, 0, 0);
        return;
    }
    if (!cat_run) {
        int16_t dx = player.x - cat_x, dy = player.y - cat_y;
        if (dx > -28 && dx < 28 && dy > -28 && dy < 28) {
            cat_run = 1;
            sound_clink();                 /* a startled little mew */
        }
    }
    if (cat_run) {
        tx = cat_path[cat_wp][0];
        ty = cat_path[cat_wp][1];
        if (cat_x < tx) cat_x++; else if (cat_x > tx) cat_x--;
        if (cat_y < ty) cat_y++; else if (cat_y > ty) cat_y--;
        if (cat_x == tx && cat_y == ty) {
            if (++cat_wp == CAT_STEPS) {   /* in through the cafe door */
                game_flags |= FLAG_CAT_GONE;
                close_caption();
                box_open(4);
                box_print(1, 1, txt_cat_gone);
                box_print(1, 2, txt_cat_gone_2);
                caption_timer = 240;
                caption_note = 1;
                sound_clink();
                return;
            }
            cat_run = cat_wp < 2;          /* pauses on the way: catch up! */
        }
    }
    sprite16(CAT_OAM, cat_x, cat_y, cat_run ? SPR_CAT_RUN : SPR_CAT_SIT, 7);
}
static uint8_t rng = 0x5A;
static uint8_t barge_x;


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

/* VRAM bank 1: scenery tiles and dialogue portraits (the title art borrows it). */
static void load_bank1(void) {
    VBK_REG = VBK_ATTRIBUTES;
    SWITCH_ROM(PORTRAIT_BANK);
    set_bkg_data(0, ENV_TILE_COUNT, env_tiles);
    set_sprite_data(PORTRAIT_BASE, PORTRAIT_TILE_COUNT, portrait_tiles);
    set_sprite_data(0, LOW_FIXED, low_tiles);   /* thief, sparkle: bank 1, 0x8000 */
    SWITCH_ROM(ASSET_BANK);
    VBK_REG = VBK_TILES;
}

static void load_graphics(void) {
    VBK_REG = VBK_TILES;
    set_sprite_data(0, SPRITE_TILE_COUNT, sprite_tiles);
    set_bkg_data(FONT_BASE, FONT_TILE_COUNT, font_tiles);
    set_sprite_data(254, 2, bubble_tiles);
    load_bank1();
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

#define TITLE_TEXT_ROW 16

/* Copy one row of the title art back (it lives in ROM bank TITLE_BANK). */
static void title_art_row(uint8_t y) {
    SWITCH_ROM(TITLE_BANK);
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_tiles(0, y, MAP_W, 1, title_attr + y * MAP_W);
    VBK_REG = VBK_TILES;
    set_bkg_tiles(0, y, MAP_W, 1, title_map + y * MAP_W);
    SWITCH_ROM(ASSET_BANK);
}

/* Title palettes scaled by level/4 (0 = black): a short fade-in. */
static palette_color_t fade_buf[32];
static void title_fade(uint8_t level) {
    uint8_t i;
    SWITCH_ROM(TITLE_BANK);
    for (i = 0; i < 32; i++) {
        uint16_t c = title_pal[i];
        uint8_t r = c & 31, g = (c >> 5) & 31, b = (c >> 10) & 31;
        fade_buf[i] = (uint16_t)((r * level) >> 2) | ((uint16_t)((g * level) >> 2) << 5)
                      | ((uint16_t)((b * level) >> 2) << 10);   /* CGB 15-bit BGR */
    }
    SWITCH_ROM(ASSET_BANK);
    fade_buf[5 * 4 + 1] = fade_buf[5 * 4];   /* font text paper = night */
    set_bkg_palette(0, 8, fade_buf);
}

/* English, French or Korean, chosen once after the title. */
static void lang_menu(void) {
    uint8_t sel = 0, i;
    box_open(SCREEN_H / 8);
    box_print(6, 5, txt_lang_title);
    box_print(7, 8, txt_lang_en);
    box_print(7, 10, txt_lang_fr);
    ko_draw_name(7, 12);
    box_print(6, 15, txt_lang_go);
    while (1) {
        for (i = 0; i < 3; i++) box_tile(5, 8 + i * 2, sel == i ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
        wait_vbl_done();
        input_update();
        if (joy_pressed & (J_DOWN | J_SELECT)) { sel = sel == 2 ? 0 : sel + 1; sound_blip(); }
        if (joy_pressed & J_UP) { sel = sel ? sel - 1 : 2; sound_blip(); }
        if (joy_pressed & (J_A | J_START)) break;
    }
    sound_confirm();
    lang = sel == 2;        /* Korean: its own text engine (src/ko.c) */
    lang_fr = sel == 1;
}

/* Setting, goal and controls on three short pages. A: next, B/START: skip. */
static void intro(void) {
    uint8_t page = 0;
    intro_show(0);
    while (1) {
        wait_vbl_done();
        input_update();
        if (joy_pressed & (J_B | J_START)) break;
        if (joy_pressed & J_A) {
            sound_blip();
            if (box_has_next()) { box_more(); continue; }   /* Korean: rest of the page */
            if (++page == 3) break;
            intro_show(page);
        }
    }
    box_close();
}

/* Save/continue: one slot in battery-backed cartridge RAM (MBC5+RAM+BATTERY).
   Written after every room change and every closed dialogue; checked with a
   magic word and a checksum, so blank or random RAM reads as "no save". */
#define SAVE_SIZE (12 + EVENT_SAVE_SIZE + 1)
static uint8_t save_buf[SAVE_SIZE];
static const uint8_t save_magic[4] = { 'P', 'D', 'S', '4' };

static uint8_t save_sum(void) {
    uint8_t i, s = 0xA5;
    for (i = 0; i < SAVE_SIZE - 1; i++) s += save_buf[i];
    return s;
}

static void save_write(void) {
    uint8_t i;
    for (i = 0; i < 4; i++) save_buf[i] = save_magic[i];
    save_buf[4] = cur_room;
    save_buf[5] = (uint8_t)player.x;  save_buf[6] = (uint8_t)((uint16_t)player.x >> 8);
    save_buf[7] = (uint8_t)player.y;  save_buf[8] = (uint8_t)((uint16_t)player.y >> 8);
    save_buf[9] = player.dir;
    save_buf[10] = howto_seen;
    save_buf[11] = music_muted;
    event_pack(save_buf + 12);
    save_buf[SAVE_SIZE - 1] = save_sum();
    ENABLE_RAM;
    SWITCH_RAM(0);
    for (i = 0; i < SAVE_SIZE; i++) ((volatile uint8_t *)0xA000)[i] = save_buf[i];
    DISABLE_RAM;
}

static uint8_t save_read(void) {    /* 1 when a valid save is in save_buf */
    uint8_t i;
    ENABLE_RAM;
    SWITCH_RAM(0);
    for (i = 0; i < SAVE_SIZE; i++) save_buf[i] = ((volatile uint8_t *)0xA000)[i];
    DISABLE_RAM;
    for (i = 0; i < 4; i++) if (save_buf[i] != save_magic[i]) return 0;
    return save_buf[SAVE_SIZE - 1] == save_sum();
}

static void save_erase(void) {      /* the story is over: next start is a new game */
    ENABLE_RAM;
    SWITCH_RAM(0);
    *(volatile uint8_t *)0xA000 = 0;
    DISABLE_RAM;
}

/* Continue or new game, shown only when a save exists. 1 = continue. */
static uint8_t continue_menu(void) {
    uint8_t sel = 0;
    box_open(SCREEN_H / 8);
    box_print(7, 7, txt_save_cont);
    box_print(7, 9, txt_save_new);
    box_print(6, 14, txt_lang_go);
    box_flush();
    while (1) {
        box_tile(5, 7, sel ? GLYPH_SPACE : GLYPH_ARROW_RIGHT);
        box_tile(5, 9, sel ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
        wait_vbl_done();
        input_update();
        if (joy_pressed & (J_UP | J_DOWN | J_SELECT)) {
            sel ^= 1;
            sound_blip();
        }
        if (joy_pressed & (J_A | J_START)) break;
    }
    sound_confirm();
    box_close();
    return sel == 0;
}

/* Key art: a night view of Les Halles (tools/title_art.py). Its tiles use
   VRAM bank 1, so the scenery tiles are reloaded afterwards. */
static void title_screen(void) {
    uint8_t y, t = 0;

    title_fade(0);
    SWITCH_ROM(TITLE_BANK);
    VBK_REG = VBK_ATTRIBUTES;
    set_bkg_data(0, TITLE_TILE_COUNT, title_tiles);
    VBK_REG = VBK_TILES;
    SWITCH_ROM(ASSET_BANK);
    for (y = 0; y < MAP_H; y++) title_art_row(y);
    for (y = 1; y <= 4; y++) {
        uint8_t f;
        for (f = 0; f < 8; f++) wait_vbl_done();
        title_fade(y);
    }
    music_play(SONG_TITLE);
    bkg_fill_row(MAP_H - 1, GLYPH_SPACE, 5);      /* creator credit, bottom row */
    bkg_text_centered(MAP_H - 1, txt_title_6);

    while (1) {
        wait_vbl_done();
        input_update();
        if (joy_pressed) break;     /* any button starts: no guessing which one */
        t++;
        if ((t & 31) == 0) {
            if (t & 32) {
                bkg_fill_row(TITLE_TEXT_ROW, GLYPH_SPACE, 5);
                bkg_text_centered(TITLE_TEXT_ROW, txt_title_5);
            } else {
                title_art_row(TITLE_TEXT_ROW);
            }
        }
        next_random();   /* time spent on the title varies the ambience */
    }
    sound_confirm();
    load_bank1();                      /* scenery and portraits back into bank 1 */
}

static void close_caption(void) {
    if (caption_timer || bark_from != NO_BARK) box_close();
    caption_timer = 0;
    bark_from = NO_BARK;
}

/* Dawn creeps in with each quest done (the clock runs 3 a.m. to 7 a.m.). */
static uint8_t dawn_now(void) {
    uint8_t i, n = 0;
    for (i = 0; i < 4; i++) n += quest_done(i);
    return n;
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
    save_write();
}

/* Start whatever an interaction returned: text, or an interactive sequence. */
static uint8_t howto_ev;         /* mini-game waiting behind its rules card */
static uint16_t idle;            /* frames without input, for the goal reminder */

static void open_dialogue(uint8_t d);
static void start_game(uint8_t d) {
    howto_seen |= 1 << (d - EV_FIRST);
    open_dialogue(d);
}

/* "!" over the person to talk to next, "A" over the player when A would do
   something. OAM 14 and 15, tiles in VRAM bank 1. */
#define MARK_OAM 14
static void update_markers(void) {
    uint8_t i, who = goal_npc(), bob = (sys_time >> 4) & 1;
    const room_def_t *r = &rooms[cur_room];
    move_sprite(MARK_OAM, 0, 0);
    move_sprite(MARK_OAM + 1, 0, 0);
    if (state != ST_WALK) return;
    for (i = 0; i < r->npc_count; i++) {
        const npc_def_t *n = &r->npcs[i];
        if (n->npc == who) {
            set_sprite_tile(MARK_OAM, SPR1_MARK);
            set_sprite_prop(MARK_OAM, 3 | S_BANK1);
            move_sprite(MARK_OAM, n->x + 12, n->y + 2 + bob);
        }
    }
    if (!sparkle && player_interact() != NO_DIALOGUE) {   /* the star has the spot */
        set_sprite_tile(MARK_OAM + 1, SPR1_PRESS);
        set_sprite_prop(MARK_OAM + 1, 3 | S_BANK1);
        move_sprite(MARK_OAM + 1, player.x + 12, player.y + 2);
    }
}

static void open_dialogue(uint8_t d) {
    caption_timer = 0;           /* the dialogue box replaces any caption */
    bark_from = NO_BARK;
    if (d >= EV_FIRST && d != EV_END && d != NO_DIALOGUE && !(howto_seen & (1 << (d - EV_FIRST)))) {
        howto_ev = d;            /* first time: the rules, then A to start */
        howto_show(d);
        state = ST_HOWTO;
        return;
    }
    if (d == EV_LIFT || d == EV_PUNCH) {
        lift_punch = d == EV_PUNCH;
        if (lift_punch) timing_start(1, txt_punch_1, txt_punch_2);
        else lift_start(event_lift_level());
        music_play(SONG_GAME);
        state = ST_LIFT;
    } else if (d == EV_DIAL) {
        dial_start();
        state = ST_DIAL;
    } else if (d == EV_SERVE || d == EV_CUPS || d == EV_CATCH || d == EV_CHASE) {
        game_ev = d;
        music_play(SONG_GAME);
        if (d == EV_SERVE) serve_start();
        else if (d == EV_CUPS) cups_start();
        else if (d == EV_CHASE) { chase_start(); shake = 16; }   /* the bump */
        else catch_start();
        state = ST_GAME;
    } else if (d == EV_END) {    /* daybreak on the quai, then the end screen */
        save_erase();
        box_close();
        music_play(SONG_TITLE);
        player_place(72, 60, DIR_DOWN);
        enter_room(ROOM_QUAI);
        day_start();
        state = ST_DAWN;
    } else if (d == NO_DIALOGUE) {
        state = ST_WALK;
        save_write();
        if (new_quest) {         /* the big moment: a quest is done */
            uint8_t q = new_quest - 1;
            new_quest = 0;
            new_clue = 0;
            box_open(6);
            box_print(1, 1, txt_quest_done);
            box_print(1, 2, quest_name[q]);
            box_print(1, 3, quest_got[q]);
            box_print(1, 4, txt_poem_new);     /* each quest: a line of the poem */
            world_dawn(dawn_now());        /* an hour later: the sky pales */
            caption_timer = 300;
            caption_note = 1;
            fanfare = 32;
            sparkle = SPARKLE_FRAMES;
        } else if (new_clue) {   /* tell the player the notebook changed */
            new_clue = 0;
            box_open(5);
            box_print(1, 1, txt_new_clue);
            goal_print(2);                 /* what to do next, at a glance */
            caption_timer = 240;
            caption_note = 1;
            sound_confirm();
            sparkle = SPARKLE_FRAMES;
        } else if (new_curio) {  /* a vanished object found, or explained */
            uint8_t solved = new_curio == 2;
            new_curio = 0;
            box_open(4);
            box_print(1, 1, solved ? txt_cu_solved : txt_cu_found);
            if (solved) {
                box_tile(16, 1, txt_digits[curio_total()]);
                box_print(17, 1, txt_cu_of);
            }
            box_print(1, 2, solved ? txt_cu_solved2 : txt_cu_found2);
            caption_timer = 240;
            caption_note = 1;
            fanfare = 16;
            sparkle = SPARKLE_FRAMES;
        } else if (new_extra | new_secret) {   /* a side goal done, or a secret */
            uint8_t sec = new_secret;
            new_extra = new_secret = 0;
            box_open(4);
            box_print(1, 1, sec ? txt_secret_done : txt_extra_done);
            box_tile(15, 1, txt_digits[sec ? secret_total() : extra_total()]);
            box_print(16, 1, sec ? txt_secret_of : txt_extra_of);
            box_print(1, 2, txt_extra_more);
            caption_timer = 240;
            caption_note = 1;
            if (sec) fanfare = 16;         /* a little two-note "ding" */
            else sound_confirm();
            sparkle = SPARKLE_FRAMES;
        }
        if (item_note()) {       /* the object itself, over the note or alone */
            caption_timer = 240;
            caption_note = 1;
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
    if (has_water && !ko_borrowed && (anim_frame & 31) == 0) {   /* not while Korean text holds the tiles */
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

/* Lamp light breathes a little: palette 3 (street lamps) colours 2-3
   move between three close levels at random. */
static const palette_color_t lamp_glow[3][2] = {
    { RGB(28, 23, 9), RGB(31, 31, 22) },
    { RGB(27, 21, 8), RGB(31, 29, 19) },
    { RGB(29, 24, 10), RGB(31, 31, 24) },
};
static void flicker_lamps(void) {
    uint8_t k;
    if (anim_frame & 7) return;
    k = next_random() & 3;
    if (k == 3) k = 0;
    set_bkg_palette_entry(3, 2, lamp_glow[k][0]);
    set_bkg_palette_entry(3, 3, lamp_glow[k][1]);
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
    if (caption_timer && caption_note) return;   /* a quest/clue note stays up */
    for (i = 0; i < r->npc_count; i++) {
        const npc_def_t *n = &r->npcs[i];
        uint8_t dx = absdiff(player.x, n->x), dy = absdiff(player.y, n->y);
        uint8_t bit = 1 << i;
        if (dx < 28 && dy < 28) {
            if ((bark_armed & bit) && bark_from != i) {   /* the nearest call wins */
                uint8_t d = event_bark(n->npc);
                bark_armed &= ~bit;
                if (d == NO_DIALOGUE) continue;   /* cameos do not call out */
                caption_timer = 0;      /* a call replaces the opening caption */
                sound_blip();           /* a speech bubble, not a box (bubble_update) */
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
/* Music for each room, in ROOM_* order (tools/music.py). */
static const uint8_t room_song[ROOM_COUNT] = {
    SONG_STREET, SONG_MARKET, SONG_MARKET, SONG_CAFE, SONG_RIVER, SONG_RIVER, SONG_METRO,
    SONG_STREET, SONG_CAFE, SONG_CAFE    /* Bourse, hotel, brasserie */
};

static void update_ambience(void) {
    if (ambience_timer) ambience_timer--;
    else ambience_timer = ambience_play(next_random());
}

void main(void) {
    DISPLAY_OFF;
    LCDC_REG &= ~LCDCF_BG8000;   /* BG tiles 0-127 at 0x9000: frees 0x8000-0x8FFF for sprites */
    if (_cpu != CGB_TYPE) dmg_screen();

    sound_init();
    music_init();
    load_graphics();
    SPRITES_8x16;
    SHOW_BKG;
    SHOW_SPRITES;
    HIDE_WIN;
    DISPLAY_ON;

    title_screen();
    lang_menu();
    if (save_read() && continue_menu()) {   /* back where the player left off */
        event_unpack(save_buf + 12);
        howto_seen = save_buf[10];
        music_set_muted(save_buf[11]);
        player_place((int16_t)(save_buf[5] | ((uint16_t)save_buf[6] << 8)),
                     (int16_t)(save_buf[7] | ((uint16_t)save_buf[8] << 8)), save_buf[9]);
        dawn_stage = dawn_now();
        enter_room(save_buf[4]);
        box_open(5);
        box_print(1, 1, txt_save_back);
        goal_print(2);
    } else {
        intro();
        player_place(84, 92, DIR_LEFT);
        dawn_stage = 0;
        enter_room(ROOM_STREET);
        box_open(4);
        box_print(1, 1, txt_caption);
        box_print(1, 2, txt_caption_2);
    }
    caption_timer = CAPTION_FRAMES;
    caption_note = 0;
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
                nb_open();
                sound_confirm();
                state = ST_NOTEBOOK;
                break;
            }
            if (joy_pressed & J_SELECT) {     /* music on / off */
                music_set_muted(!music_muted);
                close_caption();
                box_open(3);
                box_print(1, 1, music_muted ? txt_music_off : txt_music_on);
                caption_timer = 120;
                caption_note = 0;
                save_write();
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
                else if ((exit_dir = coin_take(player.x + 8, player.y + 13)) != 0) {
                    close_caption();           /* a coin underfoot */
                    coin_note();
                    caption_timer = 180;
                    caption_note = 1;
                    sound_confirm();
                    sparkle = SPARKLE_FRAMES;
                    save_write();
                }
            }
            if (caption_timer && --caption_timer == 0) box_close();
            if (joy_held) idle = 0;
            else if (++idle == 600 && !caption_timer && bark_from == NO_BARK) {
                box_open(4);                   /* standing still: what to do next */
                goal_print(1);
                caption_timer = 240;
                caption_note = 0;   /* a reminder: a call may replace it */
            }
            update_barks();
            update_cat();
            update_ambience();
            music_play(room_song[cur_room]);   /* no-op while it already plays */
            break;

        case ST_HOWTO:
            if (joy_pressed & (J_A | J_START)) start_game(howto_ev);
            else if (joy_pressed & J_B) { box_close(); state = ST_WALK; }
            break;

        case ST_DIALOGUE:
            if (!dialogue_update()) {
                open_dialogue(event_after(cur_dialogue, dialogue_choice));
            }
            break;

        case ST_LIFT: {
            uint8_t r = lift_update();
            if (r != MG_RUNNING)
                open_dialogue(lift_punch ? event_punch_result(r == MG_SUCCESS)
                                         : event_lift_result(r == MG_SUCCESS));
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
            } else if (joy_pressed & J_SELECT) {          /* sound on / off, shown on page 1 */
                music_set_muted(!music_muted);
                save_write();
                nb_open();
            } else if (joy_pressed & (J_UP | J_DOWN)) {      /* Korean: long page, other half */
                if (box_more()) sound_blip();
            } else if (joy_pressed & (J_LEFT | J_RIGHT)) {   /* clues <-> extras */
                nb_turn(joy_pressed & J_RIGHT);
                sound_blip();
            }
            break;

        case ST_GAME: {
            uint8_t r;
            if (game_ev == EV_SERVE) r = serve_update();
            else if (game_ev == EV_CUPS) r = cups_update();
            else if (game_ev == EV_CHASE) r = chase_update();
            else r = catch_update();
            if (r != MG_RUNNING) open_dialogue(event_game_result(game_ev, r == MG_SUCCESS));
            break;
        }

        case ST_DAWN:         /* daybreak; A/START after the caption: the end screen */
            if (day_update(joy_pressed)) {
                end_show();
                sound_confirm();
                state = ST_END;
            }
            break;

        case ST_END:          /* START: the next episode's teaser, then the title */
            if (joy_pressed & (J_A | J_UP | J_DOWN)) {    /* Korean: long page, other half */
                if (box_more()) sound_blip();
            } else if (joy_pressed & J_START) {
                if (teaser_seen) reset();
                teaser_seen = 1;
                teaser_show();
                sound_confirm();
            }
            break;
        }
        actors_draw();
        animate_river();
        if (fanfare) {           /* four rising notes, 8 frames apart */
            fanfare--;
            if ((fanfare & 7) == 7) sound_note(fanfare_x[fanfare >> 3], fanfare >= 8 ? 0x62 : 0x65);
        }
        update_markers();
        update_juice();
        hud_update(state == ST_WALK || state == ST_DIALOGUE);   /* carried items, top-left */
        rat_update(state == ST_WALK);
        bubble_update(state == ST_WALK ? bark_from : NO_BARK);
        if (state != ST_END && state != ST_DAWN) flicker_lamps();
    }
}
