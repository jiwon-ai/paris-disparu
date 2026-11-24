/* Episode 2 story logic: the last night of Les Halles (27-28 Feb 1969).
   Fernand's plate is gone; the poet follows a boy in a red scarf along
   métro line 4. Three things to learn in three districts (any order), then
   the boy under the bridge, then dawn. All state is in the variables below. */
#pragma bank 5   /* EVENTS_BANK */
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
uint8_t album_flags;
uint8_t new_album;
uint8_t money;
uint8_t new_money;
uint8_t coin_flags;
uint16_t cameo_met;
uint8_t carrying = NO_CRATE;
uint8_t metro_room, metro_x, metro_y;
uint8_t encounter;
static uint8_t cups_paid;        /* francs Dédé has given (he has 4) */
static uint8_t tramp_seen, rooms_since, enc_seed;
static uint8_t last_look, look_count;

static const uint8_t quest_clue[QUEST_COUNT - 1] = { K_TRAIL, K_NAME, K_HIDE, K_HEART, K_PLATE };

uint8_t quest_done(uint8_t q) BANKED {
    if (q == QUEST_COUNT - 1) return (game_flags & F_SOLVED) != 0;
    return (clue_flags & quest_clue[q]) != 0;
}

void event_pack(uint8_t *b) BANKED {
    b[0] = (uint8_t)game_flags;   b[1] = (uint8_t)(game_flags >> 8);
    b[2] = clue_flags;  b[3] = extra_flags;  b[4] = quest_flags;
    b[5] = (uint8_t)secret_flags; b[6] = (uint8_t)(secret_flags >> 8);
    b[7] = (uint8_t)cameo_met;    b[8] = (uint8_t)(cameo_met >> 8);
    b[9] = album_flags;  b[10] = money;  b[11] = coin_flags;  b[12] = cups_paid;
}

void event_unpack(const uint8_t *b) BANKED {
    game_flags = b[0] | ((uint16_t)b[1] << 8);
    clue_flags = b[2];  extra_flags = b[3];  quest_flags = b[4];
    secret_flags = b[5] | ((uint16_t)b[6] << 8);
    cameo_met = b[7] | ((uint16_t)b[8] << 8);
    album_flags = b[9];  money = b[10];  coin_flags = b[11];  cups_paid = b[12];
    new_clue = new_extra = new_quest = new_secret = new_album = new_money = 0;
    last_look = look_count = 0;
    encounter = 0;
}

/* Francs lying about: room, tile x, tile y. Walking over one picks it up. */
static const uint8_t coin_spot[COIN_COUNT][3] = {
    { ROOM_EUSTACHE, 3, 15 },
    { ROOM_FLEURS, 17, 16 },
    { ROOM_DAUPHINE, 2, 16 },
    { ROOM_HUCHETTE, 9, 16 },
    { ROOM_TERRASSE, 15, 16 },
};

uint8_t coin_here(void) BANKED {
    uint8_t i;
    for (i = 0; i < COIN_COUNT; i++)
        if (coin_spot[i][0] == cur_room && !(coin_flags & (1 << i))) return i;
    return 0xFF;
}
uint8_t coin_x(uint8_t i) BANKED { return coin_spot[i][1]; }
uint8_t coin_y(uint8_t i) BANKED { return coin_spot[i][2]; }

static void earn(void) {
    if (money < MONEY_MAX) money++;
    new_money = 1;
}

uint8_t coin_take(int16_t px, int16_t py) BANKED {
    uint8_t i = coin_here();
    if (i == 0xFF || px < 0 || py < 0 || (uint8_t)(px >> 3) != coin_spot[i][1]
        || (uint8_t)(py >> 3) != coin_spot[i][2]) return 0;
    coin_flags |= 1 << i;
    earn();
    new_money = 0;               /* main shows its own note */
    return 1;
}

#define HAS(f) (game_flags & (f))
#define SET(f) (game_flags |= (f))
#define KNOW(c) (clue_flags & (c))

