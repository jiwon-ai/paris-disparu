/* Episode 2 mini-games: pétanque, the jazz beat, finding the red scarf,
   throwing flowers at dawn, and the line-4 map. One button each. The two
   timing games start with a demo in which the game plays itself once
   ("Watch!"), so the rule is seen before it is read; every retry is a
   little easier. Each returns MG_RUNNING until MG_SUCCESS / MG_FAIL. */
#pragma bank 24
#include <gb/gb.h>
#include "actor.h"
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "minigame.h"
#include "ko.h"     /* lang */
#include "notebook.h"
#include "sound.h"
#include "world.h"

static uint8_t seed2;
static uint8_t rnd2(void) {
    seed2 = (seed2 >> 1) ^ (-(seed2 & 1) & 0xB8);
    return seed2 ^ DIV_REG;
}

#define G_SPACE GLYPH_SPACE

/* ------------------------------------------------------------ pétanque
   The court is row 3 of the box, columns 2-17; the jack (·) lies at a
   random column. A boule's shadow (▲) sweeps the power row; A stops it and
   the boule (●) lands right there. Three boules: one within a column of the
   jack wins. Each lost game makes the shadow slower. */
#define PT_C0 2
#define PT_C1 17
static uint8_t pt_tries;         /* games lost so far: slower */
static uint8_t pt_jack, pt_pos, pt_dir, pt_tick, pt_ball, pt_best, pt_pause, pt_demo, pt_wait;

static void pt_draw_bar(void) {
    uint8_t c;
    for (c = PT_C0; c <= PT_C1; c++)
        box_tile(c, 5, c == pt_pos ? GLYPH_BAR_CURSOR : (c == pt_jack ? GLYPH_BAR_ZONE : GLYPH_BAR));
}

static void pt_new_ball(void) {
    pt_pos = PT_C0;
    pt_dir = 1;
    pt_tick = 0;
    pt_wait = 30;                /* A pressed during the pause does not throw */
    box_clear(6);
    box_print(1, 6, pt_demo ? txt_g_watch : txt_pt_hint);
    box_tile(16, 1, (uint8_t)(GLYPH_DIGIT0 + pt_ball + 1));
}

void petanque_start(void) BANKED {
    uint8_t c;
    seed2 = DIV_REG | 1;
    box_open(8);
    box_print(1, 1, txt_pt_title);
    box_print(17, 1, txt_pt_of);
    pt_jack = 7 + (rnd2() % 8);
    for (c = 1; c < 19; c++) box_tile(c, 3, c == pt_jack ? GLYPH_DOT : G_SPACE);
    pt_ball = 0;
    pt_best = 99;
    pt_pause = 0;
    pt_demo = 1;                 /* the first boule is thrown by the game */
    pt_new_ball();
    pt_draw_bar();
}

uint8_t petanque_update(void) BANKED {
    uint8_t speed = 6 + (pt_tries > 3 ? 6 : pt_tries * 2), hit = 0, d;
    if (pt_pause) {
        if (--pt_pause) return MG_RUNNING;
        if (pt_demo) {           /* demo done: clear its boule, the real game */
            uint8_t c;
            pt_demo = 0;
            for (c = 1; c < 19; c++) box_tile(c, 4, G_SPACE);
            pt_new_ball();
            return MG_RUNNING;
        }
        if (pt_best <= 1 || ++pt_ball == 3) {
            box_close();
            if (pt_best <= 1) return MG_SUCCESS;
            pt_tries++;
            return MG_FAIL;
        }
        pt_new_ball();
        return MG_RUNNING;
    }
    if (++pt_tick >= speed) {
        pt_tick = 0;
        if (pt_dir) { if (++pt_pos == PT_C1) pt_dir = 0; }
        else if (--pt_pos == PT_C0) pt_dir = 1;
        pt_draw_bar();
    }
    if (pt_demo) {
        if (pt_pos == pt_jack && pt_dir) hit = 1;          /* the game presses A */
        if ((pt_tick & 8) == 0) box_tile(18, 6, GLYPH_A_BUTTON);
    } else if (pt_wait) pt_wait--;
    else if (joy_pressed & J_A) hit = 1;
    if (!hit) return MG_RUNNING;
    box_tile(pt_pos, 4, GLYPH_COIN);                       /* the boule lands */
    d = pt_pos > pt_jack ? pt_pos - pt_jack : pt_jack - pt_pos;
    if (!pt_demo && d < pt_best) pt_best = d;
    box_clear(6);
    box_print(1, 6, d == 0 ? txt_pt_kiss : d <= 1 ? txt_pt_close : txt_pt_far);
    if (d <= 1) sound_confirm(); else sound_clink();
    pt_pause = pt_demo ? 70 : 50;
    return MG_RUNNING;
}

