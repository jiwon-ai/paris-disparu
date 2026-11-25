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
#include "actor.h"
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

/* Second page: the poem. The hero is a young poet who came down to the
   market for words. Lines are earned by what each player did (quests,
   side goals, mysteries, secrets, people met), so every poem differs; at
   daybreak the poet finishes it on a café terrace (original text). */
#define POEM_LINES 10
static const uint8_t * const poem_txt[POEM_LINES] = {
    txt_pl_0, txt_pl_1, txt_pl_2, txt_pl_3, txt_pl_4,
    txt_pl_5, txt_pl_6, txt_pl_7, txt_pl_8, txt_pl_9
};

static uint8_t poem_has(uint8_t i) {
    uint8_t n = 0;
    uint16_t m = cameo_met;
    switch (i) {
    case 4: return extra_flags & X_TICKETS;
    case 5: return extra_flags & X_BUTCHER;
    case 6: return extra_flags & X_PAINTER;
    case 7: return curio_flags & 0x70;          /* a mystery solved */
    case 8: return secret_total() >= 5;
    case 9:                                     /* six people of the night met */
        for (; m; m >>= 1) n += m & 1;
        return n >= 6;
    default: return quest_done(i);
    }
}

/* The lines earned so far, one under the other from row y; returns the next row. */
static uint8_t poem_print(uint8_t y) {
    uint8_t i;
    for (i = 0; i < POEM_LINES; i++)
        if (poem_has(i)) box_print(1, y++, poem_txt[i]);
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

/* The ending: the whole poem, closed by Fernand's choice. */
static void poem_final(void) {
    uint8_t y;
    box_open(SCREEN_H / 8);
    box_print(1, 1, txt_poem_end_title);
    y = poem_print(3) + 1;
    box_print(1, y, (extra_flags & X_STAYED) ? txt_pc_sa : txt_pc_ga);
    box_print(1, y + 1, (extra_flags & X_STAYED) ? txt_pc_sb : txt_pc_gb);
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
    case 1:  poem_show(); break;
    case 2:  extras_show(); break;
    case 3:  album_show(); break;
    case 4:  curio_show(); break;
    default: notebook_show();
    }
}

/* Third page: optional side goals, each with where to look. */
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

/* Fourth page: the vanished-Paris album. Each object found shows a one-line
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

/* Fifth page: curiosities. Unknown: ???. Found: what it looks like and who
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

/* A rat scurries along the setts by the Bourse, stops, turns, runs back. */
#define RAT_OAM 18
static uint8_t rat_x = 24, rat_wait = 40, rat_left, rat_on;

/* A pigeon pecks by the parapet on the quai; it flies off if you come near
   and comes back a while later. Same OAM pair as the rat (another room). */
static uint8_t pg_on, pg_t, pg_fly;
#define PG_X 76   /* on the pavement between the two lamps by the bridge */
#define PG_Y 84
static int16_t pg_x = PG_X, pg_y = PG_Y;

static void pigeon_update(void) {
    int16_t dx, dy;
    if (!pg_on) {
        static uint8_t buf[128];
        far_read(PORTRAIT_BANK, rat_tiles + 64, buf, 128);
        VBK_REG = 1;
        set_sprite_data(PIGEON_TILE, 8, buf);
        VBK_REG = 0;
        pg_on = 1;
    }
    pg_t++;
    if (pg_fly) {
        if (pg_fly == 1) {                      /* flying up and away */
            pg_x -= 1; pg_y -= 2;
            if (pg_y < -16) { pg_fly = 2; pg_t = 0; }
        } else if (pg_t == 255) {               /* back when you're gone */
            dx = player.x - PG_X; dy = player.y - PG_Y;
            if (dx > 48 || dx < -48 || dy > 40 || dy < -40) { pg_fly = 0; pg_x = PG_X; pg_y = PG_Y; }
        }
    } else {
        dx = player.x - pg_x; dy = player.y - pg_y;
        if (dx < 24 && dx > -24 && dy < 20 && dy > -20) pg_fly = 1;
    }
    if (pg_fly == 2) { move_sprite(RAT_OAM, 0, 0); move_sprite(RAT_OAM + 1, 0, 0); return; }
    sprite16(RAT_OAM, pg_x, pg_y, PIGEON_TILE + ((pg_fly || ((pg_t >> 4) & 3) == 0) ? 0 : 4),
             4 | S_BANK1);
}

void rat_update(uint8_t walking) BANKED {
    if (!walking || cur_room != ROOM_QUAI) {
        if (pg_on) { move_sprite(RAT_OAM, 0, 0); move_sprite(RAT_OAM + 1, 0, 0); pg_on = 0; }
    }
    if (walking && cur_room == ROOM_QUAI) { pigeon_update(); return; }
    if (!walking || cur_room != ROOM_BOURSE) {
        if (rat_on) {
            move_sprite(RAT_OAM, 0, 0);
            move_sprite(RAT_OAM + 1, 0, 0);
            rat_on = 0;
        }
        return;
    }
    if (!rat_on) {                      /* its tiles share the item picture's */
        static uint8_t buf[64];
        far_read(PORTRAIT_BANK, rat_tiles, buf, 64);
        VBK_REG = 1;
        set_sprite_data(RAT_TILE, 4, buf);
        VBK_REG = 0;
        rat_on = 1;
    }
    if (rat_wait) rat_wait--;
    else {
        rat_x += rat_left ? -2 : 2;
        if (rat_x >= 64 || rat_x <= 8) {
            rat_left ^= 1;
            rat_wait = 50 + (DIV_REG & 127);
        }
    }
    sprite16(RAT_OAM, rat_x, rat_wait ? 112 : 112 - ((rat_x >> 1) & 1),
             RAT_TILE, 4 | S_BANK1 | (rat_left ? S_FLIPX : 0));
}

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

/* Per-room tilesets: room_env holds, for each room, a count and the ids of
   the scenery tiles it uses; they go to VRAM bank 1 BG tiles 0.. in order. */
void env_load(uint8_t room) BANKED {
    static uint8_t ids[ENV_SLOTS + 1];
    uint8_t i, j;
    far_read(PORTRAIT_BANK, room_env + (uint16_t)room * (ENV_SLOTS + 1), ids, (uint8_t)(ENV_SLOTS + 1));
    VBK_REG = 1;
    for (i = 0; i < ids[0]; i = j) {        /* runs of consecutive tiles in one copy */
        for (j = i + 1; j < ids[0] && ids[j + 1] == ids[j] + 1; j++) ;
        far_bkg_data(PORTRAIT_BANK, i, j - i, env_tiles + (uint16_t)ids[i + 1] * 16);
    }
    VBK_REG = 0;
}

/* A café table on the pavement of rue Rambuteau, drawn over the poet's
   legs (BG priority) so he seems to sit at it. */
void day_start(void) BANKED {
    static const uint8_t t[3] = { TILE_CHAIR, TILE_TABLE, TILE_TABLE };
    static const uint8_t a[3] = { PAL_CHAIR | 0x88, PAL_TABLE | 0x88, PAL_TABLE | 0x88 };
    day_t = 0;
    if (cur_room != ROOM_STREET) return;
    wait_vbl_done();
    VBK_REG = 1;
    set_bkg_tiles(3, 10, 3, 1, a);
    VBK_REG = 0;
    set_bkg_tiles(3, 10, 3, 1, t);
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
