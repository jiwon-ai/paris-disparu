/* The poet's notebook (START), the end screen, the epilogue. ROM bank 7:
   strings go through box_print (it maps bank 1); digits through digit_glyph. */
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
#include "actor.h"
#include "sound.h"

static const uint8_t * const quest_txt[QUEST_COUNT] = {
    txt_q_0, txt_q_1, txt_q_2, txt_q_3, txt_q_4, txt_q_5
};

static uint8_t quests_total(void) {
    uint8_t i, n = 0;
    for (i = 0; i < QUEST_COUNT; i++) n += quest_done(i);
    return n;
}

#define GOALS 11
static const uint8_t * const goal_txt[GOALS * 2] = {
    txt_goal_0a, txt_goal_0b, txt_goal_1a, txt_goal_1b,
    txt_goal_2a, txt_goal_2b, txt_goal_3a, txt_goal_3b,
    txt_goal_4a, txt_goal_4b, txt_goal_5a, txt_goal_5b,
    txt_goal_6a, txt_goal_6b, txt_goal_7a, txt_goal_7b,
    txt_goal_8a, txt_goal_8b, txt_goal_9a, txt_goal_9b,
    txt_goal_10a, txt_goal_10b
};
/* Who to talk to for each goal (a "!" is drawn over them). */
static const uint8_t goal_who[GOALS] = {
    NPC_FERNAND, NPC_ROUX, 0xFF, NPC_PETANQUE, NPC_DOORMAN, NPC_DOORMAN,
    NPC_TRUMPET, NPC_OLDPOET, NPC_LULU, NPC_FERNAND, 0xFF
};

/* What to do next. The three questions can be answered in any order: the
   goal names the first one still open. */
static uint8_t goal(void) {
    if (game_flags & F_SOLVED) return 10;
    if (clue_flags & K_PLATE) return 9;
    if (!(game_flags & F_MET_FERNAND)) return 0;
    if (!(clue_flags & K_TRAIL)) return 1;
    if ((clue_flags & K_READY) == K_READY) return 8;
    if (!(clue_flags & (K_NAME | K_HIDE | K_HEART))) return 2;
    if (!(clue_flags & K_NAME)) return 3;
    if (!(clue_flags & K_HIDE)) {
        if (game_flags & F_INSIDE) return 6;
        return (game_flags & F_MUTE) ? 5 : 4;
    }
    return 7;
}

uint8_t goal_npc(void) BANKED {
    return goal_who[goal()];
}

/* Phone numbers the poet knows, in notebook order. */
#define NUMS 5
static const uint8_t * const num_txt[NUMS] = { txt_num_0, txt_num_1, txt_num_2, txt_num_3, txt_num_4 };
static const uint16_t num_value[NUMS] = { 8400, 4127, 2271, 3301, 6120 };
static const uint8_t num_exch[NUMS] = { EX_ODE, EX_ARC, EX_ROQ, EX_MED, EX_ODE };
static uint8_t num_known(uint8_t k) {
    switch (k) {
    case 0: case 1: return 1;       /* the speaking clock; Ep1's diarist */
    case 2: return (game_flags & F_NUMBER) != 0;
    case 3: return (game_flags & F_MATCHES) != 0;
    default: return (game_flags & F_CARD) != 0;
    }
}
static uint8_t num_next;
uint8_t notebook_exch;

uint16_t notebook_number(void) BANKED {
    uint8_t i;
    for (i = 0; i < NUMS; i++) {
        uint8_t k = num_next++ % NUMS;
        if (num_known(k)) { notebook_exch = num_exch[k]; return num_value[k]; }
    }
    return 0;
}

/* Rules card: EV_* - EV_FIRST indexes the rows (0: none). */
static const uint8_t * const howto_txt[9][5] = {
    { txt_how_o0, txt_how_o1, txt_how_o2, txt_how_o3, txt_how_o4 },   /* EV_TOSS */
    { txt_how_f0, txt_how_f1, txt_how_f2, txt_how_f3, txt_how_f4 },   /* EV_FIND */
    { txt_how_j0, txt_how_j1, txt_how_j2, txt_how_j3, txt_how_j4 },   /* EV_JAZZ */
    { txt_how_b0, txt_how_b1, txt_how_b2, txt_how_b3, txt_how_b4 },   /* EV_PETANQUE */
    { txt_how_t0, txt_how_t1, txt_how_t2, txt_how_t3, txt_how_t4 },   /* EV_CHASE */
    { txt_how_c0, txt_how_c1, txt_how_c2, txt_how_c3, txt_how_c4 },   /* EV_CUPS */
    { 0, 0, 0, 0, 0 },                                                 /* EV_METRO */
    { 0, 0, 0, 0, 0 },                                                 /* EV_END */
    { txt_how_d0, txt_how_d1, txt_how_d2, txt_how_d3, txt_how_d4 },   /* EV_DIAL */
};

