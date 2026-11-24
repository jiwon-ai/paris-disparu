/* Episode 2 rooms: NPCs, things to examine, doors and exits.
   Les Halles rooms reuse Episode 1's text maps (assets/maps); the new
   districts are painted scenes (tools/scenes_ep2.py). Coordinates: NPCs in
   pixels, hotspots and doors in tiles. */
#include "assets.h"
#include "game.h"
#include "world.h"

/* ============================================================ Les Halles */

/* --- rue Rambuteau, Saint-Eustache lit up, the flower bed ------------- */
static const npc_def_t eustache_npcs[] = {
    { 120, 104, SPR1_DANCER, 6 | S_BANK1, NPC_DANCER },
};
static const hotspot_t eustache_hotspots[] = {
    { 14, 10, 6, 2, DLG_FLOWERBED },     /* the lit bed before the church */
    { 14, 5, 6, 5, DLG_EUSTACHE_LIT },
    {  2, 9, 1, 2, DLG_SECRET_RIBBON },
    {  8, 9, 1, 2, DLG_CRATES_PYRAMID },
};
static const door_t eustache_doors[] = {
    { 4, 7, 1, 2, ROOM_CAFE, 72, 118, DIR_UP },
};

/* --- rue Baltard: crates in pyramids, the accordion, Fernand ---------- */
static const npc_def_t baltard_npcs[] = {
    { 64, 40, SPR_FERNAND_0, 1, NPC_FERNAND },
    { 32, 100, SPR_PORTER_0, 1, NPC_PORTER },
    { 120, 56, SPR1_MUSICIAN, 4 | S_BANK1, NPC_MUSICIAN },
};
static const hotspot_t baltard_hotspots[] = {
    {  5, 6, 3, 1, DLG_CRATES_PYRAMID },
    { 13, 8, 2, 3, DLG_CRATES_PYRAMID },
    { 10, 12, 5, 1, DLG_METRO_SIGN },
};
static const door_t baltard_doors[] = {
    { 11, 13, 2, 2, ROOM_METRO, 72, 108, DIR_UP },
};

/* --- the café: Simone and the telephone ------------------------------- */
static const npc_def_t cafe_npcs[] = {
    { 40, 18, SPR_CAFE_0, 3, NPC_CAFE },
};
static const hotspot_t cafe_hotspots[] = {
    { 17, 3, 1, 1, DLG_PHONE_LOOK },
    {  0, 4, 13, 2, TALK(NPC_CAFE) },
};
static const door_t cafe_doors[] = {
    { 9, 17, 2, 1, ROOM_EUSTACHE, 28, 72, DIR_DOWN },
};

/* --- the quai: Mme Roux's box ----------------------------------------- */
static const npc_def_t quai_npcs[] = {
    { 40, 46, SPR1_ROUX, 5 | S_BANK1, NPC_ROUX },
};
static const hotspot_t quai_hotspots[] = {
    {  0, 12, 20, 1, DLG_SECRET_MOON },
};

/* --- Les Halles ticket hall -------------------------------------------- */
static const hotspot_t metro_hotspots[] = {
    {  3, 4, 3, 5, DLG_METRO_PLATFORM },   /* the window, then the gate */
    { 12, 5, 4, 4, DLG_METRO_PLATFORM },
};
static const door_t metro_doors[] = {
    { 8, 17, 4, 1, ROOM_BALTARD, 64, 100, DIR_LEFT },
};

/* ============================================================ Cité */

/* --- Place Louis-Lépine: the flower market, the métro ----------------- */
static const npc_def_t fleurs_npcs[] = {
    { 104, 100, SPR1_FLOWER, 3 | S_BANK1, NPC_FLOWER },
};
static const hotspot_t fleurs_hotspots[] = {
    {  2, 9, 3, 3, DLG_METRO_CITE },
    {  7, 6, 13, 6, DLG_STALLS_CLOSED },
    {  0, 0, 20, 6, DLG_CITE_HELLO },
};

/* --- Notre-Dame square ------------------------------------------------- */
static const npc_def_t notredame_npcs[] = {
    { 104, 112, SPR1_CLOCHARDE, 4 | S_BANK1, NPC_CLOCHARDE },
};
static const hotspot_t notredame_hotspots[] = {
    {  5, 1, 2, 2, DLG_SECRET_OWL },
    {  9, 6, 2, 4, DLG_ND_POEM },          /* the rose window */
    {  0, 0, 20, 13, DLG_ND_FACADE },
    { 13, 16, 1, 1, DLG_SECRET_SHARD },
    {  4, 15, 9, 2, DLG_DIG_FENCE },
};

