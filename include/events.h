#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

#define NO_CRATE 0xFF
#define CRATE_CHOUX    0
#define CRATE_POMMES   1
#define CRATE_CAROTTES 2

extern uint8_t carrying;   /* crate type held by the player, or NO_CRATE */

/* Each returns the dialogue to show, or NO_DIALOGUE. */
uint8_t event_talk(uint8_t npc);
uint8_t event_examine(uint8_t id);
uint8_t event_after(uint8_t dlg, uint8_t choice);  /* follow-up after a dialogue closes */
void event_enter_room(uint8_t room);

#endif