uint8_t howto_has(uint8_t ev) BANKED {
    return howto_txt[ev - EV_FIRST][0] != 0;
}

void howto_show(uint8_t ev) BANKED {
    uint8_t i;
    const uint8_t * const *t = howto_txt[ev - EV_FIRST];
    box_open(10);
    box_print(1, 1, txt_howto);
    for (i = 0; i < 5; i++) box_print(1, 3 + i, t[i]);
    box_print(1, 8, txt_howto_go);
}

/* The current goal on two rows of the open box. */
void goal_print(uint8_t y) BANKED {
    uint8_t g = goal() * 2;
    box_print(1, y, goal_txt[g]);
    box_print(1, y + 1, goal_txt[g + 1]);
}

static const uint8_t * const clock_txt[6] = {
    txt_clock_0, txt_clock_1, txt_clock_2, txt_clock_3, txt_clock_4, txt_clock_5
};

void notebook_show(void) BANKED {
    uint8_t i, g = goal() * 2;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_nb_title);
    box_print(14, 1, clock_txt[dawn_stage > 5 ? 5 : dawn_stage]);
    box_print(1, 3, goal_txt[g]);
    box_print(1, 4, goal_txt[g + 1]);
    box_print(1, 6, txt_nb_quests);
    box_tile(15, 6, digit_glyph(quests_total()));
    box_print(16, 6, txt_of_6);
    for (i = 0; i < QUEST_COUNT; i++) {
        box_print(1, 7 + i, quest_done(i) ? txt_x_done : txt_x_todo);
        box_print(4, 7 + i, quest_txt[i]);
    }
    box_print(1, 14, txt_nb_coins);
    box_tile(10, 14, digit_glyph(money));
    box_print(1, 15, music_muted ? txt_nb_snd_off : txt_nb_snd_on);
    box_print(1, 17, txt_nb_next);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* Page 2: phone numbers found. */
static void phone_show(void) {
    uint8_t i, y = 3;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_nb_phone);
    for (i = 0; i < NUMS; i++)
        if (num_known(i)) { box_print(1, y, num_txt[i]); y += 2; }
    box_print(1, 14, txt_nb_ph_1);
    box_print(1, 15, txt_nb_ph_2);
    box_print(1, 17, txt_nb_poem);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* Page 3: the poem. Lines are earned by what each player did, so every
   poem differs; at dawn the poet finishes it (original text). */
#define POEM_LINES 10
static const uint8_t * const poem_txt[POEM_LINES] = {
    txt_pl_0, txt_pl_1, txt_pl_2, txt_pl_3, txt_pl_4,
    txt_pl_5, txt_pl_6, txt_pl_7, txt_pl_8, txt_pl_9
};

static uint8_t poem_has(uint8_t i) {
    switch (i) {
    case 5: return extra_flags & X_MOTHER;
    case 6: return extra_flags & X_TRAMP;
    case 7: return extra_flags & X_THIEF;
    case 8: return extra_flags & X_VIOLET;
    case 9: return secret_total() >= 5;
    default: return quest_done(i);
    }
}

static uint8_t poem_print(uint8_t y) {
    uint8_t i;
    for (i = 0; i < POEM_LINES; i++) {
        if (!poem_has(i)) continue;
        if (i == 3 && (extra_flags & X_POET_YOUNG)) box_print(1, y++, txt_pl_3y);
        else box_print(1, y++, poem_txt[i]);
    }
    return y;
}

static void poem_show(void) {
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_poem_title);
    if (poem_print(3) == 3) box_print(1, 3, txt_poem_none);
    box_print(1, 17, txt_nb_extras);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

static void poem_final(void) {
    uint8_t y;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_poem_end_title);
    y = poem_print(3) + 1;
    box_print(1, y, (extra_flags & X_POET_SING) ? txt_pc_a : txt_pc_b);
    box_print(1, y + 1, txt_pc_c);
    box_print(1, 17, txt_poem_end);
    box_flush();
}

static uint8_t nb_page;

void nb_open(void) BANKED {
    nb_page = 0;
    notebook_show();
}

void nb_turn(uint8_t right) BANKED {
    nb_page = right ? (nb_page == 4 ? 0 : nb_page + 1) : (nb_page ? nb_page - 1 : 4);
    switch (nb_page) {
    case 1:  phone_show(); break;
    case 2:  poem_show(); break;
    case 3:  extras_show(); break;
    case 4:  album_show(); break;
    default: notebook_show();
    }
}

