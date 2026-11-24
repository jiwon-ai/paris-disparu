/* Story logic: where is Fernand? Who says what, the porter's crates, the
   cafe telephone. All state lives in game_flags, clue_flags and the few
   variables below. */
#pragma bank 5   /* EVENTS_BANK: story logic, called through BANKED trampolines */
#include "assets.h"
#include "events.h"
#include "game.h"
#include "items.h"
#include "minigame.h"
#include "world.h"

uint16_t game_flags;
uint8_t clue_flags;
uint8_t new_clue;
uint8_t extra_flags;
uint8_t new_extra;
uint8_t quest_flags;
uint8_t new_quest;
uint16_t secret_flags;
uint8_t new_secret;
uint8_t curio_flags;
uint8_t new_curio;
uint8_t coin_flags;               /* bit i: coin i picked up */
static uint8_t last_look, look_count;
static uint16_t cameo_met;                /* bit k: cameo NPC_ACTOR + k already met */   /* the same thing examined n times in a row */

/* The clue that completes each quest (the last one is the ending). */
static const uint8_t quest_clue[QUEST_COUNT - 1] = { CLUE_COLTIN, CLUE_BLEU, CLUE_RUNGIS, CLUE_PLATE };
static const uint8_t quest_item[QUEST_COUNT - 1] = { ITEM_COLTIN, ITEM_BLEU, NO_ITEM, ITEM_PLATE };

uint8_t quest_done(uint8_t q) BANKED {
    if (q == QUEST_COUNT - 1) return (game_flags & FLAG_SOLVED) != 0;
    return (clue_flags & quest_clue[q]) != 0;
}
uint8_t carrying = NO_CRATE;
static uint8_t crates_taken;     /* 0-3, taken from Fernand's cart */
static uint8_t crates_delivered; /* 0-3 */
uint8_t punched;                 /* passengers' tickets punched for the poinconneur */

/* Save/continue: everything that is progress (not what is on screen). */
void event_pack(uint8_t *b) BANKED {
    b[0] = (uint8_t)game_flags;   b[1] = (uint8_t)(game_flags >> 8);
    b[2] = clue_flags;  b[3] = extra_flags;  b[4] = quest_flags;
    b[5] = (uint8_t)secret_flags; b[6] = (uint8_t)(secret_flags >> 8);
    b[7] = (uint8_t)cameo_met;    b[8] = (uint8_t)(cameo_met >> 8);
    b[9] = carrying;  b[10] = crates_taken;  b[11] = crates_delivered;  b[12] = punched;
    b[13] = curio_flags;
    b[14] = coin_flags;
}

void event_unpack(const uint8_t *b) BANKED {
    game_flags = b[0] | ((uint16_t)b[1] << 8);
    clue_flags = b[2];  extra_flags = b[3];  quest_flags = b[4];
    secret_flags = b[5] | ((uint16_t)b[6] << 8);
    cameo_met = b[7] | ((uint16_t)b[8] << 8);
    carrying = b[9];  crates_taken = b[10];  crates_delivered = b[11];  punched = b[12];
    curio_flags = b[13];
    coin_flags = b[14];
    new_clue = new_extra = new_quest = new_secret = new_curio = 0;
    last_look = look_count = 0;
}


/* Loose change: a 20-centime coin (1967 Marianne, sourced) on the setts by
   the Bourse; walking over it picks it up, and the brasserie waiter sells a
   jeton for it. coin_flags: bit 0 found, bit 1 spent. Room, tile x, tile y. */
static const uint8_t coin_spot[COIN_COUNT][3] = {
    { ROOM_BOURSE, 13, 15 },
};

/* The coin still lying in this room: its index, or 0xFF. */
uint8_t coin_here(void) BANKED {
    uint8_t i;
    for (i = 0; i < COIN_COUNT; i++)
        if (coin_spot[i][0] == cur_room && !(coin_flags & (1 << i))) return i;
    return 0xFF;
}

uint8_t coin_x(uint8_t i) BANKED { return coin_spot[i][1]; }
uint8_t coin_y(uint8_t i) BANKED { return coin_spot[i][2]; }

