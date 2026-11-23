#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "sound.h"

#define BOX_ATTR (0x80u | 5u)   /* BG-over-sprite priority, text palette */
#define DLG_ROWS 5

static uint8_t row_buf[MAP_W];
static uint8_t page_buf[PAGE_SIZE];
static void load_page(void);
static uint8_t page;        /* absolute page index in dlg_pages */
static uint8_t pages_left;  /* pages after the current one */
static uint8_t pos;         /* characters revealed on the current page */
static uint8_t blink;
static uint8_t choice;      /* 0 = no choice pending, else pair index + 1 */
static uint8_t in_choice;   /* 1 while the options are shown */
uint8_t dialogue_choice;    /* result: 0 = first option, 1 = second */

void box_open(uint8_t rows) {
    uint8_t y, x;

    /* Attributes first (VRAM bank 1), then tiles (bank 0). */
    for (x = 0; x < MAP_W; x++) row_buf[x] = BOX_ATTR;
    VBK_REG = VBK_ATTRIBUTES;
    for (y = 0; y < rows; y++) set_win_tiles(0, y, MAP_W, 1, row_buf);
    VBK_REG = VBK_TILES;

    for (y = 0; y < rows; y++) {
        uint8_t l, m, r;
        if (y == 0) {
            l = GLYPH_BOX_TL; m = GLYPH_BOX_T; r = GLYPH_BOX_TR;
        } else if (y == rows - 1) {
            l = GLYPH_BOX_BL; m = GLYPH_BOX_B; r = GLYPH_BOX_BR;
        } else {
            l = GLYPH_BOX_L; m = GLYPH_SPACE; r = GLYPH_BOX_R;
        }
        row_buf[0] = l;
        for (x = 1; x < MAP_W - 1; x++) row_buf[x] = m;
        row_buf[MAP_W - 1] = r;
        set_win_tiles(0, y, MAP_W, 1, row_buf);
    }
    move_win(7, SCREEN_H - rows * 8);
    SHOW_WIN;
}

void box_close(void) {
    HIDE_WIN;
}

void box_print(uint8_t x, uint8_t y, const uint8_t *text) {
    uint8_t n = 0;
    while (text[n] && x + n < MAP_W) n++;
    set_win_tiles(x, y, n, 1, text);
}

static void clear_text(void) {
    uint8_t x, y;
    for (x = 0; x < TEXT_COLS; x++) row_buf[x] = GLYPH_SPACE;
    for (y = 0; y < TEXT_ROWS; y++) set_win_tiles(1, 1 + y, TEXT_COLS, 1, row_buf);
    set_win_tile_xy(MAP_W - 2, DLG_ROWS - 1, GLYPH_BOX_B);
    pos = 0;
    blink = 0;
}

void dialogue_start(uint8_t id) {
    choice = dlg_choice[id];
    in_choice = 0;
    dialogue_choice = 1;
    page = dlg_first_page[id];
    pages_left = dlg_page_count[id] - 1;
    load_page();
    box_open(DLG_ROWS);
    clear_text();
    sound_confirm();
}

/* Copy the current page from the dialogue ROM bank into RAM. */
static void load_page(void) {
    SWITCH_ROM(DLG_BANK);
    memcpy(page_buf, dlg_pages + (uint16_t)page * PAGE_SIZE, PAGE_SIZE);
    SWITCH_ROM(ASSET_BANK);
}

static void draw_choice(void) {
    const uint8_t *t = choice_text + (uint16_t)(choice - 1) * 14;
    set_win_tile_xy(3, 2, dialogue_choice == 0 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    set_win_tiles(4, 2, 7, 1, t);
    set_win_tile_xy(11, 2, dialogue_choice == 1 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    set_win_tiles(12, 2, 7, 1, t + 7);
}

uint8_t dialogue_update(void) {
    const uint8_t *text = page_buf;

    if (joy_pressed & J_B) {   /* B always closes; a pending choice counts as "no" */
        dialogue_choice = 1;
        box_close();
        return 0;
    }

    if (in_choice) {
        if (joy_pressed & (J_LEFT | J_RIGHT)) {
            dialogue_choice ^= 1;
            draw_choice();
            sound_blip();
        }
        if (joy_pressed & J_A) {
            box_close();
            return 0;
        }
        return 1;
    }

    if (pos < PAGE_SIZE) {
        if (joy_pressed & J_A) {
            /* Reveal the rest of the page at once. */
            uint8_t y;
            for (y = 0; y < TEXT_ROWS; y++)
                set_win_tiles(1, 1 + y, TEXT_COLS, 1, text + y * TEXT_COLS);
            pos = PAGE_SIZE;
            return 1;
        }
        /* Skip runs of spaces so line padding costs no time. */
        while (pos < PAGE_SIZE && text[pos] == GLYPH_SPACE) pos++;
        if (pos < PAGE_SIZE) {
            set_win_tile_xy(1 + pos % TEXT_COLS, 1 + pos / TEXT_COLS, text[pos]);
            if ((pos & 3) == 0) sound_blip();
            pos++;
        }
        return 1;
    }

    /* Page complete: blinking arrow, wait for A. */
    blink++;
    set_win_tile_xy(MAP_W - 2, DLG_ROWS - 1,
                    (blink & 0x10) ? GLYPH_BOX_B : GLYPH_ARROW_DOWN);
    if (joy_pressed & J_A) {
        if (pages_left == 0) {
            if (choice) {
                clear_text();
                in_choice = 1;
                dialogue_choice = 0;
                draw_choice();
                return 1;
            }
            box_close();
            return 0;
        }
        page++;
        pages_left--;
        load_page();
        clear_text();
    }
    return 1;
}
