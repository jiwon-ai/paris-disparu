/* The player's notebook (START) and the end screen. ROM bank 7: strings go
   through box_print (it maps bank 1); digits through digit_glyph. */
#pragma bank 7
#include <gb/gb.h>
#include "assets.h"
#include "dialogue.h"
#include "game.h"
#include "notebook.h"
#include "events.h"
#include "world.h"
#include "music.h"
#include "ko.h"
#include "sound.h"

static const uint8_t * const quest_txt[QUEST_COUNT] = {
    txt_q_0, txt_q_1, txt_q_2, txt_q_3, txt_q_4
};

static uint8_t quests_total(void) {
    uint8_t i, n = 0;
    for (i = 0; i < QUEST_COUNT; i++) n += quest_done(i);
    return n;
}
static const uint8_t * const goal_txt[20] = {
    txt_goal_0a, txt_goal_0b, txt_goal_1a, txt_goal_1b,
    txt_goal_2a, txt_goal_2b, txt_goal_3a, txt_goal_3b,
    txt_goal_4a, txt_goal_4b, txt_goal_5a, txt_goal_5b,
    txt_goal_6a, txt_goal_6b, txt_goal_7a, txt_goal_7b,
    txt_goal_8a, txt_goal_8b, txt_goal_9a, txt_goal_9b
};
static const uint8_t * const num_txt[4] = { txt_num_0, txt_num_1, txt_num_2, txt_num_3 };
static const uint16_t num_flag[4] = {
    FLAG_SAW_PHONE, FLAG_MET_STRANGER, FLAG_KNOW_7302, FLAG_MET_PAINTER
};

/* What to do next, from the furthest step reached. */
static uint8_t goal(void) {
    if ((clue_flags & CLUE_READY) == CLUE_READY) return 3;
    if (clue_flags & CLUE_COLTIN) {    /* the first missing clue */
        if (!(clue_flags & CLUE_BLEU)) {
            if (clue_flags & CLUE_COURIER) return 4;
            return (game_flags & (FLAG_HAS_JETON | FLAG_PHONE_DONE)) ? 2 : 9;
        }
        if (quest_flags & QF_SOUP) return 8;
        return (quest_flags & QF_ROUX) ? 6 : 5;
    }
    if (game_flags & FLAG_MET_CAFE) return 7;   /* the coffee rush */
    if (clue_flags & CLUE_CASE) return 1;
    return 0;
}

/* Who to talk to for each goal (a "!" is drawn over them). */
static const uint8_t goal_who[10] = {
    NPC_STRANGER, NPC_CAFE, 0xFF, NPC_FERNAND, NPC_CAFE,
    NPC_ROUX, NPC_CAFE, NPC_CAFE, NPC_ROUX, NPC_WORKER
};

uint8_t goal_npc(void) BANKED {
    return goal_who[goal()];
}

static const uint16_t num_value[4] = { 614, 4127, 7302, 2508 };
static const uint8_t num_exch[4] = { EX_ROQ, EX_ARC, EX_MED, EX_PAS };
static uint8_t num_next;
uint8_t notebook_exch;

uint16_t notebook_number(void) BANKED {
    uint8_t i;
    for (i = 0; i < 4; i++) {
        uint8_t k = num_next++ & 3;
        if (game_flags & num_flag[k]) { notebook_exch = num_exch[k]; return num_value[k]; }
    }
    return 0;
}

/* Rules card: EV_* - EV_FIRST indexes the rows (EV_END has none). */
static const uint8_t * const howto_txt[8][5] = {
    { txt_how_t0, txt_how_t1, txt_how_t2, txt_how_t3, txt_how_t4 },   /* EV_CHASE */
    { txt_how_k0, txt_how_k1, txt_how_k2, txt_how_k3, txt_how_k4 },   /* EV_CATCH */
    { txt_how_c0, txt_how_c1, txt_how_c2, txt_how_c3, txt_how_c4 },   /* EV_CUPS */
    { txt_how_s0, txt_how_s1, txt_how_s2, txt_how_s3, txt_how_s4 },   /* EV_SERVE */
    { txt_how_p0, txt_how_l1, txt_how_l2, txt_how_l3, txt_how_l4 },   /* EV_PUNCH */
    { 0, 0, 0, 0, 0 },                                                 /* EV_END */
    { txt_how_d0, txt_how_d1, txt_how_d2, txt_how_d3, txt_how_d4 },   /* EV_DIAL */
    { txt_how_l0, txt_how_l1, txt_how_l2, txt_how_l3, txt_how_l4 },   /* EV_LIFT */
};

void howto_show(uint8_t ev) BANKED {
    uint8_t i;
    const uint8_t * const *t = howto_txt[ev - EV_FIRST];
    box_open(10);
    box_print(1, 1, txt_howto);
    for (i = 0; i < 5; i++) box_print(1, 3 + i, t[i]);
    box_print(1, 8, txt_howto_go);
}

/* The current goal on two rows of the open box (also under "New clue!"). */
void goal_print(uint8_t y) BANKED {
    uint8_t g = goal() * 2;
    box_print(1, y, goal_txt[g]);
    box_print(1, y + 1, goal_txt[g + 1]);
}

