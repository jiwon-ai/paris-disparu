#pragma bank 7   /* with games.c: banked to keep bank 0 free */
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "notebook.h"
#include "input.h"
#include "minigame.h"
#include "ko.h"        /* far_read */
#include "sound.h"

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
