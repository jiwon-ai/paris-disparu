#ifndef NOTEBOOK_H
#define NOTEBOOK_H

/* Full-screen pages on the window layer. */
void notebook_show(void);   /* START: goal, clues found, phone numbers known */
void end_show(void);        /* the mystery is solved */
void goal_print(uint8_t y); /* current goal, two rows, in the open box */
void intro_show(uint8_t page);  /* explanation after the title, pages 0-2 */

#endif