/* Feet at pixel (px, py): picks the coin up; returns coins found, or 0. */
uint8_t coin_take(int16_t px, int16_t py) BANKED {
    uint8_t i = coin_here();
    if (i == 0xFF || px < 0 || py < 0 || (uint8_t)(px >> 3) != coin_spot[i][1]
        || (uint8_t)(py >> 3) != coin_spot[i][2]) return 0;
    coin_flags |= 1 << i;
    return 1;
}

/* The cart hands out crates in this order; the player must read the stall signs. */
static const uint8_t crate_order[3] = { CRATE_CHOUX, CRATE_POMMES, CRATE_CAROTTES };
static const uint8_t crate_take_dlg[3] = { DLG_CART_CHOUX, DLG_CART_POMMES, DLG_CART_CAROTTES };

#define HAS(f) (game_flags & (f))
#define SET(f) (game_flags |= (f))
#define CLUE(c) (clue_flags & (c))

static void found(uint8_t c) {
    uint8_t q;
    if (!CLUE(c)) {
        clue_flags |= c;
        for (q = 0; q < QUEST_COUNT - 1; q++)
            if (quest_clue[q] == c) { new_quest = q + 1; new_item = quest_item[q]; return; }
        new_clue = 1;
    }
}

static void extra(uint8_t x) {
    if (!(extra_flags & x)) {
        extra_flags |= x;
        if (x & X_ALL) new_extra = 1;
    }
}

uint8_t extra_total(void) BANKED {
    uint8_t n = 0, x;
    for (x = extra_flags & X_ALL; x; x >>= 1) n += x & 1;
    return n;
}

static uint8_t secret(uint8_t n, uint8_t dlg) {
    uint16_t bit = (uint16_t)1 << n;
    if (!(secret_flags & bit)) {
        secret_flags |= bit;
        new_secret = 1;
    }
    return dlg;
}

uint8_t secret_total(void) BANKED {
    uint8_t n = 0;
    uint16_t x;
    for (x = secret_flags; x; x >>= 1) n += x & 1;
    return n;
}

/* Where each curiosity is found, and who can say what it is. */
static const uint8_t curio_spot[CURIO_COUNT] = { DLG_BENCH, DLG_BASKETS, DLG_CHAIRS };
static const uint8_t curio_who[CURIO_COUNT] = { NPC_PUNCHER, NPC_YOUNG_MAN, NPC_PAINTER };

uint8_t curio_total(void) BANKED {
    uint8_t n = 0, c;
    for (c = curio_flags >> 4; c; c >>= 1) n += c & 1;
    return n;
}

/* Examining a curiosity's spot: the first time, the object turns up. */
static uint8_t curio_find(uint8_t id) {
    uint8_t i;
    for (i = 0; i < CURIO_COUNT; i++)
        if (curio_spot[i] == id && !(curio_flags & (1 << i))) {
            curio_flags |= 1 << i;
            new_curio = 1;
            new_item = ITEM_GREEN + i;
            return DLG_CURIO_0_FIND + i;
        }
    return id;
}

/* Talking to someone while carrying the object they know: the answer. */
static uint8_t curio_ask(uint8_t npc) {
    uint8_t i;
    for (i = 0; i < CURIO_COUNT; i++)
        if (curio_who[i] == npc && (curio_flags & (0x11 << i)) == (1 << i)) {
            curio_flags |= 0x10 << i;
            new_curio = 2;
            new_item = ITEM_GREEN + i;
            return DLG_CURIO_0_SOLVE + i;
        }
    return NO_DIALOGUE;
}

uint8_t clue_total(void) BANKED {
    uint8_t n = 0, c;
    for (c = clue_flags; c; c >>= 1) n += c & 1;
    return n;
}

void event_enter_room(uint8_t room) BANKED {
    (void)room;
}

/* Each crate is a little harder to lift than the last. */
uint8_t event_lift_level(void) BANKED {
    return crates_taken;
}

