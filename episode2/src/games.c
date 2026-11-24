/* Quest mini-games: the café coffee rush, the porter's cup game, catching
   Germaine's cabbages and chasing the thief on the quai. Each returns
   MG_RUNNING until it ends with MG_SUCCESS or MG_FAIL (see minigame.h).
   ROM bank 7: UI strings go through box_print, which maps bank 1 itself. */
#pragma bank 7
#include <gb/gb.h>
#include "actor.h"
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "items.h"
#include "minigame.h"
#include "sound.h"
#include "world.h"

static uint8_t seed;
static uint8_t rnd(void) {          /* small LFSR, mixed with the divider */
    seed = (seed >> 1) ^ (-(seed & 1) & 0xB8);
    return seed ^ DIV_REG;
}

static void put_digit(uint8_t x, uint8_t y, uint8_t n) {
    box_tile(x, y, GLYPH_DIGIT0 + n);
}

/* ------------------------------------------------------------ cup game
   A jeton goes under one of three cups; two cups at a time slide past each
   other (one above, one below), so the eye can follow the jeton's cup; then
   pick the right one. */
#define CUP_Y 3
static const uint8_t cup_x[3] = { 4, 9, 14 };
uint8_t cup_where;
static uint8_t c_swaps, c_phase, c_timer, c_a, c_b, c_sel, c_k;

