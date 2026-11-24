/* Room contents: NPCs, examinable spots, doors and exits.
   Maps are in assets/maps/; coordinates of hotspots/doors are in tiles. */
#include "assets.h"
#include "game.h"
#include "world.h"

/* --- ROOM_STREET: rue Rambuteau ------------------------------------------ */
static const npc_def_t street_npcs[] = {
    { 120, 64, SPR_STRANGER_0, 4, NPC_STRANGER },
    { 136, 100, SPR1_CHANTEUSE, 3 | S_BANK1, NPC_CHANTEUSE },   /* cameo */
    { 24, 108, SPR1_ACTOR, 6 | S_BANK1, NPC_ACTOR },            /* cameo */
};
static const hotspot_t street_hotspots[] = {
    {  0, 5, 1, 4, DLG_PLAQUE },     /* plaque above reach: whole wall column */
    {  9, 7, 1, 2, DLG_HOTEL },      /* the hotel's windows either side of its door (x10) */
    { 11, 7, 1, 2, DLG_HOTEL },
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
    {  0, 16, 4, 2, DLG_CAR },        /* a little grey car */
    {  6, 7, 2, 2, DLG_DEFENSE },     /* stone pier under the painted DEFENSE D'AFFICHER */
};
static const door_t street_doors[] = {
    { 4, 7, 1, 2, ROOM_CAFE, 72, 118, DIR_UP },
    { 10, 7, 1, 2, ROOM_HOTEL, 72, 118, DIR_UP },
};

/* --- ROOM_ENTRANCE: rue Baltard, the open street between the pavilions --- */
static const npc_def_t entrance_npcs[] = {
    { 64, 40, SPR_PORTER_0, 1, NPC_PORTER },    /* Dede, beside the cart */
    { 32, 100, SPR_PHOTO_0, 2, NPC_PHOTO },     /* photographing the unloading */
    { 40, 16, SPR1_FILMMAKER, 4 | S_BANK1, NPC_FILMMAKER },     /* cameo */
    { 100, 24, SPR1_MEATPORTER, 3 | S_BANK1, NPC_MEATPORTER },  /* white smock, half a beef */
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
    { 136, 96, SPR1_FORT2, 1 | S_BANK1, NPC_FORT2 },      /* another fort, resting */
    { 48, 120, SPR1_COOK, 3 | S_BANK1, NPC_COOK },         /* a cook in a white apron, eyeing the crates */
};
static const hotspot_t market_hotspots[] = {
    {  1, 3, 5, 3, DLG_STALL_CHOUX },
    {  7, 3, 6, 3, DLG_STALL_POMMES },
    { 14, 3, 6, 3, DLG_STALL_CAROTTES },
    {  4, 9, 1, 2, DLG_COLUMN },
    { 16, 9, 1, 2, DLG_SECRET_HEART },
    {  1, 13, 2, 2, DLG_CRATES },
    { 16, 15, 4, 2, DLG_BASKETS },
    { 18, 11, 2, 2, DLG_CRATES },
};
static const door_t market_doors[] = {
    { 8, 17, 4, 1, ROOM_ENTRANCE, 112, 26, DIR_LEFT },
};

/* --- ROOM_CAFE ------------------------------------------------------------- */
static const npc_def_t cafe_npcs[] = {
    { 40, 18, SPR_CAFE_0, 3, NPC_CAFE },
    { 88, 40, SPR1_WORKER_CAFE, 1 | S_BANK1, NPC_WORKER },
    { 24, 64, SPR1_PHILOSOPHER, 4 | S_BANK1, NPC_PHILOSOPHER }, /* cameo */
    { 44, 80, SPR1_CRITIC, 6 | S_BANK1, NPC_CRITIC },          /* cameo */
};
static const hotspot_t cafe_hotspots[] = {
    { 17, 3, 1, 1, DLG_PHONE_LOOK },
    {  0, 4, 13, 2, TALK(NPC_CAFE) },  /* talk to Simone across the counter */
    {  1, 1, 12, 3, DLG_SHELF },
    {  3, 10, 2, 1, DLG_TABLE },
    { 14, 11, 2, 1, DLG_SOUP },
    { 17, 15, 3, 1, DLG_CHAIRS },
    {  2, 10, 1, 1, DLG_CHAIRS },
    {  0, 15, 2, 2, DLG_SECRET_CAT },  /* the street cat, once it has run in */
    { 19, 4, 1, 2, DLG_PINBALL },
};
static const door_t cafe_doors[] = {
    { 9, 17, 2, 1, ROOM_STREET, 28, 72, DIR_DOWN },
};