uint8_t event_lift_result(uint8_t success) BANKED {
    if (!success) return DLG_LIFT_MISS;
    carrying = crate_order[crates_taken];
    return crate_take_dlg[crates_taken++];
}

uint8_t event_punch_result(uint8_t success) BANKED {
    if (!success) return DLG_PUNCH_MISS;
    if (++punched < 3) return DLG_PUNCH_OK;
    extra(X_TICKETS);
    return DLG_PUNCH_DONE;
}

uint8_t event_game_result(uint8_t ev, uint8_t success) BANKED {
    switch (ev) {
    case EV_SERVE:
        if (!success) return DLG_SERVE_LOSE;
        found(CLUE_COLTIN);
        return DLG_SERVE_WIN;
    case EV_CUPS:
        if (!success) return DLG_CUPS_LOSE;
        SET(FLAG_HAS_JETON);
        new_item = ITEM_JETON;
        return DLG_CUPS_WIN;
    case EV_CATCH:
        if (!success) return DLG_CATCH_LOSE;
        found(CLUE_RUNGIS);
        return DLG_MERCHANT_RUNGIS;
    case EV_CHASE:              /* the plate is only yours once he drops it */
        if (!success) return DLG_CHASE_LOSE;
        found(CLUE_PLATE);
        return DLG_CHASE_WIN;
    }
    return NO_DIALOGUE;
}

uint8_t event_dial_result(uint16_t number) BANKED {
    game_flags = (game_flags & ~FLAG_HAS_JETON) | FLAG_PHONE_DONE;
    switch (number) {           /* each number with its exchange (ROQ 0614...) */
    case 614:  if (dial_exch != EX_ROQ) break; extra(X_BUTCHER); return DLG_CALL_0614;
    case 4127: if (dial_exch != EX_ARC) break; found(CLUE_COURIER); return DLG_CALL_4127;
    case 7302: if (dial_exch != EX_MED) break; return DLG_CALL_7302;
    case 2508: if (dial_exch != EX_PAS) break; extra(X_WIFE); return DLG_CALL_2508;
    case 8400: if (dial_exch != EX_ODE) break; return DLG_CALL_CLOCK;   /* the speaking clock */
    }
    return DLG_CALL_NONE;
}

