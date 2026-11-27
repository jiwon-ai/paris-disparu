#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_blip(void);          /* text typing tick */
void sound_confirm(void);       /* opening a dialogue / menu */
void sound_market(uint8_t vol); /* a dull knock in the market, vol 1-4 */
void sound_water(void);         /* the river */
void sound_clink(void);         /* a cup in the cafe */
void sound_note(uint16_t x, uint8_t env);   /* channel 1 note (quest fanfare) */

#endif
