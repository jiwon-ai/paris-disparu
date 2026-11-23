/* Three quest mini-games: the café coffee rush, the porter's cup game and
   catching Germaine's cabbages. Each returns MG_RUNNING until it ends with
   MG_SUCCESS or MG_FAIL (see minigame.h). */
#include <gb/gb.h>
#include "actor.h"
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "minigame.h"
#include "sound.h"

static uint8_t seed;
static uint8_t rnd(void) {          /* small LFSR, mixed with the divider */
    seed = (seed >> 1) ^ (-(seed & 1) & 0xB8);
    return seed ^ DIV_REG;
}

static void put_digit(uint8_t x, uint8_t y, uint8_t n) {
    set_win_tile_xy(x, y, txt_digits[n]);
}

/* ------------------------------------------------------------ coffee rush
   Table n wants one of three things before the bar runs out. 5 orders,
   4 right to win. */
#define SERVE_ROUNDS 5
#define SERVE_NEED   4
static const uint8_t * const srv_item[3] = { txt_srv_0, txt_srv_1, txt_srv_2 };
static const uint8_t srv_x[3] = { 2, 9, 14 };
static uint8_t s_round, s_want, s_sel, s_hits, s_cells, s_tick, s_speed, s_pause;

static void serve_draw_sel(void) {
    uint8_t i;
    for (i = 0; i < 3; i++)
        set_win_tile_xy(srv_x[i] - 1, 4, i == s_sel ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
}

static void serve_draw_bar(void) {
    uint8_t i;
    for (i = 0; i < 16; i++) set_win_tile_xy(2 + i, 3, i < s_cells ? GLYPH_BAR_ZONE : GLYPH_BAR);
}

static void serve_order(void) {
    uint8_t i;
    s_want = rnd() % 3;
    s_cells = 16;
    s_tick = 0;
    s_speed = 7 - s_round;            /* frames per cell: faster each order */
    for (i = 1; i < 19; i++) set_win_tile_xy(i, 2, GLYPH_SPACE);
    box_print(1, 2, txt_srv_table);
    put_digit(7, 2, 1 + rnd() % 8);
    box_print(9, 2, srv_item[s_want]);
    serve_draw_bar();
}

void serve_start(void) {
    uint8_t i;
    seed = DIV_REG | 1;
    s_round = s_hits = s_sel = s_pause = 0;
    box_open(6);
    box_print(1, 1, txt_srv_title);
    for (i = 0; i < 3; i++) box_print(srv_x[i], 4, srv_item[i]);
    put_digit(16, 1, 0);
    box_print(17, 1, txt_srv_of);
    serve_draw_sel();
    serve_order();
}

static uint8_t serve_next(uint8_t hit) {
    uint8_t i;
    if (hit) { s_hits++; sound_confirm(); } else sound_clink();
    for (i = 1; i < 19; i++) set_win_tile_xy(i, 2, GLYPH_SPACE);
    put_digit(16, 1, s_hits);
    box_print(1, 2, hit ? txt_srv_ok : txt_srv_bad);
    s_pause = 30;                       /* show the result a moment */
    return MG_RUNNING;
}

uint8_t serve_update(void) {
    if (s_pause) {
        if (--s_pause) return MG_RUNNING;
        if (++s_round == SERVE_ROUNDS) {
            box_close();
            return s_hits >= SERVE_NEED ? MG_SUCCESS : MG_FAIL;
        }
        serve_order();
        return MG_RUNNING;
    }
    if (joy_pressed & J_LEFT)  { s_sel = s_sel ? s_sel - 1 : 2; serve_draw_sel(); sound_blip(); }
    if (joy_pressed & J_RIGHT) { s_sel = s_sel == 2 ? 0 : s_sel + 1; serve_draw_sel(); sound_blip(); }
    if (joy_pressed & J_A) return serve_next(s_sel == s_want);
    if (++s_tick >= s_speed) {
        s_tick = 0;
        if (--s_cells == 0) return serve_next(0);   /* too slow */
        serve_draw_bar();
    }
    return MG_RUNNING;
}

uint8_t serve_hits(void) { return s_hits; }

/* ------------------------------------------------------------ cup game
   A jeton goes under one of three cups; the cups swap places; pick the
   right one. The two cups about to swap are shown raised. */
#define CUP_Y 3
static const uint8_t cup_x[3] = { 4, 9, 14 };
uint8_t cup_where;
static uint8_t c_swaps, c_phase, c_timer, c_a, c_b, c_sel;

static void cups_draw(uint8_t raise_a, uint8_t raise_b, uint8_t show) {
    uint8_t i, up;
    for (i = 0; i < 3; i++) {
        up = (i == raise_a || i == raise_b);
        set_win_tile_xy(cup_x[i], CUP_Y - 1, up ? txt_cup[0] : GLYPH_SPACE);
        set_win_tile_xy(cup_x[i], CUP_Y, up ? GLYPH_SPACE : txt_cup[0]);
        set_win_tile_xy(cup_x[i], CUP_Y + 1, (show && i == cup_where) ? txt_coin[0] : GLYPH_SPACE);
        set_win_tile_xy(cup_x[i] - 1, CUP_Y + 2, (c_phase == 2 && i == c_sel) ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    }
}

void cups_start(void) {
    seed = DIV_REG | 1;
    cup_where = rnd() % 3;
    c_swaps = 0;
    c_phase = 0;                        /* 0 show, 1 shuffle, 2 pick */
    c_timer = 60;
    c_sel = 1;
    box_open(7);
    box_print(1, 1, txt_cup_watch);
    cups_draw(0xFF, 0xFF, 1);
}

uint8_t cups_update(void) {
    if (c_phase == 0) {
        if (--c_timer == 0) { c_phase = 1; c_timer = 1; }
        return MG_RUNNING;
    }
    if (c_phase == 1) {
        if (--c_timer) return MG_RUNNING;
        if (c_a != 0xFF) {              /* finish the shown swap */
            if (cup_where == c_a) cup_where = c_b;
            else if (cup_where == c_b) cup_where = c_a;
            c_a = 0xFF;
            cups_draw(0xFF, 0xFF, 0);
            sound_clink();
            c_timer = 8;
            if (++c_swaps == 7) {
                c_phase = 2;
                box_print(1, 1, txt_cup_pick);
                cups_draw(0xFF, 0xFF, 0);
            }
            return MG_RUNNING;
        }
        c_a = rnd() % 3;
        c_b = (c_a + 1 + (rnd() & 1)) % 3;
        cups_draw(c_a, c_b, 0);
        c_timer = 22 - c_swaps * 2;     /* faster and faster */
        return MG_RUNNING;
    }
    if (joy_pressed & J_LEFT)  { c_sel = c_sel ? c_sel - 1 : 2; cups_draw(0xFF, 0xFF, 0); sound_blip(); }
    if (joy_pressed & J_RIGHT) { c_sel = c_sel == 2 ? 0 : c_sel + 1; cups_draw(0xFF, 0xFF, 0); sound_blip(); }
    if (joy_pressed & J_A) {
        box_close();
        return c_sel == cup_where ? MG_SUCCESS : MG_FAIL;
    }
    return MG_RUNNING;
}

uint8_t cups_answer(void) { return cup_where; }   /* for the tests */

/* ------------------------------------------------------------ cabbages
   Germaine's cabbages roll off the top of the stall: move left/right under
   them. 6 caught wins, 3 dropped loses. Drawn with the crate sprite. */
#define CATCH_OAM 16   /* free: player 0-1, NPCs 2-9, crate 10-11, cat 12-13 */
#define CATCH_Y 100
#define CATCH_NEED 6
#define CATCH_LIVES 3
int16_t cab_x, cab_y;
static uint8_t k_caught, k_missed, k_speed;

static void catch_hud(void) {
    put_digit(5, 1, k_caught);
    put_digit(16, 1, k_missed);
}

static void cabbage_new(void) {
    cab_x = 24 + (rnd() % 112);
    cab_y = 20;
}

void catch_start(void) {
    seed = DIV_REG | 1;
    k_caught = k_missed = 0;
    k_speed = 1;
    player_place(72, CATCH_Y, DIR_UP);
    box_open(3);
    box_print(1, 1, txt_catch_hud);
    catch_hud();
    cabbage_new();
}

uint8_t catch_update(void) {
    uint8_t i;
    if ((joy_held & J_LEFT) && player.x > 24) player.x -= 2;
    if ((joy_held & J_RIGHT) && player.x < 136) player.x += 2;
    player.dir = DIR_UP;
    for (i = 0; i < k_speed; i++) cab_y++;
    if (cab_y >= CATCH_Y - 6 && cab_y <= CATCH_Y + 2 &&
        cab_x - player.x < 12 && player.x - cab_x < 12) {
        k_caught++;
        sound_confirm();
        if (k_caught == 3 || k_caught == 5) k_speed++;
        catch_hud();
        cabbage_new();
    } else if (cab_y > CATCH_Y + 14) {
        k_missed++;
        sound_clink();
        catch_hud();
        cabbage_new();
    }
    if (k_caught >= CATCH_NEED || k_missed >= CATCH_LIVES) {
        move_sprite(CATCH_OAM, 0, 0);
        move_sprite(CATCH_OAM + 1, 0, 0);
        box_close();
        return k_caught >= CATCH_NEED ? MG_SUCCESS : MG_FAIL;
    }
    sprite16(CATCH_OAM, cab_x, cab_y, SPR_CRATE, 5);
    return MG_RUNNING;
}