/* ------------------------------------------------------------ jazz beat
   Notes (♪) come from the right along row 3 towards the line (|) at column
   3. A when a note is on the line or next to it. 8 notes, 5 to win. The
   first two notes are the demo. Each lost game slows the notes. */
#define JZ_LINE 3
#define JZ_NOTES 8
#define JZ_NEED 5
static uint8_t jz_tries;
static uint8_t jz_col[3];        /* up to three notes on screen; 0 = none */
static uint8_t jz_sent, jz_hits, jz_done, jz_tick, jz_gap, jz_demo, jz_flash, jz_lock;

static void jz_hud(void) {
    box_tile(15, 1, GLYPH_DIGIT0 + jz_hits);
    box_tile(17, 1, (uint8_t)(GLYPH_DIGIT0 + JZ_NEED));
}

void jazz_start(void) BANKED {
    uint8_t i;
    seed2 = DIV_REG | 1;
    box_open(8);
    box_print(1, 1, txt_jz_title);
    box_tile(16, 1, GLYPH_SLASH);
    box_tile(JZ_LINE, 2, GLYPH_ARROW_DOWN);
    box_tile(JZ_LINE, 3, GLYPH_PIPE);
    box_tile(JZ_LINE, 4, GLYPH_PIPE);
    box_print(1, 6, txt_g_watch);
    for (i = 0; i < 3; i++) jz_col[i] = 0;
    jz_sent = jz_hits = jz_done = jz_tick = jz_flash = jz_lock = 0;
    jz_gap = 1;
    jz_demo = 2;                 /* two notes played by the game */
    jz_hud();
}

static void jz_judge(uint8_t ok, uint8_t i) {
    box_tile(jz_col[i], 3, jz_col[i] == JZ_LINE ? GLYPH_PIPE : G_SPACE);
    jz_col[i] = 0;
    jz_done++;
    if (jz_demo) {
        jz_demo--;
        sound_note(1923, 0x62);
        if (!jz_demo) { box_clear(6); box_print(1, 6, txt_jz_hint); jz_done = 0; }
        return;
    }
    if (ok) { jz_hits++; sound_note(1985, 0x62); jz_flash = 12; }
    else sound_clink();
    jz_hud();
}

uint8_t jazz_update(void) BANKED {
    uint8_t i, speed = 7 + (jz_tries > 3 ? 6 : jz_tries * 2);
    if (jz_flash) {
        jz_flash--;
        box_tile(JZ_LINE, 4, (jz_flash & 4) ? GLYPH_HEART : GLYPH_PIPE);
    }
    if (!jz_demo && jz_done >= JZ_NOTES) {
        box_close();
        if (jz_hits >= JZ_NEED) return MG_SUCCESS;
        jz_tries++;
        return MG_FAIL;
    }
    if (++jz_tick >= speed) {
        jz_tick = 0;
        /* a new note every 4 or 6 steps, like a swing */
        if (--jz_gap == 0 && jz_sent < JZ_NOTES + 2) {
            for (i = 0; i < 3 && jz_col[i]; i++) ;
            if (i < 3) { jz_col[i] = 18; jz_sent++; }
            jz_gap = (rnd2() & 1) ? 4 : 6;
        }
        for (i = 0; i < 3; i++) {
            if (!jz_col[i]) continue;
            box_tile(jz_col[i], 3, G_SPACE);
            if (jz_col[i] == JZ_LINE - 1) { jz_judge(0, i); continue; }   /* missed */
            jz_col[i]--;
            box_tile(jz_col[i], 3, GLYPH_NOTE);
        }
    }
    if (jz_lock) { jz_lock--; return MG_RUNNING; }
    for (i = 0; i < 3; i++) {
        uint8_t c = jz_col[i];
        if (!c) continue;
        if (jz_demo) {
            if (c == JZ_LINE) jz_judge(1, i);
        } else if ((joy_pressed & J_A) && c <= JZ_LINE + 1) {
            jz_judge(1, i);
            return MG_RUNNING;
        }
    }
    if (!jz_demo && (joy_pressed & J_A)) {   /* a tap with no note: a short stumble */
        sound_blip();
        jz_lock = 20;
    }
    return MG_RUNNING;
}

/* ------------------------------------------------------------ find him
   Six sleepers under the bridge, drawn as sprites on the quay; the red
   scarf peeks out of one coat now and then. Left/Right moves the hand,
   A wakes that sleeper. Each wrong guess shows the scarf for longer. */
