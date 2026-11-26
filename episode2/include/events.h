#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include <gb/gb.h>      /* BANKED: events.c lives in ROM bank EVENTS_BANK */

#define NO_CRATE 0xFF
extern uint8_t carrying;   /* kept for actor.c (half speed); always NO_CRATE */

/* Special results: start an interactive sequence instead of text. */
#define EV_DIAL     0xFE   /* dial a number */
#define EV_END      0xFD   /* dawn, the poem, the end screen */
#define EV_METRO    0xFC   /* the line-4 map: choose a stop */
#define EV_CUPS     0xFB   /* Dédé's cup game, for a franc */
#define EV_CHASE    0xFA   /* catch the pickpocket */
#define EV_PETANQUE 0xF9   /* three boules at Place Dauphine */
#define EV_JAZZ     0xF8   /* keep the beat in the jazz cellar */
#define EV_FIND     0xF7   /* find the red scarf among the sleepers */
#define EV_TOSS     0xF6   /* throw flowers at dawn */
#define EV_FIRST    EV_TOSS   /* lowest EV_*: dialogue ids stay below it */

#define EVENT_SAVE_SIZE 13
#define COIN_COUNT 5
extern uint8_t coin_flags;
uint8_t coin_here(void) BANKED;
uint8_t coin_x(uint8_t i) BANKED;
uint8_t coin_y(uint8_t i) BANKED;
uint8_t coin_take(int16_t px, int16_t py) BANKED;
void event_pack(uint8_t *b) BANKED;
void event_unpack(const uint8_t *b) BANKED;
uint8_t event_talk(uint8_t npc) BANKED;
uint8_t event_bark(uint8_t npc) BANKED;
uint8_t event_examine(uint8_t id) BANKED;
uint8_t event_after(uint8_t dlg, uint8_t choice) BANKED;
uint8_t event_enter_room(uint8_t room) BANKED;   /* a random encounter: EV/dialogue or NO_DIALOGUE */
uint8_t event_game_result(uint8_t ev, uint8_t success) BANKED;
uint8_t event_dial_result(uint16_t number) BANKED;
uint8_t event_metro(uint8_t stop) BANKED;        /* ride to a stop: sets the arrival */
extern uint8_t metro_room, metro_x, metro_y;     /* where the ride arrives */
extern uint8_t encounter;                        /* 0 none, 1 tramp, 2 pickpocket (walking up) */

#endif