uint8_t event_talk(uint8_t npc) BANKED {
    uint8_t d = curio_ask(npc);
    if (d != NO_DIALOGUE) return d;
    switch (npc) {
    case NPC_STRANGER:
        /* He noticed the missing hat and knows where the notebooks are. */
        if (!HAS(FLAG_MET_STRANGER)) {
            SET(FLAG_MET_STRANGER);
            found(CLUE_CASE);
            return DLG_STRANGER_HELLO;
        }
        if ((clue_flags & CLUE_READY) == CLUE_READY) return DLG_STRANGER_KNOWS;
        return DLG_STRANGER_AGAIN;

    case NPC_PORTER:
        if (HAS(FLAG_TASK_DONE)) return DLG_PORTER_AFTER;
        if (HAS(FLAG_TASK_ACTIVE)) {
            if (crates_delivered == 3) {
                game_flags = (game_flags & ~FLAG_TASK_ACTIVE) | FLAG_TASK_DONE;
                return DLG_PORTER_DONE;
            }
            return DLG_PORTER_WORKING;
        }
        if (!HAS(FLAG_MET_PORTER)) { SET(FLAG_MET_PORTER); return DLG_PORTER_HELLO; }
        return DLG_PORTER_ASK;

    case NPC_MERCHANT:
        if (!HAS(FLAG_MET_MERCHANT)) { SET(FLAG_MET_MERCHANT); return DLG_MERCHANT_HELLO; }
        if (HAS(FLAG_TASK_DONE) && !CLUE(CLUE_RUNGIS))   /* the crates earn her trust */
            return DLG_MERCHANT_CATCH;                    /* ... and one more job */
        return DLG_MERCHANT_AGAIN;

    case NPC_CAFE:
        if (!HAS(FLAG_MET_CAFE)) {      /* the rush: help, and she gives his coltin */
            SET(FLAG_MET_CAFE);
            found(CLUE_CASE);
            return DLG_CAFE_HELLO;
        }
        if (!CLUE(CLUE_COLTIN)) return DLG_CAFE_RUSH;
        if (CLUE(CLUE_COURIER) && !CLUE(CLUE_BLEU)) {
            found(CLUE_BLEU);
            return DLG_CAFE_BLEU;
        }
        if ((quest_flags & QF_ROUX) && !(quest_flags & QF_SOUP) && !CLUE(CLUE_PLATE)) {
            quest_flags |= QF_SOUP;     /* a gratinee to take away, for Mme Roux */
            new_item = ITEM_GRATINEE;
            return DLG_CAFE_SOUP;
        }
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON))
            return DLG_CAFE_JETON;      /* out of jetons: the porter has some */
        return CLUE(CLUE_BLEU) ? DLG_CAFE_BLEU_AGAIN : DLG_CAFE_AGAIN;

    case NPC_WORKER:
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON)) return DLG_WORKER_CUPS;
        if (!HAS(FLAG_MET_WORKER)) { SET(FLAG_MET_WORKER); return DLG_WORKER_HELLO; }
        if (HAS(FLAG_PHONE_DONE)) return DLG_WORKER_PHONE;
        return DLG_WORKER_FERNAND;

    case NPC_PAINTER:
        if (!HAS(FLAG_MET_PAINTER)) { SET(FLAG_MET_PAINTER); return DLG_PAINTER_HELLO; }
        if ((extra_flags & X_WIFE) && !(extra_flags & X_PAINTER)) {   /* her message */
            extra(X_PAINTER);
            return DLG_PAINTER_WIFE;
        }
        return DLG_PAINTER_AGAIN;

    case NPC_PHOTO:     /* the cameo: what he says depends on how far you got */
        if ((clue_flags & CLUE_READY) == CLUE_READY) return DLG_PHOTO_KNOWS;
        if (!HAS(FLAG_MET_PHOTO)) { SET(FLAG_MET_PHOTO); return DLG_PHOTO_HELLO; }
        return DLG_PHOTO_AGAIN;

    case NPC_ROUX:      /* the plate, for a hot gratinee */
        if (CLUE(CLUE_PLATE)) return DLG_ROUX_AFTER;
        if (quest_flags & QF_SOUP) {
            quest_flags &= ~QF_SOUP;
            return DLG_ROUX_SOUP;       /* then the thief (event_after) */
        }
        if (quest_flags & QF_ROUX) return DLG_ROUX_WAIT;
        quest_flags |= QF_ROUX;
        return DLG_ROUX_HELLO;

    case NPC_SINGER:    /* the cameo: then a hint at the first missing clue */
        if (!(extra_flags & X_MET_SINGER)) { extra(X_MET_SINGER); return DLG_SINGER_HELLO; }
        if ((clue_flags & CLUE_READY) == CLUE_READY) return DLG_SINGER_READY;
        if (!CLUE(CLUE_COLTIN)) return DLG_SINGER_HAT;
        if (!CLUE(CLUE_BLEU)) return DLG_SINGER_BLEU;
        return DLG_SINGER_PLATE;

    case NPC_ACTOR: case NPC_CHANTEUSE: case NPC_PHILOSOPHER: case NPC_FILMMAKER: case NPC_WRITER:
    case NPC_NOVELIST: case NPC_PLAYWRIGHT: case NPC_CRITIC: case NPC_YOUNG_MAN:
    case NPC_FORT2: case NPC_COOK: case NPC_SLEEPER:
    case NPC_WAITER:            /* the coin from the street buys a jeton */
        if (coin_flags == 1 && !HAS(FLAG_HAS_JETON)) {
            coin_flags = 3;
            SET(FLAG_HAS_JETON);
            new_item = ITEM_JETON;
            return DLG_WAITER_JETON;
        }
        /* fall through */
    case NPC_NIGHTPORTER: case NPC_SOUPER: {   /* first line, then a second */
        uint8_t k = npc - NPC_ACTOR;
        uint16_t bit = 1u << k;
        if (cameo_met & bit) return DLG_ACTOR_HELLO + 2 * k + 1;   /* his or her hint */
        cameo_met |= bit;
        return DLG_ACTOR_HELLO + 2 * k;
    }

    case NPC_PUNCHER:
        if (punched >= 3) return DLG_PUNCHER_AFTER;
        if (HAS(FLAG_TICKET)) return DLG_PUNCHER_TICKET;
        return DLG_PUNCHER_HELLO;

    case NPC_FERNAND:   /* he only opens up to someone who understands */
        if ((clue_flags & CLUE_READY) == CLUE_READY) return DLG_FERNAND_TALK;
        if (!CLUE(CLUE_COLTIN)) return DLG_FERNAND_HELLO;      /* says what is missing */
        return CLUE(CLUE_BLEU) ? DLG_FERNAND_PLATE : DLG_FERNAND_WHY;
    }
    return NO_DIALOGUE;
}

