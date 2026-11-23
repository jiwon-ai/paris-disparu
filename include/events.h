#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

#define NO_CRATE 0xFF
#define CRATE_CHOUX    0
#define CRATE_POMMES   1
#define CRATE_CAROTTES 2

extern uint8_t carrying;   /* crate type held by the player, or NO_CRATE */

/* Special results: start an interactive sequence instead of text. */
#define EV_LIFT 0xFE   /* lift a crate from the cart (timing) */
#define EV_DIAL 0xFD   /* dial a number on the cafe telephone */
#define EV_END  0xFC   /* the mystery is solved: end screen */

/* Each returns the dialogue to show, EV_*, or NO_DIALOGUE. */
uint8_t event_talk(uint8_t npc);
uint8_t event_bark(uint8_t npc);   /* line called out when the player is near */
uint8_t event_examine(uint8_t id);
uint8_t event_after(uint8_t dlg, uint8_t choice);  /* follow-up after a dialogue closes */
void event_enter_room(uint8_t room);
uint8_t event_lift_level(void);
uint8_t event_lift_result(uint8_t success);
uint8_t event_dial_result(uint16_t number);

#endif
