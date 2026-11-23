/* Story logic: the apple mystery, who says what, the porter's crates, the
   cafe telephone. All state lives in game_flags, clue_flags and the few
   variables below. */
#include "assets.h"
#include "events.h"
#include "game.h"
#include "world.h"

uint16_t game_flags;
uint8_t clue_flags;
uint8_t new_clue;
uint8_t carrying = NO_CRATE;
static uint8_t crates_taken;     /* 0-3, taken from Fernand's cart */
static uint8_t crates_delivered; /* 0-3 */

/* The cart hands out crates in this order; the player must read the stall signs. */
static const uint8_t crate_order[3] = { CRATE_CHOUX, CRATE_POMMES, CRATE_CAROTTES };
static const uint8_t crate_take_dlg[3] = { DLG_CART_CHOUX, DLG_CART_POMMES, DLG_CART_CAROTTES };

#define HAS(f) (game_flags & (f))
#define SET(f) (game_flags |= (f))
#define CLUE(c) (clue_flags & (c))

static void found(uint8_t c) {
    if (!CLUE(c)) {
        clue_flags |= c;
        new_clue = 1;
    }
}

uint8_t clue_total(void) {
    uint8_t n = 0, c;
    for (c = clue_flags; c; c >>= 1) n += c & 1;
    return n;
}

void event_enter_room(uint8_t room) {
    (void)room;
}

/* Each crate is a little harder to lift than the last. */
uint8_t event_lift_level(void) {
    return crates_taken;
}

uint8_t event_lift_result(uint8_t success) {
    if (!success) return DLG_LIFT_MISS;
    carrying = crate_order[crates_taken];
    return crate_take_dlg[crates_taken++];
}

uint8_t event_dial_result(uint16_t number) {
    game_flags = (game_flags & ~FLAG_HAS_JETON) | FLAG_PHONE_DONE;
    switch (number) {
    case 614:  return DLG_CALL_0614;
    case 4127: found(CLUE_HAT);  return DLG_CALL_4127;
    case 7302: found(CLUE_BOOK); return DLG_CALL_7302;
    case 2508: return DLG_CALL_2508;
    }
    return DLG_CALL_NONE;
}

uint8_t event_talk(uint8_t npc) {
    switch (npc) {
    case NPC_STRANGER:
        /* He sets up the mystery and knows where the notebooks are. */
        if (!HAS(FLAG_MET_STRANGER)) { SET(FLAG_MET_STRANGER); return DLG_STRANGER_HELLO; }
        if (CLUE(CLUE_RUN)) return DLG_STRANGER_KNOWS;
        return DLG_STRANGER_AGAIN;

    case NPC_PORTER:
        if (CLUE(CLUE_RUN)) return DLG_PORTER_CONFRONT;   /* the reveal */
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
        if (CLUE(CLUE_CASE) && !CLUE(CLUE_STALL)) return DLG_MERCHANT_APPLE;
        return DLG_MERCHANT_AGAIN;

    case NPC_CAFE:
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON)) {
            /* A token each time you ask: calls are the fun part. */
            SET(FLAG_HAS_JETON);
            return HAS(FLAG_PHONE_DONE) ? DLG_CAFE_JETON_MORE : DLG_CAFE_JETON;
        }
        if (HAS(FLAG_TASK_DONE) && HAS(FLAG_MET_CAFE) && !HAS(FLAG_FERNAND_TOLD)) {
            SET(FLAG_FERNAND_TOLD);
            return DLG_CAFE_FERNAND;
        }
        if (!HAS(FLAG_MET_CAFE)) {
            SET(FLAG_MET_CAFE);
            found(CLUE_CASE);
            return DLG_CAFE_HELLO;
        }
        return DLG_CAFE_AGAIN;

    case NPC_WORKER:
        if (CLUE(CLUE_HAT) && !CLUE(CLUE_RUN)) {
            found(CLUE_RUN);
            return DLG_WORKER_RUN;
        }
        if (HAS(FLAG_PHONE_DONE)) return DLG_WORKER_PHONE;
        return DLG_WORKER_HELLO;

    case NPC_PAINTER:
        if (!HAS(FLAG_MET_PAINTER)) {
            SET(FLAG_MET_PAINTER);
            found(CLUE_RIVER);
            return DLG_PAINTER_HELLO;
        }
        return DLG_PAINTER_AGAIN;
    }
    return NO_DIALOGUE;
}

