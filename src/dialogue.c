#include <gb/gb.h>
#include <gb/cgb.h>
#include <string.h>
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "sound.h"
#include "ko.h"
#include "fr.h"

#define BOX_ATTR (0x80u | 5u)   /* BG-over-sprite priority, text palette */
#define DLG_ROWS (lang ? 6 : 5)   /* Korean: two 16-pixel lines */

uint8_t lang;
uint8_t lang_fr;            /* French: the English engine with fr_* text (fr_gen.py) */

void far_read(uint8_t bank, const uint8_t *src, uint8_t *dst, uint8_t n) {
    uint8_t save = _current_bank;
    SWITCH_ROM(bank);
    memcpy(dst, src, n);
    SWITCH_ROM(save);
}

static uint8_t row_buf[MAP_W];
static uint8_t page_buf[PAGE_SIZE];
static void load_page(void);
static uint16_t page;       /* absolute page index in dlg_pages */
static uint8_t pages_left;  /* pages after the current one */
static uint8_t pos;         /* characters revealed on the current page */
static uint8_t blink;
static uint8_t choice;      /* 0 = no choice pending, else pair index + 1 */
static uint8_t in_choice;   /* 1 while the options are shown */
uint8_t dialogue_choice;    /* result: 0 = first option, 1 = second */

static void env_reload(void) {        /* scenery tiles back into VRAM bank 1 */
    uint8_t save = _current_bank;
    ko_borrowed = 0;
    VBK_REG = VBK_ATTRIBUTES;
    SWITCH_ROM(PORTRAIT_BANK);
    set_bkg_data(0, ENV_TILE_COUNT, env_tiles);
    SWITCH_ROM(save);
    VBK_REG = VBK_TILES;
}

static void box_frame(uint8_t rows);

/* Korean: a full-screen box (18 rows) is laid out by box_flush(); a smaller
   one gets two tile rows per line (src/ko.c). */
void box_open(uint8_t rows) {
    if (lang) {
        if (rows == SCREEN_H / 8) {
            ko_ui_reset(2);
        } else {
            if (ko_borrowed) env_reload();
            ko_ui_reset(1);
            rows = ko_small_rows(rows);
        }
    }
    box_frame(rows);
}

static void box_dialogue(void) {
    if (lang) {
        if (ko_borrowed) env_reload();
        ko_ui_reset(4);
    }
    box_frame(DLG_ROWS);
}

