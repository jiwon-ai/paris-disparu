/* Room contents: NPCs, examinable spots and exits. Maps are in assets/maps/. */
#include "assets.h"
#include "game.h"
#include "world.h"

static const npc_def_t street_npcs[] = {
    { 120, 64, SPR_PORTER_0, 1, DLG_PORTER_HELLO, DLG_PORTER_AGAIN, FLAG_MET_PORTER },
};

static const hotspot_t street_hotspots[] = {
    {  0, 5, 1, 4, DLG_PLAQUE },   /* plaque above reach: whole wall column */
    {  3, 7, 1, 2, DLG_DOOR },
    { 10, 7, 1, 2, DLG_DOOR },
    { 15, 7, 1, 2, DLG_HOTEL },
    {  2, 9, 1, 2, DLG_LAMP },
    { 10, 9, 1, 2, DLG_LAMP },
    { 18, 9, 1, 2, DLG_LAMP },
    {  0, 7, 6, 2, DLG_SHOP },
    {  8, 7, 6, 2, DLG_SHOP },
};

const room_def_t rooms[ROOM_COUNT] = {
    /* ROOM_STREET */
    { street_npcs, 1, street_hotspots, 9,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, 1 },
};