/* Page 4: side goals, each with where to look. */
static const uint8_t * const extra_txt[X_COUNT * 2] = {
    txt_x_0, txt_x_0h, txt_x_1, txt_x_1h, txt_x_2, txt_x_2h, txt_x_3, txt_x_3h
};
static const uint8_t extra_bit[X_COUNT] = { X_MOTHER, X_TRAMP, X_THIEF, X_VIOLET };

void extras_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_x_title);
    box_tile(15, 1, digit_glyph(extra_total()));
    box_print(16, 1, txt_extra_of);
    for (i = 0; i < X_COUNT; i++) {
        box_print(1, 3 + i * 3, (extra_flags & extra_bit[i]) ? txt_x_done : txt_x_todo);
        box_print(4, 3 + i * 3, extra_txt[i * 2]);
        box_print(4, 4 + i * 3, extra_txt[i * 2 + 1]);
    }
    box_print(1, 15, txt_secrets);
    box_tile(14, 15, digit_glyph(secret_total() / 10));
    box_tile(15, 15, digit_glyph(secret_total() % 10));
    box_print(16, 15, txt_secret_of);
    box_print(1, 17, txt_nb_album);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* Page 5: the vanished-Paris album (facts: docs/episode2/RESEARCH.md). */
static const uint8_t * const album_txt[ALBUM_COUNT * 2] = {
    txt_al_0, txt_al_0f, txt_al_1, txt_al_1f, txt_al_2, txt_al_2f,
    txt_al_3, txt_al_3f, txt_al_4, txt_al_4f, txt_al_5, txt_al_5f
};

void album_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_al_title);
    for (i = 0; i < ALBUM_COUNT; i++) {
        if (album_flags & (1 << i)) {
            box_print(1, 3 + i * 2, album_txt[i * 2]);
            box_print(1, 4 + i * 2, album_txt[i * 2 + 1]);
        } else {
            box_print(1, 3 + i * 2, txt_al_none);
        }
    }
    box_tile(16, 1, digit_glyph(album_total()));
    box_print(17, 1, txt_al_of);
    box_print(1, 17, txt_nb_prev);
    box_print(14, 17, txt_nb_close);
    box_flush();
}

/* After the end screen: a look at the next episode. */
static const uint8_t * const teaser_txt[13] = {
    txt_next_0, 0, txt_next_1, txt_next_2, txt_next_3, txt_next_4, 0,
    txt_next_5, txt_next_6, txt_next_7, 0, txt_next_8, txt_next_9
};   /* Episode 3 */

void teaser_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    for (i = 0; i < 13; i++)
        if (teaser_txt[i]) box_print(1, 2 + i, teaser_txt[i]);
    box_print(1, 16, txt_end_11);
    box_flush();
}

/* A random background sound for this room; returns frames to the next. */
uint16_t ambience_play(uint8_t r) BANKED {
    switch (rooms[cur_room].ambience) {
    case AMBIENCE_STREET:   /* only near the south side, where the market is */
        if (player.y > 100) sound_market(1);
        break;
    case AMBIENCE_MARKET:
        sound_market(2 + (r & 1));
        return 60 + (r & 127);
    case AMBIENCE_HALL:
        sound_market(2);
        break;
    case AMBIENCE_RIVER:
        sound_water();
        return 70 + (r & 63);
    case AMBIENCE_CAFE:
        sound_clink();
        return 180 + r;
    default:
        return 255;
    }
    return 90 + (r & 127);
}

void rat_update(uint8_t walking) BANKED { (void)walking; }

/* Bank-1 sprites (cameos, Mme Roux, the waiter...) are kept in ROM and
   copied, per room, into VRAM bank 1 tiles LOW_FIXED.. (POOL_SLOTS of them):
   any number of such characters in the game, up to POOL_SLOTS per room. */
uint8_t npc_tile[8];

void pool_load(void) BANKED {
    static uint8_t buf[64];
    uint8_t i, j, used = 0, slot_frame[POOL_SLOTS];
    const room_def_t *r = &rooms[cur_room];
    for (i = 0; i < r->npc_count && i < 8; i++) {
        uint8_t f = r->npcs[i].frame;
        if (!(r->npcs[i].palette & S_BANK1) || f < LOW_FIXED) { npc_tile[i] = f; continue; }
        for (j = 0; j < used && slot_frame[j] != f; j++) ;
        if (j == used) {
            if (used == POOL_SLOTS) { npc_tile[i] = 0; continue; }
            slot_frame[used++] = f;
            far_read(PORTRAIT_BANK, low_tiles + (uint16_t)f * 16, buf, 64);
            VBK_REG = 1;
            set_sprite_data(LOW_FIXED + j * 4, 4, buf);
            VBK_REG = 0;
        }
        npc_tile[i] = LOW_FIXED + j * 4;
    }
}