/* --- Place Dauphine: pétanque under the trees ------------------------- */
static const npc_def_t dauphine_npcs[] = {
    { 56, 96, SPR1_OLDMAN, 2 | S_BANK1, NPC_PETANQUE },
    { 96, 96, SPR1_OLDMAN, 4 | S_BANK1, NPC_PETANQUE2 },
};
static const hotspot_t dauphine_hotspots[] = {
    { 15, 12, 4, 1, DLG_SECRET_LOVERS },   /* the bench */
    {  0, 0, 20, 11, DLG_DAUPHINE_TREES },
};

/* ============================================================ Saint-Michel */

/* --- Place Saint-Michel: the fountain ---------------------------------- */
static const npc_def_t stmichel_npcs[] = {
    { 128, 100, SPR1_YOUNG_MAN, 5 | S_BANK1, NPC_STUDENT },
};
static const hotspot_t stmichel_hotspots[] = {
    {  3, 9, 2, 3, DLG_SECRET_DRAGON },
    { 11, 9, 2, 3, DLG_SECRET_DRAGON },
    {  5, 0, 6, 12, DLG_FOUNTAIN },
    { 15, 9, 3, 3, DLG_METRO_SM },
    {  8, 15, 1, 1, DLG_SM_PAVE },
};
static const door_t stmichel_doors[] = {
    { 0, 12, 1, 3, ROOM_BASPORT, 136, 104, DIR_LEFT },   /* down to the river */
};

/* --- rue de la Huchette ------------------------------------------------ */
static const npc_def_t huchette_npcs[] = {
    { 88, 92, SPR1_DOORMAN, 2 | S_BANK1, NPC_DOORMAN },
};
static const hotspot_t huchette_hotspots[] = {
    { 15, 13, 3, 2, DLG_SECRET_SLEEPCAT },
    {  0, 0, 20, 11, DLG_HUCHETTE_STREET },
};
static const door_t huchette_doors[] = {
    { 9, 10, 2, 2, ROOM_CAVEAU, 72, 112, DIR_UP },
};

/* --- the jazz cellar ---------------------------------------------------- */
static const npc_def_t caveau_npcs[] = {
    { 72, 40, SPR1_TRUMPET, 3 | S_BANK1, NPC_TRUMPET },
    { 24, 96, SPR1_YOUNG_MAN, 2 | S_BANK1, NPC_STUDENT },
};
static const hotspot_t caveau_hotspots[] = {
    { 17, 13, 3, 3, DLG_SECRET_DRUMSTICK },
    {  0, 0, 20, 4, DLG_CELLAR_POEM },
    {  0, 4, 20, 6, DLG_CELLAR_CROWD },
};
static const door_t caveau_doors[] = {
    { 8, 17, 4, 1, ROOM_HUCHETTE, 76, 96, DIR_DOWN },
};

/* --- under the bridge: the low quay ------------------------------------- */
static const npc_def_t basport_npcs[] = {
    { 72, 96, SPR1_THIEF_0, 7 | S_BANK1, NPC_LULU },
    { 32, 100, SPR1_SLEEPER, 1 | S_BANK1, NPC_SLEEPER },
    { 104, 104, SPR1_SLEEPER, 4 | S_BANK1, NPC_SLEEPER },
};
static const hotspot_t basport_hotspots[] = {
    {  0, 15, 20, 3, DLG_SECRET_MOON },
    { 16, 9, 4, 4, DLG_BASPORT_STAIRS },
};
static const door_t basport_doors[] = {
    { 19, 12, 1, 3, ROOM_STMICHEL, 8, 104, DIR_RIGHT },
};

/* ============================================================ Saint-Germain */

/* --- the church square -------------------------------------------------- */
static const npc_def_t stgermain_npcs[] = {
    { 120, 104, SPR1_PHILOSOPHER, 4 | S_BANK1, NPC_PHILOSOPHER },
};
static const hotspot_t stgermain_hotspots[] = {
    {  2, 9, 3, 3, DLG_METRO_SG },
    { 14, 8, 6, 4, DLG_BOOKSHOP },
    {  0, 0, 14, 12, DLG_SG_CHURCH },
};