/* --- ROOM_QUAI: the quai on the right bank --------------------------------- */
static const npc_def_t quai_npcs[] = {
    { 120, 60, SPR_SINGER_0, 4, NPC_SINGER },     /* listening to the river */
    { 40, 46, SPR1_ROUX, 5 | S_BANK1, NPC_ROUX },      /* Mme Roux, near her box */
    { 136, 44, SPR1_NOVELIST, 6 | S_BANK1, NPC_NOVELIST },      /* cameo */
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
    { 52, 56, SPR1_PLAYWRIGHT, 4 | S_BANK1, NPC_PLAYWRIGHT },   /* cameo */
};
static const hotspot_t pont_hotspots[] = {
    {  7, 7, 1, 1, DLG_GASLAMP },
    { 12, 9, 1, 1, DLG_GASLAMP },
    {  8, 16, 4, 1, DLG_BARRIER },
    {  6, 2, 2, 3, DLG_SECRET_MASCARON },
    {  6, 0, 2, 18, DLG_WATCH_WATER },    /* west parapet */
    { 12, 0, 2, 18, DLG_WATCH_WATER },    /* east parapet */
};

/* --- ROOM_BOURSE: west end of Les Halles, the Bourse de commerce -------- */
static const hotspot_t bourse_hotspots[] = {
    {  5, 6, 1, 2, DLG_BOURSE_DOOR },  /* before the rotunda: first match wins */
    {  0, 0, 11, 8, DLG_BOURSE },
    { 11, 0, 1, 8, DLG_MEDICI },
    { 12, 0, 4, 8, DLG_BRASSERIE_FRONT },
    {  8, 13, 2, 2, DLG_VESPASIENNE },
    {  1, 15, 8, 1, DLG_RAT },        /* where the rat runs */
    {  2, 9, 1, 2, DLG_LAMP },
    { 10, 9, 1, 2, DLG_LAMP },
    { 16, 0, 4, 7, DLG_WEST_PAVILION },
    { 16, 9, 4, 9, DLG_WEST_PAVILION },
    {  2, 12, 4, 3, DLG_VAN },          /* a ribbed van (1967 object) */
};
static const door_t bourse_doors[] = {
    { 13, 6, 1, 2, ROOM_BRASSERIE, 72, 118, DIR_UP },
};

/* --- ROOM_HOTEL: a small hotel on rue Rambuteau (fiction) ------------------ */
static const npc_def_t hotel_npcs[] = {
    { 32, 22, SPR_PUNCHER_0, 2, NPC_NIGHTPORTER },   /* uniform and cap, behind the desk */
};
static const hotspot_t hotel_hotspots[] = {
    {  0, 5, 8, 1, TALK(NPC_NIGHTPORTER) },   /* across the desk */
    {  1, 1, 5, 2, DLG_HOTEL_KEYS },
    {  7, 1, 4, 2, DLG_HOTEL_MIRROR },
    { 14, 1, 4, 4, DLG_HOTEL_STAIRS },
    { 12, 3, 1, 1, DLG_HOTEL_SWITCH },    /* reception telephone: guests asked to be put through */
    { 12, 8, 2, 1, DLG_HOTEL_CHAIR },
    {  2, 11, 2, 1, DLG_HOTEL_CHAIR },
};
static const door_t hotel_doors[] = {
    { 9, 17, 2, 1, ROOM_STREET, 76, 72, DIR_DOWN },
};

