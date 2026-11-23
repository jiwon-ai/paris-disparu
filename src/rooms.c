/* Room contents: NPCs, examinable spots, doors and exits.
   Maps are in assets/maps/; coordinates of hotspots/doors are in tiles. */
#include "assets.h"
#include "game.h"
#include "world.h"

/* --- ROOM_STREET: rue Rambuteau ------------------------------------------ */
static const npc_def_t street_npcs[] = {
    { 120, 64, SPR_STRANGER_0, 4, NPC_STRANGER },
};
static const hotspot_t street_hotspots[] = {
    {  0, 5, 1, 4, DLG_PLAQUE },     /* plaque above reach: whole wall column */
    { 10, 7, 1, 2, DLG_DOOR },
    { 15, 7, 1, 2, DLG_HOTEL },
    {  2, 9, 1, 2, DLG_LAMP },
    { 10, 9, 1, 2, DLG_LAMP },
    { 18, 9, 1, 2, DLG_LAMP },
    {  0, 7, 6, 2, DLG_CAFE_WINDOW },
    {  8, 7, 6, 2, DLG_SHOP },
};
static const door_t street_doors[] = {
    { 4, 7, 1, 2, ROOM_CAFE, 72, 118, DIR_UP },
};

/* --- ROOM_ENTRANCE: in front of the pavilion ------------------------------ */
static const npc_def_t entrance_npcs[] = {
    { 100, 60, SPR_PORTER_0, 1, NPC_PORTER },
};
static const hotspot_t entrance_hotspots[] = {
    { 15, 9, 3, 1, DLG_CART_LOOK },   /* the cart: handled by events */
    {  5, 9, 2, 2, DLG_CRATES },
    {  0, 2, 20, 6, DLG_PAVILION },
};
static const door_t entrance_doors[] = {
    { 9, 5, 3, 3, ROOM_MARKET, 72, 112, DIR_UP },
};

/* --- ROOM_MARKET: inside the pavilion -------------------------------------- */
static const npc_def_t market_npcs[] = {
    { 16, 40, SPR_MERCHANT_0, 2, NPC_MERCHANT },
};
static const hotspot_t market_hotspots[] = {
    {  1, 3, 5, 3, DLG_STALL_CHOUX },
    {  7, 3, 6, 3, DLG_STALL_POMMES },
    { 14, 3, 6, 3, DLG_STALL_CAROTTES },
    {  4, 9, 1, 2, DLG_COLUMN },
    { 16, 9, 1, 2, DLG_COLUMN },
    {  1, 13, 2, 2, DLG_CRATES },
};
static const door_t market_doors[] = {
    { 8, 17, 4, 1, ROOM_ENTRANCE, 72, 58, DIR_DOWN },
};

/* --- ROOM_CAFE ------------------------------------------------------------- */
static const npc_def_t cafe_npcs[] = {
    { 40, 18, SPR_CAFE_0, 3, NPC_CAFE },
    { 88, 40, SPR_PORTER_0, 1, NPC_WORKER },
};
static const hotspot_t cafe_hotspots[] = {
    { 17, 3, 1, 1, DLG_PHONE_LOOK },
    {  0, 4, 13, 2, TALK(NPC_CAFE) },  /* talk to Simone across the counter */
    {  1, 1, 12, 3, DLG_SHELF },
    {  3, 10, 2, 1, DLG_TABLE },
    { 14, 11, 2, 1, DLG_TABLE },
};
static const door_t cafe_doors[] = {
    { 9, 17, 2, 1, ROOM_STREET, 28, 72, DIR_DOWN },
};

/* --- ROOM_QUAI: the quai on the right bank --------------------------------- */
static const hotspot_t quai_hotspots[] = {
    {  3, 12, 2, 1, DLG_QUAI_BOX },    /* Mme Roux's box: reachable between the trees */
    {  0, 12, 2, 1, DLG_QUAI_BOX2 },
    { 14, 12, 2, 1, DLG_QUAI_BOX2 },
    { 17, 12, 2, 1, DLG_QUAI_BOX2 },
    {  0, 12, 20, 1, DLG_RIVER },     /* the rest of the parapet */
    {  2, 7, 1, 1, DLG_GASLAMP },
    { 17, 7, 1, 1, DLG_GASLAMP },
    {  7, 11, 1, 1, DLG_GASLAMP },
    { 12, 11, 1, 1, DLG_GASLAMP },
    {  1, 10, 2, 2, DLG_TREE },
    {  5, 10, 2, 2, DLG_TREE },
    { 13, 10, 2, 2, DLG_TREE },
    { 17, 10, 2, 2, DLG_TREE },
    {  0, 5, 7, 2, DLG_QUAI_SHOP },
    { 13, 5, 7, 2, DLG_QUAI_SHOP },
};

/* --- ROOM_PONT: on the bridge --------------------------------------------- */
static const npc_def_t pont_npcs[] = {
    { 80, 60, SPR_PAINTER_0, 2, NPC_PAINTER },
    { 92, 100, SPR_FERNAND_0, 1, NPC_FERNAND },   /* bareheaded, looking at the water */
};
static const hotspot_t pont_hotspots[] = {
    {  7, 7, 1, 1, DLG_GASLAMP },
    { 12, 9, 1, 1, DLG_GASLAMP },
    {  8, 16, 4, 1, DLG_BARRIER },
    {  6, 0, 2, 18, DLG_WATCH_WATER },    /* west parapet */
    { 12, 0, 2, 18, DLG_WATCH_WATER },    /* east parapet */
};

const room_def_t rooms[ROOM_COUNT] = {
    { street_npcs, 1, street_hotspots, 8, street_doors, 1,
      { ROOM_QUAI, NO_EXIT, NO_EXIT, ROOM_ENTRANCE }, AMBIENCE_STREET },
    { entrance_npcs, 1, entrance_hotspots, 3, entrance_doors, 1,
      { NO_EXIT, NO_EXIT, ROOM_STREET, NO_EXIT }, AMBIENCE_MARKET },
    { market_npcs, 1, market_hotspots, 6, market_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
    { cafe_npcs, 2, cafe_hotspots, 5, cafe_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    { 0, 0, quai_hotspots, 15, 0, 0,
      { ROOM_PONT, ROOM_STREET, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    { pont_npcs, 2, pont_hotspots, 5, 0, 0,
      { NO_EXIT, ROOM_QUAI, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
};
