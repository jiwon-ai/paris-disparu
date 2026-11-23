#include <gb/gb.h>
#include "sound.h"

void sound_init(void) {
    NR52_REG = 0x80;   /* sound on */
    NR50_REG = 0x77;   /* full master volume, both terminals */
    NR51_REG = 0xFF;   /* every channel to both speakers */
}

/* Channel 2: short square tick, quiet so it does not tire the ear. */
void sound_blip(void) {
    NR21_REG = 0x80;   /* 50% duty */
    NR22_REG = 0x21;   /* volume 2, fast decay */
    NR23_REG = 0x83;
    NR24_REG = 0x87;   /* trigger, frequency high bits */
}

/* Channel 1: two-note rising chime. */
void sound_confirm(void) {
    NR10_REG = 0x15;   /* upward sweep */
    NR11_REG = 0x80;
    NR12_REG = 0x52;
    NR13_REG = 0x00;
    NR14_REG = 0x86;
}

/* Channel 4: a burst of noise, like crates and cart wheels far away. */
void sound_market(uint8_t vol) {
    NR41_REG = 0x00;
    NR42_REG = (uint8_t)((vol << 4) | 0x02);
    NR43_REG = 0x45;
    NR44_REG = 0x80;
}

/* Channel 4: soft, long hiss - water against the quay. */
void sound_water(void) {
    NR41_REG = 0x00;
    NR42_REG = 0x17;   /* volume 1, slow decay */
    NR43_REG = 0x67;
    NR44_REG = 0x80;
}
