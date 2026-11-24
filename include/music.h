#ifndef MUSIC_H
#define MUSIC_H
#include <gb/gb.h>
#include <stdint.h>
#include "music_songs.h"

#define MUSIC_BANK 4   /* bank of src/music_data.c */

typedef struct {
    uint8_t tempo;            /* frames per step (an eighth note) */
    const uint8_t *mel;       /* wave channel 3 */
    const uint8_t *acc;       /* pulse channel 1 */
} song_t;

extern const song_t music_songs[];      /* bank MUSIC_BANK */
extern const uint16_t music_freq[72];   /* bank MUSIC_BANK */

/* Channel 1 is shared with sound effects: an effect sets this to the
   frames it needs, and the music leaves channel 1 alone meanwhile. */
extern volatile uint8_t music_ch1_hold;

void music_init(void);              /* once, after sound_init: VBL handler */
void music_play(uint8_t song);      /* 1..SONG_COUNT; same song: keeps playing */
void music_stop(void);
extern uint8_t music_muted;
void music_set_muted(uint8_t m);    /* 1 = music off (sound effects stay) */
#endif