/* What an NPC calls out when the player comes near: a hint of what to do. */
uint8_t event_bark(uint8_t npc) BANKED {
    switch (npc) {
    case NPC_STRANGER:
        return HAS(FLAG_MET_STRANGER) ? DLG_BARK_STRANGER_MET : DLG_BARK_STRANGER;
    case NPC_PORTER:
        if (HAS(FLAG_TASK_DONE)) return DLG_BARK_PORTER_DONE;
        if (!HAS(FLAG_TASK_ACTIVE)) return DLG_BARK_PORTER;
        if (carrying != NO_CRATE) return DLG_BARK_PORTER_CARRY;
        if (crates_delivered == 3) return DLG_BARK_PORTER_BACK;
        return DLG_BARK_PORTER_CART;
    case NPC_MERCHANT:
        return carrying == CRATE_CHOUX ? DLG_BARK_MERCHANT_CHOUX : DLG_BARK_MERCHANT;
    case NPC_CAFE:
        if (HAS(FLAG_MET_CAFE) && !CLUE(CLUE_COLTIN)) return DLG_BARK_CAFE_RUSH;
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON)) return DLG_BARK_CAFE_JETON;
        return HAS(FLAG_MET_CAFE) ? DLG_BARK_CAFE_MET : DLG_BARK_CAFE;
    case NPC_WORKER:
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON)) return DLG_BARK_WORKER_CUPS;
        return DLG_BARK_WORKER;
    case NPC_PAINTER:
        return DLG_BARK_PAINTER;
    case NPC_FERNAND:
        return DLG_BARK_FERNAND;
    case NPC_PHOTO:
        return DLG_BARK_PHOTO;
    case NPC_SINGER:
        return DLG_BARK_SINGER;
    case NPC_ROUX:
        return (quest_flags & QF_SOUP) ? DLG_BARK_ROUX_SOUP : DLG_BARK_ROUX;
    case NPC_PUNCHER:
        return punched >= 3 ? DLG_BARK_PUNCHER_DONE : DLG_BARK_PUNCHER;
    }
    return NO_DIALOGUE;
}

static uint8_t deliver(uint8_t stall, uint8_t ok_dlg, uint8_t wrong_dlg, uint8_t look_dlg) {
    if (carrying == NO_CRATE) return look_dlg;
    if (carrying != stall) return wrong_dlg;
    carrying = NO_CRATE;
    crates_delivered++;
    return ok_dlg;
}

