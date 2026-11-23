#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>

extern uint8_t joy_held;     /* buttons currently down */
extern uint8_t joy_pressed;  /* buttons that went down this frame */

void input_update(void);

#endif
