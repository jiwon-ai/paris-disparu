#ifndef ACTOR_H
#define ACTOR_H

#include <stdint.h>

typedef struct {
    int16_t x, y;      /* top-left of the 16x16 sprite, in screen pixels */
    uint8_t dir;
    uint8_t anim;      /* walk animation counter */
    uint8_t moving;
} player_t;

extern player_t player;

void actors_init_room(void);              /* spawn NPCs of cur_room */
void player_place(int16_t x, int16_t y, uint8_t dir);
#define PLAYER_DOOR 0x40
uint8_t player_update(void);   /* 0xFF, an edge exit DIR_*, or PLAYER_DOOR | door index */
void player_unstick(void);
uint8_t player_interact(void); /* hotspot id / TALK(npc) in front, or NO_DIALOGUE */
void actors_draw(void);
void actors_hide(void);

#endif
