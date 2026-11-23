/* The player's notebook (START) and the end screen. */
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "notebook.h"

static const uint8_t * const clue_txt[CLUE_COUNT] = {
    txt_clue_0, txt_clue_1, txt_clue_2, txt_clue_3, txt_clue_4, txt_clue_5
};
static const uint8_t * const goal_txt[8] = {
    txt_goal_0a, txt_goal_0b, txt_goal_1a, txt_goal_1b,
    txt_goal_2a, txt_goal_2b, txt_goal_3a, txt_goal_3b
};
static const uint8_t * const num_txt[4] = { txt_num_0, txt_num_1, txt_num_2, txt_num_3 };
static const uint16_t num_flag[4] = {
    FLAG_SAW_PHONE, FLAG_MET_STRANGER, FLAG_KNOW_7302, FLAG_MET_PAINTER
};

/* What to do next, from the furthest step reached. */
static uint8_t goal(void) {
    if ((clue_flags & CLUE_READY) == CLUE_READY) return 3;
    if (clue_flags & CLUE_COLTIN) return 2;
    if (clue_flags & CLUE_CASE) return 1;
    return 0;
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