/* A speech bubble over the NPC who calls out (index in this room, or
   NO_BARK): it bobs until the player walks away; A opens the talk. */
#define BUBBLE_OAM 35
void bubble_update(uint8_t npc) BANKED {
    const npc_def_t *n;
    if (npc >= rooms[cur_room].npc_count) {
        move_sprite(BUBBLE_OAM, 0, 0);
        return;
    }
    n = &rooms[cur_room].npcs[npc];
    set_sprite_tile(BUBBLE_OAM, 254);
    set_sprite_prop(BUBBLE_OAM, 3);
    move_sprite(BUBBLE_OAM, (uint8_t)(n->x + 18), (uint8_t)(n->y + 7 - ((sys_time >> 4) & 1)));
}

/* The caption when a coin is picked up. */
void coin_note(void) BANKED {
    box_open(5);
    box_print(1, 1, txt_coin_0);
    box_print(1, 2, txt_coin_1);
    box_print(1, 3, txt_coin_2);
}

/* The ending: the sky pales over 2 s (16 steps from tonight's colours to
   bg_day), then a caption. Returns 1 on A/START after the caption. */
static uint8_t day_t;
static palette_color_t day_mix[32];

/* A café table on the pavement of rue Rambuteau, drawn over the poet's
   legs (BG priority) so he seems to sit at it. */
void day_start(void) BANKED {
    day_t = 0;
}

uint8_t day_update(uint8_t pressed) BANKED {
    uint8_t i, k;
    if (day_t >= 160) {             /* A: the finished poem, then A: the end screen */
        if (day_t == 161 && (pressed & (J_UP | J_DOWN))) {   /* Korean: the other half */
            if (box_more()) sound_blip();
            return 0;
        }
        if (!(pressed & (J_A | J_START))) return 0;
        if (day_t == 161) return 1;
        day_t = 161;
        poem_final();
        sound_confirm();
        return 0;
    }
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
    static const uint8_t * const rank_txt[5] = {
        txt_rank_0, txt_rank_1, txt_rank_2, txt_rank_3, txt_rank_4
    };
    uint8_t s = quests_total() + extra_total() + secret_total() + album_total();   /* of 26 */
    HIDE_SPRITES;
    box_open(SCREEN_H / 8);
    box_print(6, 1, txt_end_1);
    box_print(1, 3, txt_end_2);
    box_tile(14, 3, digit_glyph(quests_total()));
    box_print(15, 3, txt_of_6);
    box_print(1, 4, txt_end_x);
    box_tile(14, 4, digit_glyph(extra_total()));
    box_print(15, 4, txt_extra_of);
    box_print(1, 5, txt_secrets);
    box_tile(13, 5, digit_glyph(secret_total() / 10));
    box_tile(14, 5, digit_glyph(secret_total() % 10));
    box_print(15, 5, txt_secret_of);
    box_print(1, 6, txt_end_al);
    box_tile(14, 6, digit_glyph(album_total()));
    box_print(15, 6, txt_al_of);
    box_print(1, 8, txt_end_3);
    box_print(1, 9, txt_end_4);
    box_print(1, 10, txt_end_5);
    box_print(1, 11, txt_end_6);
    box_print(1, 12, txt_end_7);
    box_print(1, 14, txt_rank);
    box_print(7, 14, rank_txt[s < 8 ? 0 : s < 14 ? 1 : s < 20 ? 2 : s < 25 ? 3 : 4]);
    box_print(1, 16, txt_end_11);
    box_flush();
}

/* Epilogue: dawn of Monday 3 March 1969, the lorries leave for Rungis. */
static const uint8_t * const epi_txt[12] = {
    txt_ep_0, 0, txt_ep_1, txt_ep_2, txt_ep_3, txt_ep_4, txt_ep_5, 0,
    txt_ep_6, txt_ep_7, txt_ep_8, txt_ep_9
};
void epilogue_show(void) BANKED {
    uint8_t i;
    box_open(SCREEN_H / 8);
    for (i = 0; i < 12; i++)
        if (epi_txt[i]) box_print(1, 2 + i, epi_txt[i]);
    if ((extra_flags & X_ALL) == X_ALL) box_print(1, 15, txt_ep_bonus);
    box_print(1, 17, txt_ep_next);
    box_flush();
}

/* The short explanation after the title: three pages, A = next, B = skip.
   Each page: rows of text, 0 = blank row. */
static const uint8_t * const intro_pages[3][11] = {
    { txt_intro_a0, 0, txt_intro_a1, txt_intro_a2, txt_intro_a3, 0, txt_intro_a4, txt_intro_a8,
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