static void box_frame(uint8_t rows) {
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

/* Speaker portrait: six 8x16 sprites standing on the top-left of the box.
   Tiles are in VRAM bank 1 (attribute bit 3), loaded by load_graphics(). */
#define PORTRAIT_OAM 24
static void portrait_show(uint8_t p) {
    uint8_t i;
    if (p == NO_PORTRAIT) return;
    for (i = 0; i < 6; i++) {
        uint8_t n = PORTRAIT_OAM + i;
        set_sprite_tile(n, PORTRAIT_BASE + p * 12 + i * 2);
        set_sprite_prop(n, portrait_pal[p] | 0x08);
        move_sprite(n, 8 + 8 + (i >> 1) * 8, SCREEN_H - DLG_ROWS * 8 - 32 + 16 + (i & 1) * 16);
    }
}

static void portrait_hide(void) {
    uint8_t i;
    for (i = 0; i < 6; i++) move_sprite(PORTRAIT_OAM + i, 0, 0);
}

void box_close(void) {
    HIDE_WIN;
    portrait_hide();
    if (ko_borrowed) env_reload();
    if (lang) ko_mode = 0;
}

void box_tile(uint8_t x, uint8_t y, uint8_t t) {
    if (lang && ko_mode) ko_tile(x, y, t);
    else set_win_tile_xy(x, y, t);
}

void box_clear(uint8_t y) {            /* blank the inside of one text row */
    uint8_t x;
    if (lang && ko_mode) { ko_clear_row(y); return; }
    for (x = 1; x < MAP_W - 1; x++) set_win_tile_xy(x, y, GLYPH_SPACE);
}

void box_flush(void) {
    if (lang) ko_flush();
}

uint8_t box_more(void) {
    return lang ? ko_more() : 0;
}

uint8_t box_has_next(void) {
    return lang ? ko_has_next() : 0;
}

void box_print(uint8_t x, uint8_t y, const uint8_t *text) {
    uint8_t n = 0, save = _current_bank;
    if (lang && ko_print(x, y, (uint16_t)(text - ui_text), text)) return;
    if (lang_fr) { fr_print(x, y, (uint16_t)(text - ui_text)); return; }
    SWITCH_ROM(ASSET_BANK);          /* callable from banked code too */
    while (text[n] && x + n < MAP_W) n++;
    set_win_tiles(x, y, n, 1, text);
    SWITCH_ROM(save);
}

uint8_t digit_glyph(uint8_t n) {
    uint8_t g, save = _current_bank;
    SWITCH_ROM(ASSET_BANK);
    g = txt_digits[n];
    SWITCH_ROM(save);
    return g;
}

static void clear_text(void) {
    uint8_t x, y;
    if (lang) {
        ko_clear();
    } else {
        for (x = 0; x < TEXT_COLS; x++) row_buf[x] = GLYPH_SPACE;
        for (y = 0; y < TEXT_ROWS; y++) set_win_tiles(1, 1 + 2 * y, TEXT_COLS, 1, row_buf);
    }
    set_win_tile_xy(MAP_W - 2, DLG_ROWS - 1, GLYPH_BOX_B);
    pos = 0;
    blink = 0;
}

/* First page and page count of a dialogue, English or French. */
static void dlg_pages_of(uint8_t id) {
    if (lang_fr) {
        fr_pages_of(id, &page, &pages_left);
    } else {
        far_read(TITLE_BANK, (const uint8_t *)&dlg_first_page[id], (uint8_t *)&page, 2);
        far_read(TITLE_BANK, &dlg_page_count[id], &pages_left, 1);
    }
    pages_left--;
}

void dialogue_start(uint8_t id) {
    far_read(TITLE_BANK, &dlg_choice[id], &choice, 1);
    in_choice = 0;
    dialogue_choice = 1;
    dlg_pages_of(id);
    if (lang) ko_start(id);
    else load_page();
    box_dialogue();
    clear_text();
    portrait_hide();
    portrait_show(dlg_portrait[id]);
    sound_confirm();
}

/* Copy the current page from the dialogue ROM bank into RAM. */
static void load_page(void) {
    if (lang_fr) { fr_load_page(page, page_buf); return; }
    if (page < DLG_PAGES_LO) {
        SWITCH_ROM(DLG_BANK);
        memcpy(page_buf, dlg_pages + page * PAGE_SIZE, PAGE_SIZE);
    } else {                    /* the overflow: banks DLG_HI_BANK.. */
        uint16_t k = page - DLG_PAGES_LO;
        uint8_t b = (uint8_t)(k / DLG_HI_PER_BANK);
        const uint8_t *base;
        SWITCH_ROM(ASSET_BANK);
        base = dlg_hi_base[b];
        SWITCH_ROM(DLG_HI_BANK + b);
        memcpy(page_buf, base + (k - (uint16_t)b * DLG_HI_PER_BANK) * PAGE_SIZE, PAGE_SIZE);
    }
    SWITCH_ROM(ASSET_BANK);
}

static void draw_choice(void) {
    const uint8_t *t;
    if (lang) {
        ko_draw_choice(dialogue_choice);
        return;
    }
    t = choice_text + (uint16_t)(choice - 1) * 14;
    if (lang_fr) { fr_choice(choice, row_buf); t = row_buf; }
    set_win_tile_xy(3, 2, dialogue_choice == 0 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    set_win_tiles(4, 2, 7, 1, t);
    set_win_tile_xy(11, 2, dialogue_choice == 1 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    set_win_tiles(12, 2, 7, 1, t + 7);
}

uint8_t dialogue_update(void) {
    const uint8_t *text = page_buf;

    if (joy_pressed & J_B) {   /* B always closes: "no" on the options, 2 before them */
        dialogue_choice = in_choice ? 1 : 2;
        box_close();
        return 0;
    }

    if (in_choice) {
        if (joy_pressed & (J_LEFT | J_RIGHT)) {
            dialogue_choice ^= 1;
            draw_choice();
            sound_blip();
        }
        if (joy_pressed & (J_A | J_START)) {
            box_close();
            return 0;
        }
        return 1;
    }

    if (lang) {
        if (!ko_done()) {
            if (joy_pressed & (J_A | J_START)) ko_reveal_all();
            else if (ko_reveal() && (++pos & 3) == 0) sound_blip();
            return 1;
        }
    } else if (pos < PAGE_SIZE) {
        if (joy_pressed & (J_A | J_START)) {
            /* Reveal the rest of the page at once. */
            uint8_t y;
            for (y = 0; y < TEXT_ROWS; y++)
                set_win_tiles(1, 1 + 2 * y, TEXT_COLS, 1, text + y * TEXT_COLS);
            pos = PAGE_SIZE;
            return 1;
        }
        /* Skip runs of spaces so line padding costs no time. */
        while (pos < PAGE_SIZE && text[pos] == GLYPH_SPACE) pos++;
        if (pos < PAGE_SIZE) {
            set_win_tile_xy(1 + pos % TEXT_COLS, 1 + 2 * (pos / TEXT_COLS), text[pos]);
            if ((pos & 3) == 0) sound_blip();
            pos++;
        }
        return 1;
    }

    /* Page complete: blinking arrow, wait for A. */
    blink++;
    set_win_tile_xy(MAP_W - 2, DLG_ROWS - 1,
                    (blink & 0x10) ? GLYPH_BOX_B : GLYPH_ARROW_DOWN);
    if (joy_pressed & (J_A | J_START)) {
        if (lang ? ko_last() : pages_left == 0) {
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
        if (lang) {
            ko_next();
        } else {
            page++;
            pages_left--;
            load_page();
        }
        clear_text();
    }
    return 1;
}

/* A one-page line an NPC says by itself: drawn at once, no input needed.
   The caller closes it with box_close() after a while. */
void bark_show(uint8_t id) {
    uint8_t y;
    if (lang) {
        ko_start(id);
        box_dialogue();
        ko_reveal_all();
    } else {
        dlg_pages_of(id);
        load_page();
        box_dialogue();
        for (y = 0; y < TEXT_ROWS; y++)
            set_win_tiles(1, 1 + 2 * y, TEXT_COLS, 1, page_buf + y * TEXT_COLS);
    }
    box_print(10, DLG_ROWS - 1, txt_bark_hint);   /* "A: talk" on the frame */
    portrait_hide();
    portrait_show(dlg_portrait[id]);
}
