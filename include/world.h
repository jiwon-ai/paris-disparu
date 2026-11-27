#ifndef WORLD_H
#define WORLD_H

#include <stdint.h>

#define NO_EXIT 0xFF
#define NO_DIALOGUE 0xFF
#define MAX_NPCS 4
#define TALK(npc) (0x80 | (npc))   /* hotspot that talks to an NPC (e.g. across a counter) */

/* Something to examine with A, in tile coordinates.
   id is a dialogue id, or TALK(npc). */
typedef struct {
    uint8_t x, y, w, h;
    uint8_t id;
} hotspot_t;

/* Walking into this rectangle (tile coordinates) moves the player to another room. */
typedef struct {
    uint8_t x, y, w, h;
    uint8_t room;
    uint8_t to_x, to_y, to_dir;   /* arrival position (pixels) and facing */
} door_t;

#define S_BANK1 0x08     /* OAM attribute: tile data from VRAM bank 1 */
#define S_SPLIT 0x10     /* (DMG palette bit, ignored on CGB) right 8 px of a 16x16 NPC use palette 2 */

/* A non-player character placed in a room, in pixel coordinates. */
typedef struct {
    uint8_t x, y;
    uint8_t frame;       /* first of two idle frames (SPR_*) */
    uint8_t palette;     /* sprite palette 0-7, | S_BANK1 for a one-frame cameo in VRAM bank 1 */
    uint8_t npc;         /* NPC_* id, see events.c */
} npc_def_t;

#define AMBIENCE_NONE   0
#define AMBIENCE_STREET 1   /* market noise louder towards the east */
#define AMBIENCE_MARKET 2   /* constant, busy */
#define AMBIENCE_HALL   3   /* inside the pavilion, echoing */
#define AMBIENCE_CAFE   4   /* cups and saucers */
#define AMBIENCE_RIVER  5   /* water against the quay */

typedef struct {
    const npc_def_t *npcs;
    uint8_t npc_count;
    const hotspot_t *hotspots;
    uint8_t hotspot_count;
    const door_t *doors;
    uint8_t door_count;
    uint8_t exits[4];    /* destination room per DIR_* screen edge, or NO_EXIT */
    uint8_t ambience;
} room_def_t;

extern const room_def_t rooms[];
extern uint8_t cur_room;

void world_load(uint8_t room);
extern uint8_t dawn_stage;                       /* quests done, 0-4: the clock and the sky */
void world_dawn(uint8_t stage);
extern palette_color_t bg_live[32];             /* tonight's background palettes */                  /* set it and recolour the night now */
uint8_t world_solid(int16_t px, int16_t py);     /* pixel coordinates */
uint8_t world_hotspot(int16_t px, int16_t py);   /* hotspot id or NO_DIALOGUE */
uint8_t world_door(int16_t px, int16_t py);      /* door index or NO_EXIT */

#endif
