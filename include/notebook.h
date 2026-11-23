#ifndef NOTEBOOK_H
#define NOTEBOOK_H

/* Full-screen pages on the window layer. */
void notebook_show(void);   /* START: goal, clues found, phone numbers known */
void end_show(void);        /* the mystery is solved */
void goal_print(uint8_t y); /* current goal, two rows, in the open box */
void intro_show(uint8_t page);
void extras_show(void);
uint16_t notebook_number(void);  /* the next phone number known, cycling (0 if none) */
uint8_t goal_npc(void);          /* NPC_* to talk to next, or 0xFF */
void howto_show(uint8_t ev);     /* rules card before a mini-game (EV_*) */
void teaser_show(void);  /* explanation after the title, pages 0-2 */

#endif