/* --- ROOM_BRASSERIE: open all night, by the Bourse ------------------------- */
static const npc_def_t brasserie_npcs[] = {
    { 112, 18, SPR1_WAITER, 3 | S_BANK1, NPC_WAITER },
    { 88, 64, SPR1_FORT2, 1 | S_BANK1, NPC_SOUPER },  /* at the middle table */
    { 124, 102, SPR1_ARCHIVIST, 6 | S_BANK1, NPC_ARCHIVIST },  /* cameo, by the far table */
};
static const hotspot_t brasserie_hotspots[] = {
    {  1, 3, 1, 1, DLG_PHONE_LOOK },      /* the second public telephone */
    {  7, 4, 13, 2, TALK(NPC_WAITER) },
    {  1, 1, 13, 2, DLG_BRA_MIRROR },
    { 15, 1, 4, 2, DLG_SHELF },
    {  1, 9, 3, 1, DLG_SOUP },
    {  8, 9, 3, 1, DLG_SOUP },
    { 15, 9, 3, 1, DLG_BRA_TABLE },
    {  1, 12, 3, 1, DLG_BRA_TABLE },
    {  8, 12, 3, 1, DLG_BRA_TABLE },
    { 15, 15, 3, 1, DLG_BRA_TABLE },
    { 19, 6, 1, 2, DLG_SCOPITONE },     /* jukebox with a film screen */
};
static const door_t brasserie_doors[] = {
    { 9, 17, 2, 1, ROOM_BOURSE, 100, 64, DIR_DOWN },
};

/* --- ROOM_METRO: the ticket hall of Les Halles station -------------------- */
static const npc_def_t metro_npcs[] = {
    { 84, 52, SPR_PUNCHER_0, 4, NPC_PUNCHER },    /* at the platform gate */
    { 136, 80, SPR1_WRITER, 2 | S_BANK1, NPC_WRITER },          /* cameo */
    { 140, 104, SPR1_YOUNG_MAN, 5 | S_BANK1, NPC_YOUNG_MAN },   /* cameo */
    { 48, 112, SPR1_SLEEPER, 1 | S_BANK1, NPC_SLEEPER },  /* blue work clothes, by the bench */
};
static const hotspot_t metro_hotspots[] = {
    {  2, 4, 3, 3, DLG_METRO_WINDOW },
    { 12, 5, 4, 2, DLG_METRO_GATE },
    {  6, 5, 4, 2, DLG_METRO_MAP },
    { 17, 3, 2, 4, DLG_POSTER },
    {  4, 15, 2, 1, DLG_BENCH },
    {  5, 1, 10, 1, DLG_METRO_NAME },
    {  0, 0, 20, 7, DLG_METRO_TILES },
    {  0, 16, 4, 2, DLG_SECRET_MOUSE },
};
static const door_t metro_doors[] = {
    { 8, 17, 4, 1, ROOM_ENTRANCE, 64, 100, DIR_LEFT },
};

const room_def_t rooms[ROOM_COUNT] = {
    { street_npcs, 3, street_hotspots, 15, street_doors, 2,
      { ROOM_ENTRANCE, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_STREET },
    { entrance_npcs, 4, entrance_hotspots, 7, entrance_doors, 2,
      { ROOM_QUAI, ROOM_STREET, ROOM_BOURSE, NO_EXIT }, AMBIENCE_MARKET },
    { market_npcs, 3, market_hotspots, 8, market_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
    { cafe_npcs, 4, cafe_hotspots, 9, cafe_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    { quai_npcs, 3, quai_hotspots, 16, 0, 0,
      { ROOM_PONT, ROOM_ENTRANCE, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    { pont_npcs, 3, pont_hotspots, 6, 0, 0,
      { NO_EXIT, ROOM_QUAI, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    { metro_npcs, 4, metro_hotspots, 8, metro_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
    { 0, 0, bourse_hotspots, 11, bourse_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, ROOM_ENTRANCE }, AMBIENCE_STREET },
    { hotel_npcs, 1, hotel_hotspots, 7, hotel_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    { brasserie_npcs, 3, brasserie_hotspots, 11, brasserie_doors, 1,
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
};
