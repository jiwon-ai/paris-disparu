#include <gb/gb.h>
#include "input.h"

uint8_t joy_held;
uint8_t joy_pressed;

void input_update(void) {
    uint8_t prev = joy_held;
    joy_held = joypad();
    joy_pressed = joy_held & (uint8_t)~prev;
}