/* What an NPC calls out when the player comes near: a hint of what to do. */
uint8_t event_bark(uint8_t npc) {
    switch (npc) {
    case NPC_STRANGER:
        return HAS(FLAG_MET_STRANGER) ? DLG_BARK_STRANGER_MET : DLG_BARK_STRANGER;
    case NPC_PORTER:
        if (CLUE(CLUE_RUN)) return DLG_BARK_PORTER_STARE;
        if (HAS(FLAG_TASK_DONE)) return DLG_BARK_PORTER_DONE;
        if (!HAS(FLAG_TASK_ACTIVE)) return DLG_BARK_PORTER;
        if (carrying != NO_CRATE) return DLG_BARK_PORTER_CARRY;
        if (crates_delivered == 3) return DLG_BARK_PORTER_BACK;
        return DLG_BARK_PORTER_CART;
    case NPC_MERCHANT:
        return carrying == CRATE_CHOUX ? DLG_BARK_MERCHANT_CHOUX : DLG_BARK_MERCHANT;
    case NPC_CAFE:
        if (HAS(FLAG_SAW_PHONE) && !HAS(FLAG_HAS_JETON)) return DLG_BARK_CAFE_JETON;
        return HAS(FLAG_MET_CAFE) ? DLG_BARK_CAFE_MET : DLG_BARK_CAFE;
    case NPC_WORKER:
        return (CLUE(CLUE_HAT) && !CLUE(CLUE_RUN)) ? DLG_BARK_WORKER_HAT : DLG_BARK_WORKER;
    case NPC_PAINTER:
        return DLG_BARK_PAINTER;
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

uint8_t event_examine(uint8_t id) {
    switch (id) {
    case DLG_CART_LOOK:
        if (!HAS(FLAG_TASK_ACTIVE)) return crates_taken == 3 ? DLG_CART_EMPTY : DLG_CART_LOOK;
        if (carrying != NO_CRATE) return DLG_CART_FULL;
        if (crates_taken == 3) return DLG_CART_EMPTY;
        return EV_LIFT;

    case DLG_STALL_CHOUX:
        return deliver(CRATE_CHOUX, DLG_STALL_OK_GERMAINE, DLG_STALL_WRONG_GERMAINE, DLG_STALL_CHOUX);
    case DLG_STALL_POMMES: {
        uint8_t d = deliver(CRATE_POMMES, DLG_STALL_OK_POMMES, DLG_STALL_WRONG, DLG_STALL_POMMES);
        if (d == DLG_STALL_OK_POMMES) found(CLUE_STALL);
        return d;
    }
    case DLG_QUAI_BOX:
        SET(FLAG_KNOW_7302);
        return DLG_QUAI_BOX;
    case DLG_STALL_CAROTTES:
        return deliver(CRATE_CAROTTES, DLG_STALL_OK, DLG_STALL_WRONG, DLG_STALL_CAROTTES);

    case DLG_PHONE_LOOK:
        if (HAS(FLAG_HAS_JETON)) return DLG_PHONE_USE;
        SET(FLAG_SAW_PHONE);
        return DLG_PHONE_LOOK;
    }
    if (id & 0x80) return event_talk(id & 0x7F);
    return id;
}

uint8_t event_after(uint8_t dlg, uint8_t choice) {
    switch (dlg) {
    case DLG_PORTER_HELLO:
    case DLG_PORTER_ASK:
        if (choice == 0) {
            SET(FLAG_TASK_ACTIVE);
            return DLG_PORTER_YES;
        }
        return DLG_PORTER_NO;
    case DLG_PORTER_CONFRONT:      /* both answers lead to an ending */
        if (choice == 2) break;    /* closed early: he can be asked again */
        SET(FLAG_SOLVED);
        return choice == 0 ? DLG_END_YES : DLG_END_NO;
    case DLG_END_YES:
    case DLG_END_NO:
        return EV_END;
    case DLG_PHONE_USE:
        if (choice == 0) return EV_DIAL;
        break;
    }
    return NO_DIALOGUE;
}
