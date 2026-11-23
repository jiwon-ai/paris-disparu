#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "input.h"
#include "minigame.h"
#include "sound.h"

#define BAR_X 2
#define BAR_LEN 16

/* ---------------------------------------------------------------- lift */
static uint8_t cur, dir, speed, timer, zone_lo, zone_hi;
static const uint8_t zone_lo_by_level[3] = { 6, 7, 7 };
static const uint8_t zone_hi_by_level[3] = { 9, 8, 8 };
static const uint8_t speed_by_level[3]   = { 4, 4, 3 };   /* frames per cell */

static void draw_bar(void) {
    uint8_t i, t;
    for (i = 0; i < BAR_LEN; i++) {
        if (i == cur) t = GLYPH_BAR_CURSOR;
        else if (i >= zone_lo && i <= zone_hi) t = GLYPH_BAR_ZONE;
        else t = GLYPH_BAR;
        set_win_tile_xy(BAR_X + i, 3, t);
    }
}

void lift_start(uint8_t level) {
    box_open(5);
    box_print(1, 1, txt_lift_1);
    box_print(1, 2, txt_lift_2);
    if (level > 2) level = 2;
    zone_lo = zone_lo_by_level[level];
    zone_hi = zone_hi_by_level[level];
    speed = speed_by_level[level];
    cur = 0;
    dir = 1;
    timer = 0;
    draw_bar();
}

uint8_t lift_update(void) {
    if (joy_pressed & J_A) {
        box_close();
        return (cur >= zone_lo && cur <= zone_hi) ? MG_SUCCESS : MG_FAIL;
    }
    if (++timer >= speed) {
        timer = 0;
        if (dir) {
            if (++cur == BAR_LEN - 1) dir = 0;
        } else {
            if (--cur == 0) dir = 1;
        }
        draw_bar();
    }
    return MG_RUNNING;
}

/* ---------------------------------------------------------------- dial */
static uint8_t digits[4];
static uint8_t sel;
static uint8_t digit_tile0;     /* tile of '0' */

static void draw_dial(void) {
    uint8_t i;
    for (i = 0; i < 4; i++) {
        set_win_tile_xy(6 + i * 2, 3, i == sel ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
        set_win_tile_xy(7 + i * 2, 3, digit_tile0 + digits[i]);
    }
}

void dial_start(void) {
    uint8_t i;
    digit_tile0 = txt_digits[0];   /* "0123456789" from ui.txt */
    for (i = 0; i < 4; i++) digits[i] = 0;
    sel = 0;
    box_open(5);
    box_print(1, 1, txt_dial_1);
    box_print(1, 2, txt_dial_2);
    draw_dial();
}

uint8_t dial_update(void) {
    if (joy_pressed & J_B) {
        box_close();
        return MG_FAIL;
    }
    if (joy_pressed & J_A) {
        box_close();
        return MG_SUCCESS;
    }
    if (joy_pressed & J_UP)    digits[sel] = digits[sel] == 9 ? 0 : digits[sel] + 1;
    if (joy_pressed & J_DOWN)  digits[sel] = digits[sel] == 0 ? 9 : digits[sel] - 1;
    if ((joy_pressed & J_LEFT) && sel) sel--;
    if ((joy_pressed & J_RIGHT) && sel < 3) sel++;
    if (joy_pressed & (J_UP | J_DOWN | J_LEFT | J_RIGHT)) {
        sound_blip();
        draw_dial();
    }
    return MG_RUNNING;
}

uint16_t dial_number(void) {
    return digits[0] * 1000u + digits[1] * 100u + digits[2] * 10u + digits[3];
}
