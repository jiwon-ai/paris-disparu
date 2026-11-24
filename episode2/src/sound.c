#include <gb/gb.h>
#include "sound.h"
#include "music.h"

/* All effects are single envelopes: one register write per channel, no
   sweeps and no frequency changes while a note plays. Volumes are kept low
   (envelope start 1-4 of 15) so the ambience stays in the background.
   Square pitch: 131072 / (2048 - x) Hz. Noise rate: 524288 / r / 2^(s+1) Hz. */

void sound_init(void) {
    NR52_REG = 0x80;   /* sound on */
    NR50_REG = 0x77;   /* master volume, both terminals */
    NR51_REG = 0xFF;   /* every channel to both speakers */
}

/* Channel 2: soft text tick, A5 (x=1899, 880 Hz), 16 ms. */
void sound_blip(void) {
    music_ch2_hold = 4;
    NR21_REG = 0x80;   /* 50% duty */
    NR22_REG = 0x11;   /* volume 1, decay 1/64 s */
    NR23_REG = 0x6B;
    NR24_REG = 0x87;   /* trigger, x high bits */
}

/* Channel 1: one soft chime, G5 (x=1881, 784 Hz), about 0.2 s. */
void sound_confirm(void) {
    music_ch1_hold = 16;
    NR10_REG = 0x00;   /* no sweep */
    NR11_REG = 0x80;   /* 50% duty */
    NR12_REG = 0x43;   /* volume 4, decay 3/64 s per step */
    NR13_REG = 0x59;
    NR14_REG = 0x87;
}

/* Channel 4: a dull knock (low noise, 293 Hz rate), like a crate set down.
   vol 1-4; it fades in vol/64 s. */
void sound_market(uint8_t vol) {
    NR41_REG = 0x00;
    NR42_REG = (uint8_t)((vol << 4) | 0x01);
    NR43_REG = 0x77;
    NR44_REG = 0x80;
}

/* Channel 4: soft, longer hush of water (1170 Hz rate), volume 1, 0.11 s. */
void sound_water(void) {
    NR41_REG = 0x00;
    NR42_REG = 0x17;
    NR43_REG = 0x57;
    NR44_REG = 0x80;
}

/* Channel 2: a faint high tick, C7 (x=1985, 2080 Hz), a cup on a saucer. */
void sound_clink(void) {
    music_ch2_hold = 6;
    NR21_REG = 0x40;   /* 25% duty */
    NR22_REG = 0x11;
    NR23_REG = 0xC1;
    NR24_REG = 0x87;
}

/* Channel 1: one square note, x = 2048 - 131072 / Hz; env = NR12 value. */
void sound_note(uint16_t x, uint8_t env) {
    music_ch1_hold = 24;
    NR10_REG = 0x00;
    NR11_REG = 0x80;
    NR12_REG = env;
    NR13_REG = (uint8_t)x;
    NR14_REG = 0x80 | (uint8_t)(x >> 8);
}
