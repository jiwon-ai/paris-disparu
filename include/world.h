#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

#define NO_EXIT 0xFF
#define NO_DIALOGUE 0xFF
#define MAX_NPCS 6

/* Something to examine with A, in tile coordinates. */
typedef struct {
    uint8_t x, y, w, h;
    uint8_t dlg;
} hotspot_t;

/* A non-player character placed in a room, in pixel coordinates. */
typedef struct {
    uint8_t x, y;
    uint8_t frame;       /* first of two idle frames (SPR_*) */
    uint8_t palette;     /* sprite palette 0-7 */
    uint8_t dlg_first;   /* dialogue the first time */
    uint8_t dlg_repeat;  /* dialogue afterwards */
    uint8_t flag;        /* game_flags bit set after the first talk */
} npc_def_t;

typedef struct {
    const npc_def_t *npcs;
    uint8_t npc_count;
    const hotspot_t *hotspots;
    uint8_t hotspot_count;
    uint8_t exits[4];    /* destination room per DIR_*, or NO_EXIT */
    uint8_t ambience;    /* 1 = market sounds get louder towards the east */
} room_def_t;

extern const room_def_t rooms[];
extern uint8_t cur_room;

void world_load(uint8_t room);
uint8_t world_solid(int16_t px, int16_t py);      /* pixel coordinates */
uint8_t world_hotspot(int16_t px, int16_t py);   /* dialogue id or NO_DIALOGUE */

#endif
