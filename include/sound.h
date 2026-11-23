#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_blip(void);          /* text typing tick */
void sound_confirm(void);       /* opening a dialogue / menu */
void sound_market(uint8_t vol); /* distant market clatter, vol 1-7 */
void sound_water(void);         /* the river */

#endif
