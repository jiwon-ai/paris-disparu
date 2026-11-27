#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include <gb/gb.h>      /* BANKED: events.c lives in ROM bank EVENTS_BANK */

#define NO_CRATE 0xFF
#define CRATE_CHOUX    0
#define CRATE_POMMES   1
#define CRATE_CAROTTES 2

extern uint8_t carrying;   /* crate type held by the player, or NO_CRATE */

/* Special results: start an interactive sequence instead of text. */
#define EV_LIFT 0xFE   /* lift a crate from the cart (timing) */
#define EV_DIAL 0xFD   /* dial a number on the cafe telephone */
#define EV_END  0xFC   /* the mystery is solved: end screen */
#define EV_PUNCH 0xFB  /* punch a passenger's ticket in the metro (timing) */
#define EV_SERVE 0xFA  /* the cafe coffee rush */
#define EV_CUPS  0xF9  /* the porter's cup game, for a jeton */
#define EV_CATCH 0xF8  /* catch Germaine's cabbages */
#define EV_CHASE 0xF7  /* chase the thief on the quai */
#define EV_FIRST EV_CHASE   /* lowest EV_*: dialogue ids stay below it */

/* Each returns the dialogue to show, EV_*, or NO_DIALOGUE. */
#define EVENT_SAVE_SIZE 18
#define COIN_COUNT 1
extern uint8_t coin_flags;
extern uint8_t frag_flags[3];
extern uint8_t new_frag;
#define FRAG_COUNT 24
#define POEM_WORDS 12     /* Fernand listens once the poem has this many words */
#define POEM_SECRETS 4    /* ... and this many secrets (each gives a phrase) */
uint8_t poem_ready(void) BANKED;
uint8_t frag_has(uint8_t i) BANKED;      /* word i of the poem found */
uint8_t frag_total(void) BANKED;
void frag_seen(uint8_t d) BANKED;        /* dialogue d opens: its word, if any */
uint8_t coin_here(void) BANKED;
void coin_glint(uint8_t on) BANKED;              /* draw or hide its glint (OAM 30-31) */                 /* coin lying in this room, or 0xFF */
uint8_t coin_x(uint8_t i) BANKED;               /* its tile */
uint8_t coin_y(uint8_t i) BANKED;
uint8_t coin_take(int16_t px, int16_t py) BANKED;   /* feet there: 1 when picked up */
void event_pack(uint8_t *b) BANKED;          /* story state -> EVENT_SAVE_SIZE bytes */
void event_unpack(const uint8_t *b) BANKED;
uint8_t event_talk(uint8_t npc) BANKED;
uint8_t event_bark(uint8_t npc) BANKED;   /* line called out when the player is near */
uint8_t event_examine(uint8_t id) BANKED;
uint8_t event_after(uint8_t dlg, uint8_t choice) BANKED;  /* follow-up after a dialogue closes */
void event_enter_room(uint8_t room) BANKED;
uint8_t event_lift_level(void) BANKED;
uint8_t event_lift_result(uint8_t success) BANKED;
uint8_t event_punch_result(uint8_t success) BANKED;
uint8_t event_game_result(uint8_t ev, uint8_t success) BANKED;   /* EV_SERVE/CUPS/CATCH */
uint8_t event_dial_result(uint16_t number) BANKED;

#endif
