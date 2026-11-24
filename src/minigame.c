#pragma bank 7   /* with games.c: banked to keep bank 0 free */
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "notebook.h"
#include "input.h"
#include "minigame.h"
#include "ko.h"        /* far_read */
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
        box_tile(BAR_X + i, 3, t);
    }
}

void lift_start(uint8_t level) BANKED {
    timing_start(level, txt_lift_1, txt_lift_2);
}

/* The same timing bar, with its own two lines of instructions. */
void timing_start(uint8_t level, const uint8_t *line1, const uint8_t *line2) BANKED {
    box_open(5);
    box_print(1, 1, line1);
    box_print(1, 2, line2);
    if (level > 2) level = 2;
    zone_lo = zone_lo_by_level[level];
    zone_hi = zone_hi_by_level[level];
    speed = speed_by_level[level];
    cur = 0;
    dir = 1;
    timer = 0;
    draw_bar();
}

uint8_t lift_update(void) BANKED {
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
/* A 1967 Paris number: a three-letter exchange (ODÉon 8400) and 4 digits.
   Slot 0 is the exchange, slots 1-4 the digits. */
static const uint8_t * const exch_txt[EXCH_COUNT] = {
    txt_ex_0, txt_ex_1, txt_ex_2, txt_ex_3, txt_ex_4, txt_ex_5, txt_ex_6, txt_ex_7, txt_ex_8,
};
static uint8_t digits[4];
static uint8_t sel;
uint8_t dial_exch;
static uint8_t digit_tile0;     /* tile of '0' */

static void draw_dial(void) {
    uint8_t i;
    box_tile(3, 3, sel == 0 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    box_print(4, 3, exch_txt[dial_exch]);
    for (i = 0; i < 4; i++) {
        box_tile(8 + i * 2, 3, i + 1 == sel ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
        box_tile(9 + i * 2, 3, digit_tile0 + digits[i]);
    }
}

void dial_start(void) BANKED {
    uint8_t i;
    far_read(ASSET_BANK, txt_digits, &digit_tile0, 1);   /* "0123456789" from ui.txt (bank 1) */
    for (i = 0; i < 4; i++) digits[i] = 0;
    sel = dial_exch = 0;
    box_open(6);
    box_print(1, 1, txt_dial_1);
    box_print(1, 2, txt_dial_2);
    box_print(1, 4, txt_dial_3);
    draw_dial();
}

uint8_t dial_update(void) BANKED {
    if (joy_pressed & J_B) {
        box_close();
        return MG_FAIL;
    }
    if (joy_pressed & J_A) {
        box_close();
        return MG_SUCCESS;
    }
    if (joy_pressed & J_START) {       /* the next number written in the notebook */
        uint16_t n = notebook_number();
        dial_exch = notebook_exch;
        digits[0] = n / 1000; digits[1] = n / 100 % 10; digits[2] = n / 10 % 10; digits[3] = n % 10;
        sound_blip();
        draw_dial();
    }
    if (sel == 0) {
        if (joy_pressed & J_UP)   dial_exch = dial_exch == EXCH_COUNT - 1 ? 0 : dial_exch + 1;
        if (joy_pressed & J_DOWN) dial_exch = dial_exch == 0 ? EXCH_COUNT - 1 : dial_exch - 1;
    } else {
        uint8_t *d = &digits[sel - 1];
        if (joy_pressed & J_UP)   *d = *d == 9 ? 0 : *d + 1;
        if (joy_pressed & J_DOWN) *d = *d == 0 ? 9 : *d - 1;
    }
    if ((joy_pressed & J_LEFT) && sel) sel--;
    if ((joy_pressed & J_RIGHT) && sel < 4) sel++;
    if (joy_pressed & (J_UP | J_DOWN | J_LEFT | J_RIGHT)) {
        sound_blip();
        draw_dial();
    }
    return MG_RUNNING;
}

uint16_t dial_number(void) BANKED {
    return digits[0] * 1000u + digits[1] * 100u + digits[2] * 10u + digits[3];
}
