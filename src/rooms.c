/* Room contents: NPCs, examinable spots, doors and exits.
   Maps are in assets/maps/; coordinates of hotspots/doors are in tiles. */
#include "assets.h"
#include "game.h"
#include "world.h"

/* --- ROOM_STREET: rue Rambuteau ------------------------------------------ */
static const npc_def_t street_npcs[] = {
    { 120, 64, SPR_STRANGER_0, 4, NPC_STRANGER },
    { 136, 100, SPR1_CHANTEUSE, 3 | S_BANK1, NPC_CHANTEUSE },   /* cameo */
    { 24, 108, SPR1_ACTOR, 4 | S_BANK1, NPC_ACTOR },            /* cameo */
};
static const hotspot_t street_hotspots[] = {
    {  0, 5, 1, 4, DLG_PLAQUE },     /* plaque above reach: whole wall column */
    { 10, 7, 1, 2, DLG_HOTEL },
    { 14, 5, 6, 4, DLG_EUSTACHE },  /* east end of the church */
    {  2, 9, 1, 2, DLG_LAMP },
    {  8, 9, 1, 2, DLG_LAMP },       /* not in front of the hotel door (x10) */
    { 18, 9, 1, 2, DLG_LAMP },
    {  7, 11, 1, 1, DLG_SECRET_COIN },  /* a coin in the gutter */
    {  0, 7, 6, 2, DLG_CAFE_WINDOW },
    {  8, 7, 6, 2, DLG_SHOP },
    {  6, 10, 2, 1, DLG_SOLEX },      /* VeloSolex against the wall */
    { 12, 9, 1, 2, DLG_BUSSTOP },     /* bus stop */
    { 16, 15, 4, 2, DLG_LORRY },      /* market lorry at the kerb */
};
static const door_t street_doors[] = {
    { 4, 7, 1, 2, ROOM_CAFE, 72, 118, DIR_UP },
};

/* --- ROOM_ENTRANCE: rue Baltard, the open street between the pavilions --- */
static const npc_def_t entrance_npcs[] = {
    { 64, 40, SPR_PORTER_0, 1, NPC_PORTER },    /* Dede, beside the cart */
    { 32, 100, SPR_PHOTO_0, 4, NPC_PHOTO },     /* photographing the unloading */
    { 40, 16, SPR1_FILMMAKER, 0 | S_BANK1, NPC_FILMMAKER },     /* cameo */
};
static const hotspot_t entrance_hotspots[] = {
    {  5, 6, 3, 1, DLG_CART_LOOK },   /* the cart: handled by events */
    { 13, 8, 2, 3, DLG_MORRIS },      /* Morris column */
    { 10, 12, 5, 1, DLG_METRO_SIGN },
    {  0, 7, 4, 2, DLG_SECRET_EYES },  /* covered street, west group: a cat's eyes */
    { 16, 12, 4, 2, DLG_COVERED },    /* covered street, east group */
    {  0, 0, 4, 18, DLG_PAVILION },
    { 16, 0, 4, 18, DLG_PAVILION },
};
static const door_t entrance_doors[] = {
    { 16, 3, 3, 3, ROOM_MARKET, 72, 112, DIR_UP },
    { 11, 13, 2, 2, ROOM_METRO, 72, 108, DIR_UP },   /* down the metro steps */
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
    { 16, 9, 1, 2, DLG_SECRET_HEART },
    {  1, 13, 2, 2, DLG_CRATES },
};
static const door_t market_doors[] = {
    { 8, 17, 4, 1, ROOM_ENTRANCE, 112, 26, DIR_LEFT },
};

/* --- ROOM_CAFE ------------------------------------------------------------- */
static const npc_def_t cafe_npcs[] = {
    { 40, 18, SPR_CAFE_0, 3, NPC_CAFE },
    { 88, 40, SPR_PORTER_0, 1, NPC_WORKER },
    { 24, 64, SPR1_PHILOSOPHER, 4 | S_BANK1, NPC_PHILOSOPHER }, /* cameo */
};
static const hotspot_t cafe_hotspots[] = {
    { 17, 3, 1, 1, DLG_PHONE_LOOK },
    {  0, 4, 13, 2, TALK(NPC_CAFE) },  /* talk to Simone across the counter */
    {  1, 1, 12, 3, DLG_SHELF },
    {  3, 10, 2, 1, DLG_TABLE },
    { 14, 11, 2, 1, DLG_SOUP },
    {  0, 15, 2, 2, DLG_SECRET_CAT },  /* the street cat, once it has run in */
};
static const door_t cafe_doors[] = {
    { 9, 17, 2, 1, ROOM_STREET, 28, 72, DIR_DOWN },
};

/* --- ROOM_QUAI: the quai on the right bank --------------------------------- */
static const npc_def_t quai_npcs[] = {
    { 120, 60, SPR_SINGER_0, 4, NPC_SINGER },     /* listening to the river */
    { 40, 46, SPR_MERCHANT_0, 4, NPC_ROUX },      /* Mme Roux, near her box */
};
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
    {  1, 10, 2, 2, DLG_SECRET_PIGEON },
    {  5, 10, 2, 2, DLG_TREE },
    { 13, 10, 2, 2, DLG_TREE },
    { 16, 10, 1, 2, DLG_WALLACE },    /* Wallace fountain */
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
    {  6, 2, 2, 3, DLG_SECRET_MASCARON },
    {  6, 0, 2, 18, DLG_WATCH_WATER },    /* west parapet */
    { 12, 0, 2, 18, DLG_WATCH_WATER },    /* east parapet */
};

/* --- ROOM_METRO: the ticket hall of Les Halles station -------------------- */
static const npc_def_t metro_npcs[] = {
    { 84, 52, SPR_PUNCHER_0, 4, NPC_PUNCHER },    /* at the platform gate */
    { 136, 80, SPR1_WRITER, 4 | S_BANK1, NPC_WRITER },          /* cameo */
};
static const hotspot_t metro_hotspots[] = {
    {  2, 4, 3, 3, DLG_METRO_WINDOW },
    { 12, 5, 4, 2, DLG_METRO_GATE },
    {  5, 1, 10, 1, DLG_METRO_NAME },
    {  0, 0, 20, 7, DLG_METRO_TILES },
    {  0, 16, 4, 2, DLG_SECRET_MOUSE },
};
static const door_t metro_doors[] = {
    { 8, 17, 4, 1, ROOM_ENTRANCE, 64, 100, DIR_LEFT },
};

const room_def_t rooms[ROOM_COUNT] = {
    { street_npcs, 3, street_hotspots, 12, street_doors, 1,
      { ROOM_ENTRANCE, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_STREET },
    { entrance_npcs, 3, entrance_hotspots, 7, entrance_doors, 2,
      { ROOM_QUAI, ROOM_STREET, NO_EXIT, NO_EXIT }, AMBIENCE_MARKET },
    { market_npcs, 1, market_hotspots, 6, market_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
    { cafe_npcs, 3, cafe_hotspots, 6, cafe_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    { quai_npcs, 2, quai_hotspots, 16, 0, 0,
      { ROOM_PONT, ROOM_ENTRANCE, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    { pont_npcs, 2, pont_hotspots, 6, 0, 0,
      { NO_EXIT, ROOM_QUAI, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    { metro_npcs, 2, metro_hotspots, 5, metro_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
};
