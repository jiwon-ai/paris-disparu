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

/* ------------------------------------------------------------ coffee rush
   Pour a coffee: hold A and the cup fills; let go while it is in the dark
   zone. Too little or spilt over: try the next cup. 3 cups, 2 good to win.
   (Replaced the three-item order game, 2026-11-24: players found it hard.) */
#define SERVE_ROUNDS 3
#define SERVE_NEED   2
#define CUP_LEN      16              /* bar cells, columns 2-17 */
#define ZONE_LO      10              /* good cells 10-13 */
#define ZONE_HI      13
static uint8_t s_round, s_hits, s_fill, s_tick, s_pause, s_pouring;

static void serve_draw_bar(void) {
    uint8_t i, t;
    for (i = 0; i < CUP_LEN; i++) {
        if (i < s_fill) t = GLYPH_BAR_CURSOR;                  /* coffee */
        else if (i >= ZONE_LO && i <= ZONE_HI) t = GLYPH_BAR_ZONE;
        else t = GLYPH_BAR;
        box_tile(2 + i, 3, t);
    }
}

static void serve_order(void) {
    s_fill = 0;
    s_tick = 0;
    s_pouring = 0;
    box_clear(4);
    box_print(1, 4, txt_srv_hint);
    serve_draw_bar();
}

void serve_start(void) BANKED {
    seed = DIV_REG | 1;
    s_round = s_hits = s_pause = 0;
    box_open(6);
    box_print(1, 1, txt_srv_title);
    put_digit(16, 1, 0);
    box_print(17, 1, txt_srv_of);
    box_print(1, 2, txt_srv_hold);
    box_tile(1, 3, GLYPH_CUP);          /* the cup, filling to the right */
    serve_order();
}

static uint8_t serve_next(uint8_t hit) {
    if (hit) { s_hits++; sound_confirm(); } else sound_clink();
    put_digit(16, 1, s_hits);
    box_clear(4);
    box_print(1, 4, hit ? txt_srv_ok : txt_srv_bad);
    s_pause = 45;                       /* show the result a moment */
    return MG_RUNNING;
}

uint8_t serve_update(void) BANKED {
    if (s_pause) {
        if (--s_pause) return MG_RUNNING;
        if (++s_round == SERVE_ROUNDS || s_hits >= SERVE_NEED
                || s_hits + SERVE_ROUNDS - s_round < SERVE_NEED) {
            box_close();
            return s_hits >= SERVE_NEED ? MG_SUCCESS : MG_FAIL;
        }
        serve_order();
        return MG_RUNNING;
    }
    if (joy_held & J_A) {
        s_pouring = 1;
        if (++s_tick >= 8) {            /* one cell every 8 frames: slow and steady */
            s_tick = 0;
            if (++s_fill > CUP_LEN - 1) return serve_next(0);   /* spilt over */
            serve_draw_bar();
            if ((s_fill & 1) == 0) sound_blip();
        }
    } else if (s_pouring) {             /* let go: judge the level */
        return serve_next(s_fill > ZONE_LO && s_fill <= ZONE_HI + 1);
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
        box_tile(cup_x[i], CUP_Y - 1, up ? GLYPH_CUP : GLYPH_SPACE);
        box_tile(cup_x[i], CUP_Y, up ? GLYPH_SPACE : GLYPH_CUP);
        box_tile(cup_x[i], CUP_Y + 1, (show && i == cup_where) ? GLYPH_COIN : GLYPH_SPACE);
        box_tile(cup_x[i] - 1, CUP_Y + 2, (c_phase == 2 && i == c_sel) ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    }
}

void cups_start(void) BANKED {
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

uint8_t cups_update(void) BANKED {
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
                box_clear(1);
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

uint8_t cups_answer(void) BANKED { return cup_where; }   /* for the tests */

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

void catch_start(void) BANKED {
    seed = DIV_REG | 1;
    k_caught = k_missed = 0;
    k_speed = 1;
    player_place(72, CATCH_Y, DIR_UP);
    box_open(3);
    box_print(1, 1, txt_catch_hud);
    catch_hud();
    cabbage_new();
}

uint8_t catch_update(void) BANKED {
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

/* ------------------------------------------------------------ the thief
   Dawn on the quai: a pickpocket snatches Fernand's brass plate and runs.
   Catch him (touch him) within CHASE_SECONDS. He is a little slower than
   the player and always runs away from you, so corner him. No fighting:
   you catch his sleeve and he drops the plate. Both stay on the quai's
   pavement and road (CH_* bounds), so nobody leaves the room. */
#define CHASE_OAM 16
#define CHASE_SECONDS 30
#define CH_X1 144      /* the whole quai, below Mme Roux */
#define CH_Y0 48
#define CH_Y1 92
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
    player_place(40, 64, DIR_RIGHT);
    thief_x = 80;
    thief_y = 64;
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