static void cups_draw(uint8_t show) {
    uint8_t i;
    for (i = 0; i < 3; i++) {
        box_tile(cup_x[i], CUP_Y - 1, GLYPH_SPACE);
        box_tile(cup_x[i], CUP_Y, GLYPH_CUP);
        box_tile(cup_x[i], CUP_Y + 1, (show && i == cup_where) ? GLYPH_COIN : GLYPH_SPACE);
        box_tile(cup_x[i] - 1, CUP_Y + 2, (c_phase == 2 && i == c_sel) ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    }
}

/* The two sliding cups, c_k columns from their start: cup c_a above, c_b below. */
static void cups_slide(uint8_t t) {
    box_tile(cup_x[c_a] + c_k, CUP_Y - 1, t);
    box_tile(cup_x[c_b] - c_k, CUP_Y + 1, t);
}

void cups_start(void) BANKED {
    seed = DIV_REG | 1;
    cup_where = rnd() % 3;
    c_swaps = 0;
    c_phase = 0;                        /* 0 show, 1 shuffle, 2 pick */
    c_timer = 90;
    c_sel = 1;
    c_a = 0xFF;
    box_open(7);
    box_print(1, 1, txt_cup_watch);
    cups_draw(1);
}

uint8_t cups_update(void) BANKED {
    if (c_phase == 0) {
        if (--c_timer == 0) { cups_draw(0); c_phase = 1; c_timer = 20; }
        return MG_RUNNING;
    }
    if (c_phase == 1) {
        if (--c_timer) return MG_RUNNING;
        if (c_a == 0xFF) {              /* start a swap: lift two cups off the row */
            c_a = rnd() % 3;
            c_b = (c_a + 1 + (rnd() & 1)) % 3;
            if (c_a > c_b) { c_k = c_a; c_a = c_b; c_b = c_k; }
            c_k = 0;
            box_tile(cup_x[c_a], CUP_Y, GLYPH_SPACE);
            box_tile(cup_x[c_b], CUP_Y, GLYPH_SPACE);
            cups_slide(GLYPH_CUP);
        } else if (cup_x[c_a] + c_k < cup_x[c_b]) {   /* one column further */
            cups_slide(GLYPH_SPACE);
            c_k++;
            cups_slide(GLYPH_CUP);
        } else {                        /* arrived: back on the row, swapped */
            if (cup_where == c_a) cup_where = c_b;
            else if (cup_where == c_b) cup_where = c_a;
            c_a = 0xFF;
            cups_draw(0);
            sound_clink();
            if (++c_swaps == 7) {
                c_phase = 2;
                box_clear(1);
                box_print(1, 1, txt_cup_pick);
                cups_draw(0);
                return MG_RUNNING;
            }
            c_timer = 12;
            return MG_RUNNING;
        }
        c_timer = 4 - c_swaps / 3;      /* frames per column: faster and faster */
        return MG_RUNNING;
    }
    if (joy_pressed & J_LEFT)  { c_sel = c_sel ? c_sel - 1 : 2; cups_draw(0); sound_blip(); }
    if (joy_pressed & J_RIGHT) { c_sel = c_sel == 2 ? 0 : c_sel + 1; cups_draw(0); sound_blip(); }
    if (joy_pressed & J_A) {
        box_close();
        return c_sel == cup_where ? MG_SUCCESS : MG_FAIL;
    }
    return MG_RUNNING;
}

uint8_t cups_answer(void) BANKED { return cup_where; }   /* for the tests */

/* ------------------------------------------------------------ the thief
   Dawn on the quai: a pickpocket snatches Fernand's brass plate and runs.
   Catch him (touch him) within CHASE_SECONDS. He is a little slower than
   the player and always runs away from you, so corner him. No fighting:
   you catch his sleeve and he drops the plate. Both stay on the quai's
   pavement and road (CH_* bounds), so nobody leaves the room. */
#define CHASE_OAM 16
#define CHASE_SECONDS 30
#define CH_X1 144      /* the street in front of the facades, above the box */
#define CH_Y0 84
#define CH_Y1 100
int16_t thief_x, thief_y;            /* read by tests/play_test.py */
static uint8_t t_dir, t_hold, t_left, t_tick;
static uint16_t t_time;
static const int8_t t_dx[4] = { 0, 0, -1, 1 };   /* DIR_DOWN, UP, LEFT, RIGHT */
static const int8_t t_dy[4] = { 1, -1, 0, 0 };

static uint8_t thief_free(int16_t x, int16_t y) {
    const room_def_t *r = &rooms[cur_room];
    const npc_def_t *n;
    uint8_t i;
    if (x < 0 || x > CH_X1 || y < CH_Y0 || y > CH_Y1) return 0;
    for (i = 0; i < r->npc_count; i++) {        /* people block him too */
        n = &r->npcs[i];
        if (x + 12 >= n->x + 2 && x + 3 <= n->x + 13 && y + 15 >= n->y + 8 && y + 10 <= n->y + 15)
            return 0;
    }
    return !world_solid(x + 3, y + 10) && !world_solid(x + 12, y + 10) &&
           !world_solid(x + 3, y + 15) && !world_solid(x + 12, y + 15);
}

static uint16_t away(int16_t x, int16_t y) {     /* distance to the player */
    int16_t dx = x - player.x, dy = y - player.y;
    return (uint16_t)(dx < 0 ? -dx : dx) + (uint16_t)(dy < 0 ? -dy : dy);
}

static void chase_hud(void) {
    put_digit(16, 1, t_left / 10);
    put_digit(17, 1, t_left % 10);
}

void chase_start(void) BANKED {
    seed = DIV_REG | 1;
    if (player.y < CH_Y0) player.y = CH_Y0;
    if (player.y > CH_Y1) player.y = CH_Y1;
    thief_x = player.x < 72 ? player.x + 40 : player.x - 40;
    thief_y = player.y;
    t_dir = DIR_RIGHT;
    t_hold = 0;
    t_time = 0;
    t_tick = 0;
    t_left = CHASE_SECONDS;
    box_open(3);
    box_print(1, 1, txt_chase_hud);
    chase_hud();
}

uint8_t chase_update(void) BANKED {
    uint8_t i, n, best;
    uint16_t v, bv;
    int16_t dx, dy, nx, ny;

    player_update();                   /* exits ignored: the bounds keep him in */
    if (player.y < CH_Y0) player.y = CH_Y0;
    if (player.y > CH_Y1) player.y = CH_Y1;
    if (player.x > CH_X1) player.x = CH_X1;

    /* Pick a new direction now and then, or when blocked: follow each free
       direction up to 24 px (until something blocks it) and take the one
       that ends furthest from the player, so he avoids dead ends. */
    if (t_hold) t_hold--;
    if (!t_hold || !thief_free(thief_x + t_dx[t_dir], thief_y + t_dy[t_dir])) {
        best = t_dir;
        bv = 0;
        for (i = 0; i < 4; i++) {
            nx = thief_x;
            ny = thief_y;
            for (n = 0; n < 24 && thief_free(nx + t_dx[i], ny + t_dy[i]); n++) {
                nx += t_dx[i];
                ny += t_dy[i];
            }
            if (!n) continue;
            v = away(nx, ny) + n + (rnd() & 7);
            if (v > bv) {
                bv = v;
                best = i;
            }
        }
        t_dir = best;
        t_hold = 12;
    }
    if ((t_time & 7) && thief_free(thief_x + t_dx[t_dir], thief_y + t_dy[t_dir])) {
        thief_x += t_dx[t_dir];        /* 7 steps in 8 frames: a bit slower */
        thief_y += t_dy[t_dir];
    }

    dx = thief_x - player.x;
    dy = thief_y - player.y;
    if (dx > -12 && dx < 12 && dy > -10 && dy < 10) {
        move_sprite(CHASE_OAM, 0, 0);
        move_sprite(CHASE_OAM + 1, 0, 0);
        box_close();
        return MG_SUCCESS;
    }
    if (++t_tick == 60) {
        t_tick = 0;
        t_left--;
        chase_hud();
        if (t_left == 0) {
            move_sprite(CHASE_OAM, 0, 0);
            move_sprite(CHASE_OAM + 1, 0, 0);
            box_close();
            return MG_FAIL;
        }
        if (t_left <= 5) sound_blip();
    }
    t_time++;
    sprite16(CHASE_OAM, thief_x, thief_y, (t_time & 8) ? SPR1_THIEF_1 : SPR1_THIEF_0,
             4 | S_BANK1 | (t_dir == DIR_LEFT ? S_FLIPX : 0));
    return MG_RUNNING;
}