static const uint8_t * const clock_txt[5] = {
    txt_clock_0, txt_clock_1, txt_clock_2, txt_clock_3, txt_clock_4
};

void notebook_show(void) BANKED {
    uint8_t i, y, g = goal() * 2;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_nb_title);
    box_tile(15, 1, digit_glyph(quests_total()));
    box_print(16, 1, txt_nb_of);
    box_print(1, 2, goal_txt[g]);
    box_print(1, 3, goal_txt[g + 1]);
    box_print(1, 5, txt_nb_quests);
    box_print(14, 5, clock_txt[dawn_stage]);   /* the hour: dawn comes with each quest */
    for (i = 0; i < QUEST_COUNT; i++) {
        box_print(1, 6 + i, quest_done(i) ? txt_x_done : txt_x_todo);
        box_print(4, 6 + i, quest_txt[i]);
    }
    (void)y;
    box_print(1, 11, music_muted ? txt_nb_snd_off : txt_nb_snd_on);   /* how to mute */
    box_print(1, 12, txt_nb_phone);
    y = 13;
    for (i = 0; i < 4; i++)
        if (game_flags & num_flag[i]) box_print(1, y++, num_txt[i]);
    box_print(1, 17, txt_nb_next);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* Second page: optional side goals, each with where to look. */
static const uint8_t * const extra_txt[X_COUNT * 2] = {
    txt_x_0, txt_x_0h, txt_x_1, txt_x_1h, txt_x_2, txt_x_2h
};

void extras_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_x_title);
    box_tile(15, 1, digit_glyph(extra_total()));
    box_print(16, 1, txt_extra_of);
    for (i = 0; i < X_COUNT; i++) {
        box_print(1, 3 + i * 3, (extra_flags & (1 << i)) ? txt_x_done : txt_x_todo);
        box_print(5, 3 + i * 3, extra_txt[i * 2]);
        box_print(5, 4 + i * 3, extra_txt[i * 2 + 1]);
    }
    box_print(1, 13, txt_x_note_1);
    box_print(1, 14, txt_x_note_2);
    box_print(1, 15, txt_secrets);
    box_tile(15, 15, digit_glyph(secret_total()));
    box_print(16, 15, txt_secret_of);
    box_print(1, 17, txt_nb_album);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* Third page: the vanished-Paris album. Each object found shows a one-line
   fact read in a source (docs/HISTORICAL_RESEARCH.md); the rest show ???. */
#define ALBUM_COUNT 5
static const uint8_t * const album_txt[ALBUM_COUNT * 2] = {
    txt_al_0, txt_al_0f, txt_al_1, txt_al_1f, txt_al_2, txt_al_2f,
    txt_al_3, txt_al_3f, txt_al_4, txt_al_4f
};

static uint8_t album_has(uint8_t i) {
    switch (i) {
    case 0:  return (clue_flags & CLUE_COLTIN) != 0;
    case 1:  return (clue_flags & CLUE_BLEU) != 0;
    case 2:  return (game_flags & FLAG_TICKET) != 0;
    case 3:  return (clue_flags & CLUE_PLATE) != 0;
    default: return (quest_flags & QF_SOUP) || (clue_flags & CLUE_PLATE);
    }
}

void album_show(void) BANKED {
    uint8_t i, n = 0;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_al_title);
    for (i = 0; i < ALBUM_COUNT; i++) {
        if (album_has(i)) {
            n++;
            box_print(1, 3 + i * 3, album_txt[i * 2]);
            box_print(1, 4 + i * 3, album_txt[i * 2 + 1]);
        } else {
            box_print(1, 3 + i * 3, txt_al_none);
        }
    }
    box_tile(16, 1, digit_glyph(n));
    box_print(17, 1, txt_nb_of);       /* "/5" */
    box_print(1, 17, txt_nb_myst);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

static uint8_t album_total(void) {
    uint8_t i, n = 0;
    for (i = 0; i < ALBUM_COUNT; i++) n += album_has(i);
    return n;
}

/* Fourth page: curiosities. Unknown: ???. Found: what it looks like and who
   might know. Identified: what it was (facts: docs/HISTORICAL_RESEARCH.md). */
static const uint8_t * const curio_txt[CURIO_COUNT * 4] = {
    txt_cu_0q, txt_cu_0h, txt_cu_0, txt_cu_0f,
    txt_cu_1q, txt_cu_1h, txt_cu_1, txt_cu_1f,
    txt_cu_2q, txt_cu_2h, txt_cu_2, txt_cu_2f
};

void curio_show(void) BANKED {
    uint8_t i, y;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_cu_title);
    box_tile(16, 1, digit_glyph(curio_total()));
    box_print(17, 1, txt_cu_of);
    for (i = 0; i < CURIO_COUNT; i++) {
        y = 3 + i * 4;
        if (curio_flags & (0x10 << i)) {
            box_print(1, y, curio_txt[i * 4 + 2]);
            box_print(1, y + 1, curio_txt[i * 4 + 3]);
        } else if (curio_flags & (1 << i)) {
            box_print(1, y, curio_txt[i * 4]);
            box_print(1, y + 1, curio_txt[i * 4 + 1]);
        } else {
            box_print(1, y, txt_al_none);
        }
    }
    box_print(1, 17, txt_nb_prev);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* After the end screen: a look at the next episode. */
