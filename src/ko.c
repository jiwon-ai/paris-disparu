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

/* Blob bank of item i, from where each bank's items end. */
static uint8_t ko_bank(const uint16_t *end, uint16_t i) {
    uint8_t b = KO_BLOB_BANK;
    while (i >= *end++) b++;
    return b;
}
static uint8_t k_left;          /* pages after the current one */
static uint8_t k_id;            /* dialogue id, for its choice */
static uint8_t k_cols[2];       /* tile columns used by each line */
static uint8_t k_col;           /* next column to reveal, 0..35 */


/* ---- dialogue ------------------------------------------------------------ */
/* OR one glyph (14 rows of hi, lo bytes at src) into the line at row, shifted
   right by S pixels; r = how many columns to the right exist (0-2). */
#define SHIFTS(S) do { uint8_t k_; \
    for (k_ = 0; k_ < KO_GLYPH_ROWS; k_++, row += TEXT_COLS) { \
        hi = *src++; lo = *src++; \
        if (!(hi | lo)) continue; \
        row[0] |= (uint8_t)(hi >> (S)); \
        if (r) row[1] |= (uint8_t)(hi << (8 - (S))) | (uint8_t)(lo >> (S)); \
        if (r == 2) row[2] |= (uint8_t)(lo << (8 - (S))); \
    } } while (0)

static void render_line(uint8_t L, const uint8_t *g, uint8_t n) {
    uint8_t x = 0, r, b, s, hi, lo, k, *row;
    const uint8_t *src;
    uint16_t gi;
    while (n--) {
        gi = ((uint16_t)g[0] << 8) | g[1];
        g += 2;
        far_read(ko_bank(ko_glyph_end, gi), ko_glyph_ptr[gi], gbuf, KO_GLYPH_SIZE);
        b = x >> 3;
        s = x & 7;
        if (b < TEXT_COLS) {
            row = &lbuf[L][2][b];                    /* 2 blank rows on top */
            src = gbuf + 1;
            r = b + 2 < TEXT_COLS ? 2 : (b + 1 < TEXT_COLS ? 1 : 0);   /* spill columns */
            switch (s) {                    /* constant shifts: no shift loops */
            case 0:
                for (k = 0; k < KO_GLYPH_ROWS; k++, row += TEXT_COLS) {
                    hi = *src++; lo = *src++;
                    row[0] |= hi;
                    if (r) row[1] |= lo;
                }
                break;
            case 1: SHIFTS(1); break;
            case 2: SHIFTS(2); break;
            case 3: SHIFTS(3); break;
            case 4: SHIFTS(4); break;
            case 5: SHIFTS(5); break;
            case 6: SHIFTS(6); break;
            default: SHIFTS(7); break;
            }
        }
        x += gbuf[0];
    }
    b = (x + 7) >> 3;
    k_cols[L] = b > TEXT_COLS ? TEXT_COLS : b;
}

