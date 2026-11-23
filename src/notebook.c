/* The player's notebook (START) and the end screen. */
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "notebook.h"

static const uint8_t * const clue_txt[CLUE_COUNT] = {
    txt_clue_0, txt_clue_1, txt_clue_2, txt_clue_3, txt_clue_4, txt_clue_5
};
static const uint8_t * const goal_txt[14] = {
    txt_goal_0a, txt_goal_0b, txt_goal_1a, txt_goal_1b,
    txt_goal_2a, txt_goal_2b, txt_goal_3a, txt_goal_3b,
    txt_goal_4a, txt_goal_4b, txt_goal_5a, txt_goal_5b,
    txt_goal_6a, txt_goal_6b
};
static const uint8_t * const num_txt[4] = { txt_num_0, txt_num_1, txt_num_2, txt_num_3 };
static const uint16_t num_flag[4] = {
    FLAG_SAW_PHONE, FLAG_MET_STRANGER, FLAG_KNOW_7302, FLAG_MET_PAINTER
};

/* What to do next, from the furthest step reached. */
static uint8_t goal(void) {
    if ((clue_flags & CLUE_READY) == CLUE_READY) return 3;
    if (clue_flags & CLUE_COLTIN) {    /* the first missing clue */
        if (!(clue_flags & CLUE_BLEU)) return (clue_flags & CLUE_COURIER) ? 4 : 2;
        return (game_flags & FLAG_KNOW_7302) ? 6 : 5;
    }
    if (clue_flags & CLUE_CASE) return 1;
    return 0;
}

/* The current goal on two rows of the open box (also under "New clue!"). */
void goal_print(uint8_t y) {
    uint8_t g = goal() * 2;
    box_print(1, y, goal_txt[g]);
    box_print(1, y + 1, goal_txt[g + 1]);
}

void notebook_show(void) {
    uint8_t i, y, g = goal() * 2;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_nb_title);
    set_win_tile_xy(15, 1, txt_digits[clue_total()]);
    box_print(16, 1, txt_nb_of);
    box_print(1, 2, goal_txt[g]);
    box_print(1, 3, goal_txt[g + 1]);
    box_print(1, 5, txt_nb_clues);   /* entries below fill the 18 inner columns */
    y = 6;
    for (i = 0; i < CLUE_COUNT; i++)
        if (clue_flags & (1 << i)) box_print(1, y++, clue_txt[i]);
    box_print(1, 12, txt_nb_phone);
    y = 13;
    for (i = 0; i < 4; i++)
        if (game_flags & num_flag[i]) box_print(1, y++, num_txt[i]);
    box_print(14, 17, txt_nb_close);
}

void end_show(void) {
    HIDE_SPRITES;
    box_open(SCREEN_H / 8);
    box_print(6, 2, txt_end_1);
    box_print(1, 4, txt_end_2);
    set_win_tile_xy(14, 4, txt_digits[clue_total()]);
    box_print(1, 6, txt_end_3);
    box_print(1, 7, txt_end_4);
    box_print(1, 8, txt_end_5);
    box_print(1, 9, txt_end_6);
    box_print(1, 10, txt_end_7);
    box_print(1, 11, txt_end_8);
    box_print(1, 12, txt_end_9);
    box_print(1, 14, txt_end_10);
    box_print(1, 16, txt_end_11);
}

/* The short explanation after the title: three pages, A = next, B = skip.
   Each page: rows of text, 0 = blank row. */
static const uint8_t * const intro_pages[3][11] = {
    { txt_intro_a0, 0, txt_intro_a1, txt_intro_a2, txt_intro_a3, 0, txt_intro_a4, 0,
      txt_intro_a5, txt_intro_a6, txt_intro_a7 },
    { txt_intro_b0, 0, txt_intro_b1, txt_intro_b2, txt_intro_b3, txt_intro_b4, 0,
      txt_intro_b5, txt_intro_b6, txt_intro_b7, txt_intro_b8 },
    { txt_intro_c0, 0, txt_intro_c1, txt_intro_c2, txt_intro_c3, txt_intro_c4, 0,
      0, txt_intro_c5, txt_intro_c6, 0 },
};

void intro_show(uint8_t page) {
    uint8_t i;
    box_open(SCREEN_H / 8);
    for (i = 0; i < 11; i++)
        if (intro_pages[page][i]) box_print(2, 3 + i, intro_pages[page][i]);
    box_print(2, 16, txt_intro_more);
}