static const uint8_t * const teaser_txt[13] = {
    txt_next_0, 0, txt_next_1, txt_next_2, txt_next_3, txt_next_4, 0,
    txt_next_5, txt_next_6, txt_next_7, 0, txt_next_8, txt_next_9
};

void teaser_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    for (i = 0; i < 13; i++)
        if (teaser_txt[i]) box_print(1, 2 + i, teaser_txt[i]);
    box_print(1, 16, txt_end_11);
    box_flush();
}

/* The ending: the sky pales over 2 s (16 steps from tonight's colours to
   bg_day), then a caption. Returns 1 on A/START after the caption. */
static uint8_t day_t;
static palette_color_t day_mix[32];

void day_start(void) BANKED { day_t = 0; }

uint8_t day_update(uint8_t pressed) BANKED {
    uint8_t i, k;
    if (day_t >= 160) return (pressed & (J_A | J_START)) != 0;
    day_t++;
    if (!(day_t & 7) && day_t <= 128) {
        k = day_t >> 3;
        far_read(ASSET_BANK, (const uint8_t *)bg_day, (uint8_t *)day_mix, 64);
        for (i = 0; i < 32; i++) {
            uint16_t a = bg_live[i], b = day_mix[i];
            int16_t r = a & 31, g = (a >> 5) & 31, bl = (a >> 10) & 31;
            r += (((int16_t)(b & 31) - r) * k) >> 4;
            g += (((int16_t)((b >> 5) & 31) - g) * k) >> 4;
            bl += (((int16_t)((b >> 10) & 31) - bl) * k) >> 4;
            day_mix[i] = (uint16_t)r | ((uint16_t)g << 5) | ((uint16_t)bl << 10);
        }
        wait_vbl_done();
        set_bkg_palette(0, 8, day_mix);
    }
    if (day_t == 160) {
        box_open(6);
        box_print(1, 1, txt_day_0);
        box_print(1, 2, txt_day_1);
        box_print(1, 3, txt_day_2);
        box_print(1, 4, txt_day_3);
        box_flush();
        sound_confirm();
    }
    return 0;
}

void end_show(void) BANKED {
    HIDE_SPRITES;
    box_open(SCREEN_H / 8);
    box_print(6, 2, txt_end_1);
    box_print(1, 4, txt_end_2);
    box_tile(14, 4, digit_glyph(clue_total()));
    box_print(15, 4, txt_of_6);
    box_print(1, 5, txt_end_x);
    box_tile(14, 5, digit_glyph(extra_total()));
    box_print(15, 5, txt_extra_of);
    box_print(1, 6, txt_end_3);
    box_print(1, 7, txt_end_4);
    box_print(1, 8, txt_end_5);
    box_print(1, 9, txt_end_6);
    box_print(1, 10, txt_end_7);
    box_print(1, 11, txt_end_8);
    box_print(1, 12, txt_end_9);
    box_print(1, 13, txt_secrets);
    box_tile(15, 13, digit_glyph(secret_total()));
    box_print(16, 13, txt_secret_of);
    box_print(1, 14, txt_end_10);
    {   /* everything found, out of 26 */
        static const uint8_t * const rank_txt[5] = {
            txt_rank_0, txt_rank_1, txt_rank_2, txt_rank_3, txt_rank_4
        };
        uint8_t s = clue_total() + extra_total() + secret_total() + album_total() + curio_total();
        box_print(1, 15, txt_rank);
        box_print(7, 15, rank_txt[s < 9 ? 0 : s < 15 ? 1 : s < 21 ? 2 : s < 26 ? 3 : 4]);
    }
    box_print(1, 16, txt_end_11);
    box_flush();
}

/* The short explanation after the title: three pages, A = next, B = skip.
   Each page: rows of text, 0 = blank row. */
static const uint8_t * const intro_pages[3][11] = {
    { txt_intro_a0, 0, txt_intro_a1, txt_intro_a2, txt_intro_a3, 0, txt_intro_a4, 0,
      txt_intro_a5, txt_intro_a6, txt_intro_a7 },
    { txt_intro_b0, 0, txt_intro_b1, txt_intro_b2, txt_intro_b3, txt_intro_b4, 0,
      txt_intro_b5, txt_intro_b6, txt_intro_b7, txt_intro_b8 },
    { txt_intro_c0, 0, txt_intro_c1, txt_intro_c2, txt_intro_c3, txt_intro_c4, txt_intro_c7,
      0, txt_intro_c5, txt_intro_c6, 0 },
};

void intro_show(uint8_t page) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    for (i = 0; i < 11; i++)
        if (intro_pages[page][i]) box_print(2, 3 + i, intro_pages[page][i]);
    box_print(2, 16, txt_intro_more);
    box_flush();
}