#define FD_OAM 16                 /* 16-17 hand, 24-35 the six sleepers */
#define FD_Y 92
static uint8_t fd_sel, fd_who, fd_wrong, fd_tick, fd_pause, fd_tile;
static const uint8_t fd_x[6] = { 12, 36, 60, 84, 108, 132 };

static void fd_draw(void) {
    uint8_t i, peek = fd_tick < 24 + fd_wrong * 24;
    for (i = 0; i < 6; i++)
        sprite16(24 + i * 2, fd_x[i], FD_Y, fd_tile,
                 (i == fd_who && peek ? 6 : 1) | S_BANK1 | ((i & 1) ? S_FLIPX : 0));
    sprite16(FD_OAM, fd_x[fd_sel], FD_Y - 16 - ((sys_time >> 3) & 1), SPR1_PRESS, 3 | S_BANK1);
    move_sprite(FD_OAM + 1, 0, 0);
}

void find_start(void) BANKED {
    seed2 = DIV_REG | 1;
    actors_hide();
    fd_tile = pool_tile(SPR1_SLEEPER);
    fd_who = rnd2() % 6;
    fd_sel = 0;
    fd_wrong = 0;
    fd_tick = 0;
    fd_pause = 0;
    box_open(4);
    box_print(1, 1, txt_fd_title);
    box_print(1, 2, txt_fd_hint);
}

static void fd_hide(void) {
    uint8_t i;
    move_sprite(FD_OAM, 0, 0);
    move_sprite(FD_OAM + 1, 0, 0);
    for (i = 24; i < 36; i++) move_sprite(i, 0, 0);
}

uint8_t find_update(void) BANKED {
    if (fd_pause) {
        if (--fd_pause == 0) { fd_hide(); box_close(); return MG_SUCCESS; }
        return MG_RUNNING;
    }
    if (++fd_tick >= 150) fd_tick = 0;    /* the scarf shows a moment every 2.5 s */
    if (joy_pressed & J_LEFT)  { fd_sel = fd_sel ? fd_sel - 1 : 5; sound_blip(); }
    if (joy_pressed & J_RIGHT) { fd_sel = fd_sel == 5 ? 0 : fd_sel + 1; sound_blip(); }
    if (joy_pressed & J_A) {
        box_clear(2);
        if (fd_sel == fd_who) {
            box_print(1, 2, txt_fd_found);
            sound_confirm();
            fd_tick = 0;
            fd_wrong = 5;                  /* scarf stays out */
            fd_pause = 60;
        } else {
            box_print(1, 2, txt_fd_wrong);
            sound_clink();
            if (fd_wrong < 4) fd_wrong++;
            fd_tick = 0;
        }
    }
    fd_draw();
    return MG_RUNNING;
}

/* ------------------------------------------------------------ flowers
   Dawn: dancers go round below; Left/Right moves the poet, A throws a
   flower up; a dancer under it when it comes down catches it. Eight
   flowers, nobody loses. */
#define TS_OAM 16                 /* 16-17 flower, 24-31 dancers */
#define TS_DANCERS 4
static int16_t ts_fx, ts_fy;
static int8_t ts_vy;
static uint8_t ts_left, ts_caught, ts_end, ts_tile;
static int16_t ts_dx[TS_DANCERS];
static int8_t ts_dv[TS_DANCERS];

void toss_start(void) BANKED {
    uint8_t i;
    seed2 = DIV_REG | 1;
    actors_hide();
    ts_tile = pool_tile(SPR1_DANCER);
    for (i = 0; i < TS_DANCERS; i++) {
        ts_dx[i] = 16 + i * 36;
        ts_dv[i] = (i & 1) ? -1 : 1;
    }
    ts_fy = -1;
    ts_left = 8;
    ts_caught = 0;
    ts_end = 0;
    player_place(72, 104, DIR_UP);
    box_open(3);
    box_print(1, 1, txt_ts_title);
}

uint8_t toss_score(void) BANKED { return ts_caught; }