/* --- the café terrace ---------------------------------------------------- */
static const npc_def_t terrasse_npcs[] = {
    { 48, 96, SPR1_OLDPOET, 6 | S_BANK1, NPC_OLDPOET },
    { 128, 88, SPR1_WAITER, 3 | S_BANK1, NPC_WAITER },
};
static const hotspot_t terrasse_hotspots[] = {
    {  9, 13, 2, 1, DLG_SECRET_INITIALS },
    {  1, 9, 1, 2, DLG_PHONE_LOOK },
    {  0, 0, 20, 11, DLG_SG_TERRACE },
    {  0, 11, 20, 4, DLG_SG_POEM },
};

#define N(a) (sizeof(a) / sizeof(a[0]))
/* exits: { down, up, left, right } */
const room_def_t rooms[ROOM_COUNT] = {
    /* ROOM_BALTARD */
    { baltard_npcs, N(baltard_npcs), baltard_hotspots, N(baltard_hotspots), baltard_doors, N(baltard_doors),
      { ROOM_QUAI, ROOM_EUSTACHE, NO_EXIT, NO_EXIT }, AMBIENCE_MARKET },
    /* ROOM_EUSTACHE */
    { eustache_npcs, N(eustache_npcs), eustache_hotspots, N(eustache_hotspots), eustache_doors, N(eustache_doors),
      { ROOM_BALTARD, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_STREET },
    /* ROOM_CAFE */
    { cafe_npcs, N(cafe_npcs), cafe_hotspots, N(cafe_hotspots), cafe_doors, N(cafe_doors),
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    /* ROOM_QUAI */
    { quai_npcs, N(quai_npcs), quai_hotspots, N(quai_hotspots), 0, 0,
      { NO_EXIT, ROOM_BALTARD, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    /* ROOM_METRO */
    { 0, 0, metro_hotspots, N(metro_hotspots), metro_doors, N(metro_doors),
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_HALL },
    /* ROOM_FLEURS */
    { fleurs_npcs, N(fleurs_npcs), fleurs_hotspots, N(fleurs_hotspots), 0, 0,
      { NO_EXIT, NO_EXIT, ROOM_DAUPHINE, ROOM_NOTREDAME }, AMBIENCE_STREET },
    /* ROOM_NOTREDAME */
    { notredame_npcs, N(notredame_npcs), notredame_hotspots, N(notredame_hotspots), 0, 0,
      { NO_EXIT, NO_EXIT, ROOM_FLEURS, NO_EXIT }, AMBIENCE_STREET },
    /* ROOM_DAUPHINE */
    { dauphine_npcs, N(dauphine_npcs), dauphine_hotspots, N(dauphine_hotspots), 0, 0,
      { NO_EXIT, NO_EXIT, NO_EXIT, ROOM_FLEURS }, AMBIENCE_STREET },
    /* ROOM_STMICHEL */
    { stmichel_npcs, N(stmichel_npcs), stmichel_hotspots, N(stmichel_hotspots), stmichel_doors, N(stmichel_doors),
      { NO_EXIT, NO_EXIT, NO_EXIT, ROOM_HUCHETTE }, AMBIENCE_STREET },
    /* ROOM_HUCHETTE */
    { huchette_npcs, N(huchette_npcs), huchette_hotspots, N(huchette_hotspots), huchette_doors, N(huchette_doors),
      { NO_EXIT, NO_EXIT, ROOM_STMICHEL, NO_EXIT }, AMBIENCE_STREET },
    /* ROOM_CAVEAU */
    { caveau_npcs, N(caveau_npcs), caveau_hotspots, N(caveau_hotspots), caveau_doors, N(caveau_doors),
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_CAFE },
    /* ROOM_BASPORT */
    { basport_npcs, N(basport_npcs), basport_hotspots, N(basport_hotspots), basport_doors, N(basport_doors),
      { NO_EXIT, NO_EXIT, NO_EXIT, NO_EXIT }, AMBIENCE_RIVER },
    /* ROOM_STGERMAIN */
    { stgermain_npcs, N(stgermain_npcs), stgermain_hotspots, N(stgermain_hotspots), 0, 0,
      { NO_EXIT, NO_EXIT, NO_EXIT, ROOM_TERRASSE }, AMBIENCE_STREET },
    /* ROOM_TERRASSE */
    { terrasse_npcs, N(terrasse_npcs), terrasse_hotspots, N(terrasse_hotspots), 0, 0,
      { NO_EXIT, NO_EXIT, ROOM_STGERMAIN, NO_EXIT }, AMBIENCE_CAFE },
};