static void learn(uint8_t c) {
    uint8_t q;
    if (KNOW(c)) return;
    clue_flags |= c;
    for (q = 0; q < QUEST_COUNT - 1; q++)
        if (quest_clue[q] == c) { new_quest = q + 1; return; }
    new_clue = 1;
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

static void album(uint8_t a, uint8_t item) {
    if (!(album_flags & a)) {
        album_flags |= a;
        new_album = 1;
    }
    new_item = item;
}

uint8_t album_total(void) BANKED {
    uint8_t n = 0, c;
    for (c = album_flags; c; c >>= 1) n += c & 1;
    return n;
}

uint8_t clue_total(void) BANKED {
    uint8_t n = 0, c;
    for (c = clue_flags; c; c >>= 1) n += c & 1;
    return n;
}

static uint8_t pay(void) {       /* 1 when a franc was spent */
    if (!money) return 0;
    money--;
    return 1;
}

/* Streets where someone may come up to you (not indoors, not the start). */
static uint8_t outdoor(uint8_t room) {
    return room != ROOM_CAFE && room != ROOM_METRO && room != ROOM_CAVEAU && room != ROOM_BASPORT;
}

uint8_t event_enter_room(uint8_t room) BANKED {
    encounter = 0;
    if (!KNOW(K_TRAIL) || HAS(F_SOLVED) || !outdoor(room)) return NO_DIALOGUE;
    if (++rooms_since < 3) return NO_DIALOGUE;
    enc_seed = (uint8_t)(enc_seed * 5 + 3) ^ DIV_REG;
    if ((enc_seed & 3) != 0) return NO_DIALOGUE;       /* about one street in four */
    rooms_since = 0;
    if (money && (!(extra_flags & X_THIEF) || (enc_seed & 0x10))) encounter = 2;   /* a pickpocket */
    else if (tramp_seen < 3) { tramp_seen++; encounter = 1; }
    return NO_DIALOGUE;
}

uint8_t event_metro(uint8_t stop) BANKED {
    static const uint8_t arrive[STOP_COUNT][3] = {
        { ROOM_METRO, 72, 100 },          /* Les Halles ticket hall */
        { ROOM_FLEURS, 72, 104 },         /* Cité, by the flower market */
        { ROOM_STMICHEL, 104, 108 },
        { ROOM_STGERMAIN, 40, 108 },
    };
    game_flags &= ~F_TICKET;              /* the ticket is punched and gone */
    metro_room = arrive[stop][0];
    metro_x = arrive[stop][1];
    metro_y = arrive[stop][2];
    return NO_DIALOGUE;
}

uint8_t event_game_result(uint8_t ev, uint8_t success) BANKED {
    switch (ev) {
    case EV_CUPS:
        if (!success) return DLG_CUPS_LOSE;
        cups_paid++;
        earn();
        return DLG_CUPS_WIN;
    case EV_PETANQUE:
        if (!success) return DLG_PETANQUE_LOSE;
        learn(K_NAME);
        return DLG_PETANQUE_WIN;
    case EV_JAZZ:
        if (!success) return DLG_JAZZ_LOSE;
        learn(K_HIDE);
        SET(F_MATCHES);
        return DLG_JAZZ_WIN;
    case EV_FIND:                 /* found him: he talks, or not */
        return (clue_flags & K_READY) == K_READY ? DLG_LULU_TALK : DLG_LULU_PARTIAL;
    case EV_CHASE:
        if (!success) return DLG_CHASE_LOSE;
        earn();                   /* your franc back */
        earn();                   /* and one of his */
        extra(X_THIEF);
        return DLG_CHASE_WIN;
    case EV_TOSS:
        return EV_END;
    }
    return NO_DIALOGUE;
}

uint8_t event_dial_result(uint16_t number) BANKED {
    game_flags &= ~F_JETON;
    switch (number) {
    case 8400: if (dial_exch != EX_ODE) break; return DLG_CALL_CLOCK;
    case 4127: if (dial_exch != EX_ARC) break; return DLG_CALL_NOTES;      /* Episode 1's diarist */
    case 2271: if (dial_exch != EX_ROQ) break; extra(X_CALLED); return DLG_CALL_MOTHER;
    case 3301: if (dial_exch != EX_MED) break; return DLG_CALL_DRUMMER;
    case 6120: if (dial_exch != EX_ODE) break; return DLG_CALL_EDITOR;
    }
    return DLG_CALL_NONE;
}

/* Someone talked to for the first time: hello, then again. */
static uint8_t once(uint8_t npc, uint8_t hello, uint8_t again) {
    uint16_t bit = 1u << npc;
    if (cameo_met & bit) return again;
    cameo_met |= bit;
    return hello;
}

uint8_t event_talk(uint8_t npc) BANKED {
    switch (npc) {
    case NPC_FERNAND:
        if (HAS(F_SOLVED)) return DLG_FERNAND_BACK;
        if (KNOW(K_PLATE)) {                 /* the plate goes back: dawn */
            SET(F_SOLVED);
            album(AL_PLATE, ITEM_PLATE);
            return DLG_FERNAND_BACK;
        }
        if (!HAS(F_MET_FERNAND)) { SET(F_MET_FERNAND); return DLG_FERNAND_HELLO; }
        return KNOW(K_TRAIL) ? DLG_FERNAND_TRAIL : DLG_FERNAND_WAIT;

    case NPC_ROUX:
        if (!KNOW(K_TRAIL)) {
            learn(K_TRAIL);
            SET(F_TICKET);
            album(AL_TICKET, ITEM_TICKET);
            return DLG_ROUX_HELLO;
        }
        return DLG_ROUX_AGAIN;

    case NPC_PORTER:
        if (cups_paid >= 4) return DLG_CUPS_BROKE;
        return once(NPC_PORTER, DLG_PORTER_HELLO, DLG_PORTER_AGAIN);
    case NPC_MUSICIAN:
        return once(NPC_MUSICIAN, DLG_MUSICIAN_HELLO, DLG_MUSICIAN_AGAIN);
    case NPC_DANCER:
        return DLG_DANCER_HELLO;
    case NPC_CAFE:
        return once(NPC_CAFE, DLG_CAFE_HELLO, DLG_CAFE_AGAIN);

    case NPC_FLOWER:
        if (HAS(F_VIOLET) || (extra_flags & X_VIOLET)) return DLG_FLOWER_SELLER_BUY;
        return DLG_FLOWER_SELLER_HELLO;
    case NPC_PETANQUE:
        if (KNOW(K_NAME)) return DLG_PETANQUE_AFTER;
        return once(NPC_PETANQUE, DLG_PETANQUE_HELLO, DLG_PETANQUE_AGAIN);
    case NPC_PETANQUE2:
        return DLG_PETANQUE_OTHER;
    case NPC_CLOCHARDE:
        if (HAS(F_VIOLET)) {                  /* the violet makes her smile */
            game_flags &= ~F_VIOLET;
            extra(X_VIOLET);
            return DLG_CLOCHARDE_VIOLET;
        }
        if (!HAS(F_NUMBER)) { SET(F_NUMBER); new_item = ITEM_NOTE; return DLG_CLOCHARDE_HELLO; }
        return DLG_CLOCHARDE_AGAIN;

    case NPC_STUDENT:
        return once(NPC_STUDENT, DLG_STUDENT_HELLO, DLG_STUDENT_AGAIN);
    case NPC_DOORMAN:
        if (HAS(F_INSIDE)) return DLG_DOORMAN_AFTER;
        if (HAS(F_MUTE)) { SET(F_INSIDE); return DLG_DOORMAN_MUTE; }
        return DLG_DOORMAN_HELLO;
    case NPC_TRUMPET:
        if (!HAS(F_INSIDE)) return DLG_TRUMPET_NOMUTE;
        if (KNOW(K_HIDE)) return DLG_TRUMPET_AFTER;
        return once(NPC_TRUMPET, DLG_TRUMPET_HELLO, DLG_TRUMPET_AGAIN);

    case NPC_OLDPOET:
        if (KNOW(K_HEART)) return DLG_OLD_POET_AGAIN;
        return DLG_OLD_POET_HELLO;
    case NPC_WAITER:
        return DLG_WAITER2_HELLO;
    case NPC_PHILOSOPHER:
        return DLG_PHILOSOPHER2_HELLO;

    case NPC_LULU:
        if (KNOW(K_PLATE)) {
            if ((extra_flags & X_CALLED) && !(extra_flags & X_MOTHER)) {
                extra(X_MOTHER);
                SET(F_LULU_HOME);
                return DLG_LULU_MOTHER;
            }
            return DLG_LULU_AFTER;
        }
        if (!KNOW(K_HIDE)) return DLG_LULU_HELLO;
        return EV_FIND;
    case NPC_SLEEPER:
        return DLG_LULU_HIDDEN;
    }
    return NO_DIALOGUE;
}

uint8_t event_bark(uint8_t npc) BANKED {
    switch (npc) {
    case NPC_FERNAND:
        if (HAS(F_SOLVED) || KNOW(K_PLATE)) return NO_DIALOGUE;
        return KNOW(K_TRAIL) ? DLG_BARK_FERNAND_TRAIL : DLG_BARK_FERNAND;
    case NPC_ROUX:      return KNOW(K_TRAIL) ? NO_DIALOGUE : DLG_BARK_ROUX;
    case NPC_MUSICIAN:  return DLG_BARK_MUSICIAN;
    case NPC_PETANQUE:  return KNOW(K_NAME) ? NO_DIALOGUE : DLG_BARK_PETANQUE;
    case NPC_CLOCHARDE: return DLG_BARK_CLOCHARDE;
    case NPC_STUDENT:   return DLG_BARK_STUDENT;
    case NPC_DOORMAN:   return HAS(F_INSIDE) ? NO_DIALOGUE : DLG_BARK_DOORMAN;
    case NPC_OLDPOET:   return KNOW(K_HEART) ? NO_DIALOGUE : DLG_BARK_OLD_POET;
    case NPC_LULU:      return KNOW(K_PLATE) ? NO_DIALOGUE : DLG_BARK_LULU;
    }
    return NO_DIALOGUE;
}

/* A métro entrance: with a ticket, the map; without, buy one (or ride free
   once if broke, so nobody is ever stuck on an island). */
static uint8_t metro_entrance(void) {
    if (!KNOW(K_TRAIL)) return DLG_METRO_SIGN;
    if (HAS(F_TICKET)) return EV_METRO;
    return DLG_METRO_WINDOW;
}

uint8_t event_examine(uint8_t id) BANKED {
    if (id == last_look) look_count++;
    else { last_look = id; look_count = 1; }
    if ((uint8_t)(id - DLG_SECRET_RIBBON) < SECRET_COUNT - 2 || id == DLG_SECRET_INITIALS) {
        uint8_t n = id == DLG_SECRET_INITIALS ? SECRET_INITIALS : id - DLG_SECRET_RIBBON;
        if (id == DLG_SECRET_SHARD) { SET(F_SHARD); album(AL_SHARD, ITEM_SHARD); }
        return secret(n, id);
    }
    if (id == DLG_SG_CHURCH && look_count == 3) return secret(SECRET_BELLS, DLG_SECRET_BELLS);
    switch (id) {
    case DLG_FLOWERBED:
        if (HAS(F_AZALEA)) return DLG_FLOWERBED;
        SET(F_AZALEA);
        album(AL_AZALEA, ITEM_AZALEA);
        return DLG_FLOWERBED_TAKE;
    case DLG_SM_PAVE:
        if (HAS(F_PAVE) || look_count < 2) return DLG_SM_PAVE;
        SET(F_PAVE);
        album(AL_PAVE, ITEM_PAVE);
        return DLG_SM_PAVE_TAKE;
    case DLG_FOUNTAIN:
        if (HAS(F_MUTE)) return DLG_FOUNTAIN;
        SET(F_MUTE);
        new_item = ITEM_MUTE;
        return DLG_FOUNTAIN_MUTE;
    case DLG_BOOKSHOP:
        if (HAS(F_CARD)) return DLG_BOOKSHOP;
        SET(F_CARD);
        return DLG_BOOKSHOP_CARD;
    case DLG_PHONE_LOOK:
        SET(F_PHONE_SEEN);
        return HAS(F_JETON) ? DLG_PHONE_USE : DLG_PHONE_LOOK;
    case DLG_METRO_SIGN:
    case DLG_METRO_CITE:
    case DLG_METRO_SM:
    case DLG_METRO_SG:
        if (!KNOW(K_TRAIL)) return id;
        return metro_entrance();
    case DLG_METRO_PLATFORM:
        return metro_entrance();
    case DLG_LULU_HIDDEN:
        return event_talk(NPC_LULU);
    }
    if (id & 0x80) return event_talk(id & 0x7F);
    return id;
}

uint8_t event_after(uint8_t dlg, uint8_t choice) BANKED {
    switch (dlg) {
    case DLG_FERNAND_BACK:
        if (HAS(F_SOLVED)) return EV_TOSS;
        break;
    case DLG_PORTER_HELLO:
    case DLG_PORTER_AGAIN:
    case DLG_CUPS_LOSE:
        if (choice == 0) return EV_CUPS;
        if (dlg != DLG_CUPS_LOSE) return DLG_PORTER_NO;
        break;
    case DLG_CAFE_AGAIN:
        if (choice) break;
        if (HAS(F_JETON)) return DLG_CAFE_JETON;
        if (!pay()) return DLG_CAFE_NOMONEY;
        SET(F_JETON);
        album(AL_JETON, ITEM_JETON);
        return DLG_CAFE_JETON;
    case DLG_PHONE_USE:
        if (choice == 0) return EV_DIAL;
        break;
    case DLG_METRO_WINDOW:
        if (choice) break;
        if (!pay()) {
            if (money == 0) { SET(F_TICKET); return DLG_METRO_WINDOW_FREE; }
            return DLG_METRO_WINDOW_POOR;
        }
        SET(F_TICKET);
        new_item = ITEM_TICKET;
        return DLG_METRO_WINDOW_OK;
    case DLG_METRO_WINDOW_OK:
    case DLG_METRO_WINDOW_FREE:
        return EV_METRO;
    case DLG_FLOWER_SELLER_HELLO:
        if (choice) return DLG_FLOWER_SELLER_NO;
        if (!pay()) return DLG_FLOWER_SELLER_POOR;
        SET(F_VIOLET);
        new_item = ITEM_VIOLET;
        return DLG_FLOWER_SELLER_BUY;
    case DLG_PETANQUE_HELLO:
    case DLG_PETANQUE_AGAIN:
    case DLG_PETANQUE_LOSE:
        if (choice == 0) return EV_PETANQUE;
        break;
    case DLG_TRUMPET_HELLO:
    case DLG_TRUMPET_AGAIN:
    case DLG_JAZZ_LOSE:
        if (choice == 0) return EV_JAZZ;
        break;
    case DLG_OLD_POET_HELLO:
        extra_flags |= choice ? X_POET_YOUNG : 0;
        return DLG_OLD_POET_TWO;
    case DLG_OLD_POET_TWO:
        extra_flags |= choice ? 0 : X_POET_SING;
        return DLG_OLD_POET_DONE;
    case DLG_OLD_POET_DONE:
        learn(K_HEART);
        break;
    case DLG_LULU_TALK:
        learn(K_PLATE);
        new_item = ITEM_PLATE;
        break;
    case DLG_TRAMP_HELLO:
        if (choice) return DLG_TRAMP_NO;
        if (!pay()) return DLG_TRAMP_POOR;
        extra(X_TRAMP);
        return DLG_TRAMP_GIVE;
    case DLG_PICKPOCKET:
    case DLG_CHASE_LOSE:
        if (dlg == DLG_PICKPOCKET) return EV_CHASE;
        break;
    }
    return NO_DIALOGUE;
}