uint8_t toss_update(void) BANKED {
    uint8_t i;
    if (ts_end) {
        if (--ts_end == 0) {
            for (i = 16; i < 32; i++) move_sprite(i, 0, 0);
            box_close();
            return MG_SUCCESS;
        }
        return MG_RUNNING;
    }
    if ((joy_held & J_LEFT) && player.x > 8) { player.x -= 2; player.dir = DIR_LEFT; }
    if ((joy_held & J_RIGHT) && player.x < 136) { player.x += 2; player.dir = DIR_RIGHT; }
    if (ts_fy < 0 && ts_left && (joy_pressed & J_A)) {
        ts_fx = player.x;
        ts_fy = player.y - 8;
        ts_vy = -6;
        ts_left--;
        sound_blip();
    }
    for (i = 0; i < TS_DANCERS; i++) {          /* round and round */
        if ((sys_time & 1) == 0) ts_dx[i] += ts_dv[i];
        if (ts_dx[i] < 4 || ts_dx[i] > 140) ts_dv[i] = -ts_dv[i];
        sprite16(24 + i * 2, ts_dx[i], 64 + (((sys_time >> 3) + i) & 1) * 2, ts_tile,
                 (i & 1 ? 6 : 2) | S_BANK1 | (ts_dv[i] < 0 ? S_FLIPX : 0));
    }
    if (ts_fy >= 0) {
        if ((sys_time & 3) == 0) ts_vy++;
        ts_fy += ts_vy;
        if (ts_vy > 0 && ts_fy >= 64) {          /* down among the dancers */
            for (i = 0; i < TS_DANCERS; i++)
                if (ts_fx - ts_dx[i] < 14 && ts_dx[i] - ts_fx < 14) break;
            if (i < TS_DANCERS) { ts_caught++; sound_confirm(); box_tile(18, 1, GLYPH_DIGIT0 + ts_caught); }
            else sound_clink();
            ts_fy = -1;
            move_sprite(TS_OAM, 0, 0);
            move_sprite(TS_OAM + 1, 0, 0);
            if (!ts_left) ts_end = 90;
        } else {
            sprite16(TS_OAM, ts_fx, ts_fy, SPR1_SPARKLE, 6 | S_BANK1);
        }
    }
    return MG_RUNNING;
}

/* ------------------------------------------------------------ line 4
   The map: the stops between Étienne Marcel and Saint-Germain-des-Prés,
   top to bottom (the line runs north-south). Open stops can be chosen;
   Up/Down moves, A rides, B goes back. */
uint8_t metro_choice;
static const uint8_t mp_row_en[STOP_COUNT] = { 5, 9, 11, 15 };   /* rows of the box */
static const uint8_t mp_row_ko[STOP_COUNT] = { 3, 4, 5, 6 };      /* Korean: 16 px rows, stops only */
static const uint8_t *mp_row;   /* Les Halles, Cité, St-Michel, St-Germain */
static uint8_t mp_sel;

static void mp_draw(void) {
    uint8_t i;
    for (i = 0; i < STOP_COUNT; i++)
        box_tile(1, mp_row[i], i == mp_sel ? GLYPH_ARROW_RIGHT : G_SPACE);
}

void metro_start(void) BANKED {
    uint8_t r;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_mp_title);
    if (lang) {                      /* Korean lines are twice as tall: the four stops */
        mp_row = mp_row_ko;
        box_print(5, 3, txt_mp_s1);
        box_print(5, 4, txt_mp_s3);
        box_print(5, 5, txt_mp_s4);
        box_print(5, 6, txt_mp_s6);
        for (r = 0; r < STOP_COUNT; r++) box_tile(3, mp_row[r], GLYPH_COIN);
        box_print(1, 8, txt_mp_keys);
    } else {
        mp_row = mp_row_en;
        for (r = 3; r <= 15; r++) box_tile(3, r, GLYPH_PIPE);
        box_print(5, 3, txt_mp_s0);       /* Étienne Marcel */
        box_print(5, 5, txt_mp_s1);       /* LES HALLES */
        box_print(5, 7, txt_mp_s2);       /* Châtelet */
        box_print(5, 9, txt_mp_s3);       /* CITÉ */
        box_print(5, 11, txt_mp_s4);      /* SAINT-MICHEL */
        box_print(5, 13, txt_mp_s5);      /* Odéon */
        box_print(5, 15, txt_mp_s6);      /* ST-GERMAIN */
        for (r = 0; r < STOP_COUNT; r++) box_tile(3, mp_row[r], GLYPH_COIN);
        box_tile(3, 3, GLYPH_DOT);
        box_tile(3, 7, GLYPH_DOT);
        box_tile(3, 13, GLYPH_DOT);
        box_print(1, 16, txt_mp_keys);
    }
    mp_sel = cur_room == ROOM_METRO ? 1 : 0;
    box_flush();
    mp_draw();
}

uint8_t metro_update(void) BANKED {
    if (joy_pressed & J_UP)   { mp_sel = mp_sel ? mp_sel - 1 : STOP_COUNT - 1; sound_blip(); mp_draw(); }
    if (joy_pressed & J_DOWN) { mp_sel = mp_sel == STOP_COUNT - 1 ? 0 : mp_sel + 1; sound_blip(); mp_draw(); }
    if (joy_pressed & J_B) { box_close(); return MG_FAIL; }
    if (joy_pressed & J_A) {
        metro_choice = mp_sel;
        box_close();
        return MG_SUCCESS;
    }
    return MG_RUNNING;
}
