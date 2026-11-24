/* Korean text (language option). Lives in ROM bank KO_BANK with its tables
   (src/ko_data.c); glyphs and pages are in banks 8.. and are copied to RAM
   with far_read(). Data format: tools/ko_gen.py.

   Dialogue: each page is two lines of Galmuri11 glyphs, drawn into a RAM
   bitmap (lbuf) and revealed one tile column at a time. Line L uses window
   rows 1+2L and 2+2L, columns 1-18, with VRAM tiles L*36 + half*18 + column
   (BG tiles 0-71: bank 0, 0x9000).
   Menus: Galmuri7 syllables are 8x8 tiles, loaded on demand into a small
   cache (tiles 72-127, or 0-127 in a full-screen box). */
#pragma bank 6

#include <gb/gb.h>
#include <string.h>
#include "assets.h"
#include "ko_data.h"
#include "ko.h"

static uint8_t lbuf[2][16][TEXT_COLS];
static uint8_t kpage[KO_MAX_PAGE];
static uint8_t gbuf[KO_GLYPH_SIZE];
static uint8_t tbuf[16];
static uint8_t rowbuf[MAP_W];
static uint16_t k_page;
static uint8_t k_left;          /* pages after the current one */
static uint8_t k_id;            /* dialogue id, for its choice */
static uint8_t k_cols[2];       /* tile columns used by each line */
static uint8_t k_col;           /* next column to reveal, 0..35 */

static uint16_t cache_code[128];
static uint8_t cache_n, cache_base, cache_max;

/* ---- dialogue ------------------------------------------------------------ */
static void render_line(uint8_t L, const uint8_t *g, uint8_t n) {
    uint8_t x = 0, r, b, s;
    uint16_t gi, bits;
    uint8_t *row;
    while (n--) {
        gi = ((uint16_t)g[0] << 8) | g[1];
        g += 2;
        far_read(ko_glyph_bank[gi], ko_glyph_ptr[gi], gbuf, KO_GLYPH_SIZE);
        b = x >> 3;
        s = x & 7;
        if (b < TEXT_COLS) {
            for (r = 0; r < KO_GLYPH_ROWS; r++) {
                bits = ((uint16_t)gbuf[1 + 2 * r] << 8) | gbuf[2 + 2 * r];
                if (!bits) continue;
                row = lbuf[L][r + 2] + b;            /* 2 blank rows on top */
                row[0] |= (uint8_t)(bits >> (8 + s));
                if (b + 1 < TEXT_COLS) row[1] |= (uint8_t)(bits >> s);
                if (s && b + 2 < TEXT_COLS) row[2] |= (uint8_t)(bits << (8 - s));
            }
        }
        x += gbuf[0];
    }
    b = (x + 7) >> 3;
    k_cols[L] = b > TEXT_COLS ? TEXT_COLS : b;
}

static void load(void) {
    far_read(ko_page_bank[k_page], ko_page_ptr[k_page], kpage, 2);
    far_read(ko_page_bank[k_page], ko_page_ptr[k_page] + 2, kpage + 2,
             (uint8_t)((kpage[0] + kpage[1]) * 2));
    memset(lbuf, 0, sizeof(lbuf));
    render_line(0, kpage + 2, kpage[0]);
    render_line(1, kpage + 2 + 2 * kpage[0], kpage[1]);
    k_col = 0;
}

static void put_col(uint8_t L, uint8_t c) {
    uint8_t h, r, id;
    for (h = 0; h < 2; h++) {
        for (r = 0; r < 8; r++) {
            tbuf[2 * r] = 0xFF;                  /* paper = colour 1, ink = 3 */
            tbuf[2 * r + 1] = lbuf[L][h * 8 + r][c];
        }
        id = L * 36 + h * TEXT_COLS + c;
        set_bkg_data(id, 1, tbuf);
        set_win_tile_xy(1 + c, 1 + 2 * L + h, id);
    }
}

void ko_start(uint8_t id) BANKED {
    k_id = id;
    k_page = ko_first_page[id];
    k_left = ko_page_count[id] - 1;
    load();
}

uint8_t ko_last(void) BANKED { return k_left == 0; }

void ko_next(void) BANKED {
    k_page++;
    k_left--;
    load();
}

uint8_t ko_reveal(void) BANKED {
    uint8_t L, c;
    while (k_col < 2 * TEXT_COLS) {
        L = k_col >= TEXT_COLS;
        c = L ? k_col - TEXT_COLS : k_col;
        k_col++;
        if (c < k_cols[L]) {
            put_col(L, c);
            return 1;
        }
    }
    return 0;
}

uint8_t ko_done(void) BANKED { return k_col >= 2 * TEXT_COLS; }

void ko_reveal_all(void) BANKED {
    while (ko_reveal()) ;
}

void ko_clear(void) BANKED {
    uint8_t y;
    memset(rowbuf, GLYPH_SPACE, TEXT_COLS);
    for (y = 1; y <= 4; y++) set_win_tiles(1, y, TEXT_COLS, 1, rowbuf);
}

/* ---- menus ----------------------------------------------------------------- */
void ko_ui_reset(uint8_t full) BANKED {
    cache_n = 0;
    cache_base = full ? 0 : 72;
    cache_max = full ? 128 : 56;
}

static uint8_t syl_tile(uint16_t i) {
    uint8_t k, r;
    const uint8_t *src;
    for (k = 0; k < cache_n; k++)
        if (cache_code[k] == i) return cache_base + k;
    if (cache_n >= cache_max) return GLYPH_SPACE;   /* checked by tools/ko_gen.py */
    src = ko_syl_tiles + i * 8;
    for (r = 0; r < 8; r++) {
        tbuf[2 * r] = 0xFF;
        tbuf[2 * r + 1] = src[r];
    }
    k = cache_base + cache_n;
    set_bkg_data(k, 1, tbuf);
    cache_code[cache_n++] = i;
    return k;
}

static void draw(uint8_t x, uint8_t y, const uint8_t *s) {
    uint8_t n = 0;
    while (*s && x + n < MAP_W) {
        if (*s < 16) {
            rowbuf[n++] = syl_tile(((uint16_t)(s[0] - 1) << 8) | s[1]);
            s += 2;
        } else {
            rowbuf[n++] = *s++;
        }
    }
    if (n) set_win_tiles(x, y, n, 1, rowbuf);
}

uint8_t ko_print(uint8_t x, uint8_t y, uint16_t off) BANKED {
    uint8_t lo = 0, hi = KO_UI_COUNT, mid;
    while (lo < hi) {
        mid = (lo + hi) >> 1;
        if (ko_ui_off[mid] == off) {
            draw(x, y, ko_ui_ptr[mid]);
            return 1;
        }
        if (ko_ui_off[mid] < off) lo = mid + 1;
        else hi = mid;
    }
    return 0;
}

void ko_draw_choice(uint8_t sel) BANKED {
    ko_clear();
    set_win_tile_xy(3, 2, sel == 0 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    draw(4, 2, ko_choice_ptr[2 * k_id]);
    set_win_tile_xy(11, 2, sel == 1 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    draw(12, 2, ko_choice_ptr[2 * k_id + 1]);
}

void ko_draw_name(uint8_t x, uint8_t y) BANKED {
    ko_ui_reset(1);
    draw(x, y, ko_lang_name);
}