uint8_t event_examine(uint8_t id) BANKED {
    if (id == last_look) look_count++;
    else { last_look = id; look_count = 1; }
    if ((uint8_t)(id - DLG_SECRET_COIN) <= DLG_SECRET_MOUSE - DLG_SECRET_COIN) {
        if (id == DLG_SECRET_CAT && !HAS(FLAG_CAT_GONE)) return NO_DIALOGUE;   /* not in yet */
        return secret(id - DLG_SECRET_COIN, id);
    }
    if (id == DLG_HOTEL && look_count == 3) return secret(SECRET_KNOCK, DLG_SECRET_KNOCK);
    if (id == DLG_LAMP && look_count == 5) return secret(SECRET_MOTH, DLG_SECRET_MOTH);
    switch (id) {
    case DLG_CART_LOOK:
        if (!HAS(FLAG_TASK_ACTIVE)) return crates_taken == 3 ? DLG_CART_EMPTY : DLG_CART_LOOK;
        if (carrying != NO_CRATE) return DLG_CART_FULL;
        if (crates_taken == 3) return DLG_CART_EMPTY;
        return EV_LIFT;

    case DLG_STALL_CHOUX:
        return deliver(CRATE_CHOUX, DLG_STALL_OK_GERMAINE, DLG_STALL_WRONG_GERMAINE, DLG_STALL_CHOUX);
    case DLG_STALL_POMMES:
        return deliver(CRATE_POMMES, DLG_STALL_OK, DLG_STALL_WRONG, DLG_STALL_POMMES);
    case DLG_QUAI_BOX:
        SET(FLAG_KNOW_7302);
        return DLG_QUAI_BOX;
    case DLG_STALL_CAROTTES:
        return deliver(CRATE_CAROTTES, DLG_STALL_OK, DLG_STALL_WRONG, DLG_STALL_CAROTTES);

    case DLG_METRO_WINDOW:
        if (HAS(FLAG_TICKET)) return DLG_METRO_WINDOW_AGAIN;
        SET(FLAG_TICKET);
        new_item = ITEM_TICKET;
        return DLG_METRO_WINDOW;

    case DLG_PHONE_LOOK:
        if (HAS(FLAG_HAS_JETON)) return DLG_PHONE_USE;
        SET(FLAG_SAW_PHONE);
        return DLG_PHONE_LOOK;
    case DLG_HOTEL_SWITCH:      /* the night porter puts you through, for a jeton */
        if (HAS(FLAG_HAS_JETON)) return DLG_HOTEL_SWITCH_USE;
        SET(FLAG_SAW_PHONE);
        return DLG_HOTEL_SWITCH;
    }
    if (id & 0x80) return event_talk(id & 0x7F);
    return curio_find(id);
}

uint8_t event_after(uint8_t dlg, uint8_t choice) BANKED {
    switch (dlg) {
    case DLG_PORTER_HELLO:
    case DLG_PORTER_ASK:
        if (choice == 0) {
            SET(FLAG_TASK_ACTIVE);
            return DLG_PORTER_YES;
        }
        return DLG_PORTER_NO;
    case DLG_FERNAND_TALK:         /* both answers bring him back */
        if (choice == 2) break;    /* closed early: he can be asked again */
        SET(FLAG_SOLVED);
        return choice == 0 ? DLG_END_STAY : DLG_END_GO;
    case DLG_END_STAY:
    case DLG_END_GO:               /* every extra done: one more scene */
        if ((extra_flags & X_ALL) == X_ALL) return DLG_END_BONUS;
        return EV_END;
    case DLG_END_BONUS:
        return EV_END;
    case DLG_PHONE_USE:
    case DLG_HOTEL_SWITCH_USE:
        if (choice == 0) return EV_DIAL;
        break;
    case DLG_CAFE_HELLO:           /* "Give me a hand?" */
    case DLG_CAFE_RUSH:
    case DLG_SERVE_LOSE:
        if (choice == 0) return EV_SERVE;
        break;
    case DLG_WORKER_CUPS:
    case DLG_CUPS_LOSE:
        if (choice == 0) return EV_CUPS;
        break;
    case DLG_MERCHANT_CATCH:
    case DLG_CATCH_LOSE:
        if (choice == 0) return EV_CATCH;
        break;
    case DLG_ROUX_SOUP:            /* dawn: a pickpocket grabs the plate */
        return DLG_THIEF;
    case DLG_THIEF:                /* closed early or not, the chase is on */
    case DLG_CHASE_LOSE:
        return EV_CHASE;
    case DLG_PUNCHER_TICKET:       /* "Want a go?" */
    case DLG_PUNCH_OK:             /* "Next one?" */
    case DLG_PUNCH_MISS:
        if (choice == 0) return EV_PUNCH;
        break;
    }
    return NO_DIALOGUE;
}
