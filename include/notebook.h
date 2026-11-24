#ifndef NOTEBOOK_H
#define NOTEBOOK_H

#include <gb/gb.h>   /* BANKED: notebook.c lives in ROM bank 7 */
#include <stdint.h>

/* Full-screen pages on the window layer. */
void notebook_show(void) BANKED;   /* START: goal, clues found, phone numbers known */
uint16_t ambience_play(uint8_t r) BANKED;   /* a room sound; frames to the next */
void coin_note(void) BANKED;
void rat_update(uint8_t walking) BANKED;   /* the rat by the Bourse (OAM 18-19) */            /* caption: a coin picked up */
void day_start(void) BANKED;        /* ending: daybreak on the quai */
uint8_t day_update(uint8_t pressed) BANKED;   /* 1 when the player goes on */
void end_show(void) BANKED;        /* the mystery is solved */
void goal_print(uint8_t y) BANKED; /* current goal, two rows, in the open box */
void intro_show(uint8_t page) BANKED;
void extras_show(void) BANKED;
void curio_show(void) BANKED;   /* the curiosities, found and identified */
void album_show(void) BANKED;   /* the vanished-Paris objects found */
uint16_t notebook_number(void) BANKED;  /* the next phone number known, cycling (0 if none) */
extern uint8_t notebook_exch;          /* ... and its exchange */
uint8_t goal_npc(void) BANKED;          /* NPC_* to talk to next, or 0xFF */
void howto_show(uint8_t ev) BANKED;     /* rules card before a mini-game (EV_*) */
void teaser_show(void) BANKED;  /* explanation after the title, pages 0-2 */

#endif
