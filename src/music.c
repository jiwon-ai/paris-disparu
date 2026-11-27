/* Music driver: a melody on the wave channel (3), doubled a hair sharp on
   pulse channel 2 like the two reeds of a musette accordion (the beat
   between them is the "wet" musette tremolo), over a bass-and-chords
   accompaniment on pulse channel 1.
   Stepped from the VBL interrupt so the tempo holds whatever the game loop
   is doing. Songs live in ROM bank MUSIC_BANK (tools/music.py); this file
   stays in bank 0 because it runs in the interrupt. Effects keep channel
   4, and borrow channels 1 and 2 through music_ch1_hold / music_ch2_hold. */
#include <gb/gb.h>
#include "music.h"

typedef struct {
    const uint8_t *start, *p;
    uint8_t left;
} voice_t;

static voice_t mel, acc;
static uint8_t song_now, tempo, frame;
static volatile uint8_t playing;
uint8_t music_muted;
volatile uint8_t music_ch1_hold;
volatile uint8_t music_ch2_hold;
static uint16_t reed_x;         /* the same note on channel 2 */

/* Wave: one cycle, a soft reedy shape (accordion-like), peak 14 of 15. */
static const uint8_t wave[16] = {
    0x8A, 0xCD, 0xEE, 0xDC, 0xBA, 0x99, 0xAA, 0x98,
    0x75, 0x32, 0x11, 0x23, 0x45, 0x66, 0x55, 0x67
};

static uint8_t next_note(voice_t *v, uint8_t *len) {
    uint8_t n = *v->p++;
    if (n == 0xFF) { v->p = v->start; n = *v->p++; }
    *len = *v->p++;
    return n;
}

/* Second reed on channel 2: one step sharp, steady volume 2. */
static void reed2(uint16_t x, uint8_t trigger) {
    if (music_ch2_hold) return;
    if (!x) { NR22_REG = 0x00; NR24_REG = 0x80; return; }
    x++;
    if (trigger) { NR21_REG = 0x80; NR22_REG = 0x20; }
    NR23_REG = (uint8_t)x;
    NR24_REG = (trigger ? 0x80 : 0x00) | (uint8_t)(x >> 8);
}

static void music_tick(void) {
    uint8_t n, len, save;
    if (music_ch1_hold) music_ch1_hold--;
    if (music_ch2_hold && !--music_ch2_hold && playing) reed2(reed_x, 1);
    if (!playing) return;
    if (++frame < tempo) return;
    frame = 0;
    save = _current_bank;
    SWITCH_ROM(MUSIC_BANK);
    if (--mel.left == 0) {
        n = next_note(&mel, &len);
        mel.left = len;
        if (n) {
            uint16_t x = music_freq[n + 11];     /* the wave channel sounds an octave low */
            NR32_REG = 0x40;                     /* 50% */
            NR33_REG = (uint8_t)x;
            NR34_REG = 0x80 | (uint8_t)(x >> 8);
            reed_x = music_freq[n - 1];          /* channel 2 at the real pitch */
            reed2(reed_x, 1);
        } else {
            NR32_REG = 0x00;
            reed_x = 0;
            reed2(0, 0);
        }
    }
    if (--acc.left == 0) {
        n = next_note(&acc, &len);
        acc.left = len;
        if (n && !music_ch1_hold) {
            uint16_t x = music_freq[n - 1];
            NR10_REG = 0x00;
            if (n < 25) {                        /* below C4: the bass, round and longer */
                NR11_REG = 0x80;                 /* 50% duty */
                NR12_REG = 0x54;                 /* volume 5, slow decay */
            } else {                             /* the chord: a light pluck */
                NR11_REG = 0x00;                 /* 12.5% duty */
                NR12_REG = 0x22;                 /* volume 2, short decay */
            }
            NR13_REG = (uint8_t)x;
            NR14_REG = 0x80 | (uint8_t)(x >> 8);
        }
    }
    SWITCH_ROM(save);
}

void music_init(void) {
    uint8_t i;
    NR30_REG = 0x00;
    for (i = 0; i < 16; i++) _AUD3WAVERAM[i] = wave[i];
    NR30_REG = 0x80;
    NR32_REG = 0x00;
    CRITICAL { add_VBL(music_tick); }
}

void music_play(uint8_t song) {
    uint8_t save;
    if (song == song_now && (playing || music_muted)) return;
    playing = 0;
    reed_x = 0;
    reed2(0, 0);
    if (music_muted) { song_now = song; return; }   /* remembered for later */
    save = _current_bank;
    SWITCH_ROM(MUSIC_BANK);
    tempo = music_songs[song - 1].tempo;
    mel.start = mel.p = music_songs[song - 1].mel;
    acc.start = acc.p = music_songs[song - 1].acc;
    SWITCH_ROM(save);
    mel.left = acc.left = 1;
    frame = tempo - 1;
    song_now = song;
    playing = 1;
}

/* SELECT: all sound off (music, effects, ambience) or back on; the music
   restarts from the start of the song. */
void music_set_muted(uint8_t m) {
    uint8_t s = song_now;
    music_muted = m;
    NR51_REG = m ? 0x00 : 0xFF;        /* no channel reaches the speakers */
    if (m) {
        playing = 0;
        NR32_REG = 0x00;                /* wave channel silent */
        reed_x = 0;
        reed2(0, 0);
        if (!music_ch1_hold) NR12_REG = 0x00;   /* accompaniment off */
    } else if (s) {
        song_now = 0;
        music_play(s);
    }
}

void music_stop(void) {
    playing = 0;
    song_now = 0;
    NR32_REG = 0x00;
    reed_x = 0;
    reed2(0, 0);
}