static void load(void) {
    far_read(ko_bank(ko_page_end, k_page), ko_page_ptr[k_page], kpage, 2);
    far_read(ko_bank(ko_page_end, k_page), ko_page_ptr[k_page] + 2, kpage + 2,
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

/* ---- boxes ---------------------------------------------------------------
   A UI string with Hangul is one 16-pixel line (two tile rows); 8x8 strings
   and single tiles sit on the lower row of their line. Small boxes (captions,
   mini-games) are drawn at once with logical row y on rows 2y-1 and 2y.
   Full-screen boxes are collected, then laid out by ko_flush(): one line per
   used row, gaps kept if there is room, and pages of up to 8 lines (and 256 tiles) if not. Tiles
   come from BG tiles 0-127 (bank 0), then 0-127 of VRAM bank 1 (the scenery
   tiles, reloaded by box_close() when they were borrowed). */
#define KI_TILE 0     /* one 8x8 tile */
#define KI_EN   1     /* English string in ui_text (ROM bank 1) */
#define KI_KO   2     /* record in this bank: tall line or 8x8 bytes */
#define MAX_ITEMS 32
typedef struct { uint8_t kind, x, y, tile; const uint8_t *s; } kitem_t;
static kitem_t items[MAX_ITEMS];
static uint8_t n_items;
uint8_t ko_mode;               /* 0 English, 1 small box, 2 full box (collect), 3 full box laid out */
uint8_t ko_borrowed;           /* VRAM bank 1 scenery tiles overwritten */
static uint8_t pool_next, pool_end, pool_bank1;
static uint8_t in_dlg, cur_row, seq_pool;
static uint8_t row_next[5];    /* small boxes: logical row y uses tiles 32(y-1).. */
static uint8_t rowmap[18];     /* logical row -> top physical row, 0 = none */
static uint8_t lines[18], n_lines, page_k, n_pages;
#define KO_BOX_ATTR (0x80u | 5u)

void ko_ui_reset(uint8_t mode) BANKED {   /* 1 small, 2 full, 4 inside a dialogue */
    n_items = 0;
    pool_bank1 = 0;
    in_dlg = mode == 4;
    seq_pool = 0;
    for (cur_row = 1; cur_row < 5; cur_row++) row_next[cur_row] = (uint8_t)((cur_row - 1) * 32);
    pool_next = mode == 4 ? 72 : 0;
    pool_end = 128;
    ko_mode = mode == 4 ? 1 : mode;
    page_k = n_pages = 0;
    memset(rowmap, 0, sizeof(rowmap));
}

static uint8_t alloc_tile(void) {          /* 0xFF: out of tiles */
    if (ko_mode == 1 && !in_dlg && !seq_pool) {
        uint8_t r = cur_row > 4 ? 4 : (cur_row ? cur_row : 1);
        if (row_next[r] >= r * 32) return 0xFF;
        return row_next[r]++;
    }
    if (pool_next >= pool_end) {
        /* full screen and the rules card (it covers the screen) may borrow
           the scenery tiles of VRAM bank 1; box_close() reloads them */
        if ((ko_mode < 2 && !seq_pool) || pool_bank1) return 0xFF;
        pool_bank1 = 1;
        ko_borrowed = 1;
        pool_next = 0;
    }
    return pool_next++;
}

/* Tile data for one line, uploaded in runs of consecutive tiles. */
static uint8_t tdata[2 * TEXT_COLS * 16];
static uint8_t t_first, t_count, t_bank;
static uint8_t m_tile[2][TEXT_COLS], m_attr[2][TEXT_COLS];

static void t_flush(void) {
    if (!t_count) return;
    if (t_bank) VBK_REG = 1;
    set_bkg_data(t_first, t_count, tdata);
    VBK_REG = 0;
    t_count = 0;
}

/* One 8x8 cell of the line bitmap (rows8 = 8 rows, TEXT_COLS apart). */
static void cell(uint8_t h, uint8_t c, const uint8_t *rows8) {
    uint8_t r, t, *d, any = 0;
    const uint8_t *q = rows8;
    for (r = 0; r < 8; r++, q += TEXT_COLS) any |= *q;
    m_tile[h][c] = GLYPH_SPACE;
    m_attr[h][c] = KO_BOX_ATTR;
    if (!any) return;
    t = alloc_tile();
    if (t == 0xFF) return;
    if (t_count && (t != (uint8_t)(t_first + t_count) || t_bank != pool_bank1)) t_flush();
    if (!t_count) { t_first = t; t_bank = pool_bank1; }
    d = tdata + ((uint16_t)t_count << 4);
    for (r = 0, q = rows8; r < 8; r++, q += TEXT_COLS) {
        *d++ = 0xFF;                             /* paper = colour 1, ink = 3 */
        *d++ = *q;
    }
    t_count++;
    m_tile[h][c] = t;
    if (pool_bank1) m_attr[h][c] = KO_BOX_ATTR | 0x08;
}

/* A glyph record line (count, then 2-byte indices) at column x, rows y, y+1. */
static void tall(uint8_t x, uint8_t y, const uint8_t *rec) {
    uint8_t n = rec[0], c, cols;
    memset(lbuf[0], 0, sizeof(lbuf[0]));
    render_line(0, rec + 1, n);
    cols = k_cols[0];
    if (x + cols > MAP_W - 1) cols = MAP_W - 1 - x;
    t_count = 0;
    for (c = 0; c < cols; c++) {
        cell(0, c, &lbuf[0][0][c]);
        cell(1, c, &lbuf[0][8][c]);
    }
    t_flush();
    if (ko_borrowed) {
        VBK_REG = 1;
        set_win_tiles(x, y, cols, 1, m_attr[0]);
        set_win_tiles(x, y + 1, cols, 1, m_attr[1]);
        VBK_REG = 0;
    }
    set_win_tiles(x, y, cols, 1, m_tile[0]);
    set_win_tiles(x, y + 1, cols, 1, m_tile[1]);
}

static void flat(uint8_t x, uint8_t y, const uint8_t *s) {
    uint8_t n = 0;
    while (s[n] && x + n < MAP_W) { rowbuf[n] = s[n]; n++; }
    if (n) set_win_tiles(x, y, n, 1, rowbuf);
}

static void draw_item(const kitem_t *it, uint8_t top) {
    if (it->kind == KI_TILE) {
        set_win_tile_xy(it->x, top + 1, it->tile);
    } else if (it->kind == KI_EN) {      /* bank 1 data: copied through bank 0 */
        uint8_t n = 0;
        far_read(ASSET_BANK, it->s, tbuf, 16);
        while (n < 16 && tbuf[n] && it->x + n < MAP_W) n++;
        if (n) set_win_tiles(it->x, top + 1, n, 1, tbuf);
    } else if (it->s[0] == 1) {
        tall(it->x, top, it->s + 1);
    } else {
        flat(it->x, top + 1, it->s);
    }
}

/* Small box: physical top row of logical row y. */
#define SMALL_TOP(y) ((uint8_t)((y) * 2 - 1))

static void add(uint8_t kind, uint8_t x, uint8_t y, uint8_t tile, const uint8_t *s) {
    kitem_t *it;
    if (ko_mode == 1 || ko_mode == 3) {
        kitem_t t;
        t.kind = kind; t.x = x; t.y = y; t.tile = tile; t.s = s;
        cur_row = y;
        draw_item(&t, ko_mode == 1 ? SMALL_TOP(y) : (rowmap[y] ? rowmap[y] : y));
        return;
    }
    if (n_items >= MAX_ITEMS) return;
    it = &items[n_items++];
    it->kind = kind; it->x = x; it->y = y; it->tile = tile; it->s = s;
}

static const uint8_t *find_ui(uint16_t off) {
    uint8_t lo = 0, hi = KO_UI_COUNT, mid;
    while (lo < hi) {
        mid = (lo + hi) >> 1;
        if (ko_ui_off[mid] == off) return ko_ui_ptr[mid];
        if (ko_ui_off[mid] < off) lo = mid + 1;
        else hi = mid;
    }
    return 0;
}

uint8_t ko_print(uint8_t x, uint8_t y, uint16_t off, const uint8_t *en) BANKED {
    const uint8_t *s = find_ui(off);
    if (s && !s[0]) return 1;            /* left empty in Korean: the line above says it */
    if (s) add(KI_KO, x, y, 0, s);
    else add(KI_EN, x, y, 0, en);
    return 1;
}

void ko_tile(uint8_t x, uint8_t y, uint8_t t) BANKED {
    add(KI_TILE, x, y, t, 0);
}

/* Blank logical row y of a small box (its two rows), freeing its tiles. */
void ko_clear_row(uint8_t y) BANKED {
    memset(rowbuf, GLYPH_SPACE, TEXT_COLS);
    if (ko_mode == 1) {
        set_win_tiles(1, SMALL_TOP(y), TEXT_COLS, 1, rowbuf);
        set_win_tiles(1, SMALL_TOP(y) + 1, TEXT_COLS, 1, rowbuf);
        if (y && y < 5) row_next[y] = (uint8_t)((y - 1) * 32);
    } else {
        set_win_tiles(1, rowmap[y] ? rowmap[y] : y, TEXT_COLS, 1, rowbuf);
        if (rowmap[y]) set_win_tiles(1, rowmap[y] + 1, TEXT_COLS, 1, rowbuf);
    }
}

/* Small box height for a box of `rows` (English) rows. */
uint8_t ko_small_rows(uint8_t rows) BANKED {
    seq_pool = rows > 6;      /* more than 4 lines (rules card): one shared pool */
    return (uint8_t)((rows - 2) * 2 + 2);
}

static void clear_inside(void) {
    uint8_t y;
    memset(rowbuf, KO_BOX_ATTR, TEXT_COLS);
    VBK_REG = 1;
    for (y = 1; y < 17; y++) set_win_tiles(1, y, TEXT_COLS, 1, rowbuf);
    VBK_REG = 0;
    memset(rowbuf, GLYPH_SPACE, TEXT_COLS);
    for (y = 1; y < 17; y++) set_win_tiles(1, y, TEXT_COLS, 1, rowbuf);
}

/* Tiles a line needs: 2 rows x the columns of its Hangul strings (glyph
   advances summed, as render_line() does). The pool has 256 tiles (VRAM banks
   0 and 1), so a page ends at 8 lines or before the pool would run out. */
static uint8_t page_first[19];
static uint8_t line_cost(uint8_t y) {
    uint8_t i, n, cost = 0;
    uint16_t x, gi;
    const uint8_t *g;
    for (i = 0; i < n_items; i++) {
        if (items[i].y != y || items[i].kind != KI_KO || items[i].s[0] != 1) continue;
        n = items[i].s[1];
        g = items[i].s + 2;
        x = 0;
        while (n--) {
            gi = ((uint16_t)g[0] << 8) | g[1];
            g += 2;
            far_read(ko_bank(ko_glyph_end, gi), ko_glyph_ptr[gi], gbuf, 1);
            x += gbuf[0];
        }
        x = (x + 7) >> 3;
        cost += (uint8_t)(2 * (x > TEXT_COLS ? TEXT_COLS : x));
    }
    return cost;
}

static void show_page(void) {
    uint8_t i, k, first = page_first[page_k], last = page_first[page_k + 1], top, start, need, gaps, l;
    memset(rowmap, 0, sizeof(rowmap));
    gaps = 0;
    for (k = first + 1; k < last; k++) if (lines[k] > lines[k - 1] + 1) gaps++;
    need = (last - first) * 2;
    if (need + gaps > 16) gaps = 0;
    need += gaps;
    start = lines[first];                      /* stay near the English position */
    if (start + need > 17) start = 17 - need;
    if (start < 1) start = 1;
    top = start;
    for (k = first; k < last; k++) {
        if (k > first && gaps && lines[k] > lines[k - 1] + 1) top++;
        rowmap[lines[k]] = top;
        top += 2;
    }
    pool_next = 0; pool_bank1 = 0;
    for (i = 0; i < n_items; i++) {
        l = items[i].y;
        if (l == 17 && !(items[i].kind == KI_KO && items[i].s[0] == 1)) {
            draw_item(&items[i], 16);          /* footer on the bottom border */
        } else if (rowmap[l]) {
            draw_item(&items[i], rowmap[l]);
        }
    }
    if (n_pages > 1)                            /* more on the next page */
        set_win_tile_xy(10, 17, page_k + 1 < n_pages ? GLYPH_ARROW_DOWN : GLYPH_BOX_B);
}

void ko_flush(void) BANKED {
    uint8_t i, y, used, c, k;
    uint16_t budget = 0;
    if (ko_mode != 2) return;
    n_lines = 0;
    n_pages = 0;
    for (y = 1; y < 18; y++) {
        used = 0;
        for (i = 0; i < n_items; i++)
            if (items[i].y == y && (y < 17 || (items[i].kind == KI_KO && items[i].s[0] == 1))) used = 1;
        if (!used) continue;
        c = line_cost(y);
        k = n_pages ? page_first[n_pages - 1] : 0;
        if (!n_lines || n_lines - k >= 8 || budget + c > 256) {
            page_first[n_pages++] = n_lines;
            budget = 0;
        }
        budget += c;
        lines[n_lines++] = y;
    }
    page_first[n_pages] = n_lines;
    page_k = 0;
    show_page();
    ko_mode = 3;
}

/* Next page of a long full-screen box; 0 when it has only one page left. */
uint8_t ko_more(void) BANKED {
    if (n_pages < 2) return 0;
    page_k = page_k + 1 < n_pages ? page_k + 1 : 0;
    clear_inside();
    show_page();
    return 1;
}

uint8_t ko_has_next(void) BANKED { return page_k + 1 < n_pages; }

static void draw_choice(uint8_t x, uint8_t y, const uint8_t *s) {
    if (s[0] == 1) tall(x, y, s + 1);
    else flat(x, y + 1, s);
}

void ko_draw_choice(uint8_t sel) BANKED {
    ko_clear();
    pool_next = 72; pool_end = 128; pool_bank1 = 0;
    set_win_tile_xy(3, 3, sel == 0 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    draw_choice(4, 2, ko_choice_ptr[2 * k_id]);
    set_win_tile_xy(11, 3, sel == 1 ? GLYPH_ARROW_RIGHT : GLYPH_SPACE);
    draw_choice(12, 2, ko_choice_ptr[2 * k_id + 1]);
}

void ko_draw_name(uint8_t x, uint8_t y) BANKED {
    pool_next = 64; pool_end = 128; pool_bank1 = 0;
    tall(x, y - 1, ko_lang_name + 1);
}
