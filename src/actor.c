#include <gb/gb.h>
#include "assets.h"
#include "actor.h"
#include "game.h"
#include "input.h"
#include "world.h"
#include "events.h"
#include "notebook.h"

/* Player collision box (feet), relative to the sprite's top-left. */
#define HIT_X0 3
#define HIT_X1 12
#define HIT_Y0 10
#define HIT_Y1 15

/* NPC collision box. */
#define NPC_X0 2
#define NPC_X1 13
#define NPC_Y0 8
#define NPC_Y1 15

player_t player;

static uint8_t npc_count;
static uint8_t frame_count;
static uint8_t slow_step;

/* Walk cycle: stand, step, stand, other step. */
static const uint8_t walk_cycle[4] = { 0, 1, 0, 2 };
static const uint8_t dir_base[4] = { SPR_PLAYER_DOWN_0, SPR_PLAYER_UP_0, SPR_PLAYER_SIDE_0, SPR_PLAYER_SIDE_0 };

void player_place(int16_t x, int16_t y, uint8_t dir) {
    player.x = x;
    player.y = y;
    player.dir = dir;
    player.anim = 0;
    player.moving = 0;
}

void actors_init_room(void) {
    uint8_t i;
    npc_count = rooms[cur_room].npc_count;
    pool_load();                 /* this room's bank-1 sprites */
    for (i = 2 + (npc_count + 1) * 2; i < 40; i++) move_sprite(i, 0, 0);
}

/* Does the player's feet box at (x, y) hit the map or an NPC? */
static uint8_t blocked(int16_t x, int16_t y) {
    uint8_t i;
    int16_t x0 = x + HIT_X0, x1 = x + HIT_X1, y0 = y + HIT_Y0, y1 = y + HIT_Y1;

    /* the hitbox is 10 px wide, so it can straddle a 1-tile post: test its middle too */
    if (world_solid(x0, y0) || world_solid(x1, y0) ||
        world_solid(x0, y1) || world_solid(x1, y1) ||
        world_solid(x + 8, y0) || world_solid(x + 8, y1))
        return 1;
    for (i = 0; i < npc_count; i++) {
        const npc_def_t *n = &rooms[cur_room].npcs[i];
        if (x1 >= n->x + NPC_X0 && x0 <= n->x + NPC_X1 &&
            y1 >= n->y + NPC_Y0 && y0 <= n->y + NPC_Y1)
            return 1;
    }
    return 0;
}

/* After a room change, find free ground for the feet: first straight down
   or up (up to 48 px), then sideways along the entry row. Entering the quai
   or the bridge from the side of the opening used to leave the player
   inside a wall, pushed off the top of the screen: invisible. */
void player_unstick(void) {
    int16_t y0 = player.y, x0 = player.x;
    uint8_t i;
    for (i = 0; i < 48 && blocked(player.x, player.y); i++) player.y++;
    if (i < 48) return;
    player.y = y0;
    for (i = 0; i < 48 && blocked(player.x, player.y); i++) player.y--;
    if (i < 48 && player.y >= -9) return;
    player.y = y0;
    for (i = 1; i < SCREEN_W; i++) {
        if (x0 + i <= SCREEN_W - 13 && !blocked(x0 + i, y0)) { player.x = x0 + i; return; }
        if (x0 - i >= -3 && !blocked(x0 - i, y0)) { player.x = x0 - i; return; }
    }
}

uint8_t player_update(void) {
    int8_t dx = 0, dy = 0;
    const room_def_t *r = &rooms[cur_room];

    /* Four directions only: horizontal input wins over vertical. */
    if (joy_held & J_LEFT)       { dx = -1; player.dir = DIR_LEFT; }
    else if (joy_held & J_RIGHT) { dx = 1;  player.dir = DIR_RIGHT; }
    else if (joy_held & J_UP)    { dy = -1; player.dir = DIR_UP; }
    else if (joy_held & J_DOWN)  { dy = 1;  player.dir = DIR_DOWN; }

    player.moving = (dx | dy) != 0;
    if (!player.moving) {
        player.anim = 0;
        return 0xFF;
    }
    player.anim++;

    /* Carrying a crate: half speed (parity kept across taps of the D-pad). */
    slow_step ^= 1;
    if (carrying != NO_CRATE && slow_step) return 0xFF;

    /* Walking into a door. */
    {
        int16_t px = player.x + 8, py = player.y + 12;
        uint8_t d;
        if (dx < 0) px = player.x + HIT_X0 - 2;
        else if (dx > 0) px = player.x + HIT_X1 + 2;
        else if (dy < 0) py = player.y + HIT_Y0 - 4;
        else py = player.y + HIT_Y1 + 4;
        d = world_door(px, py);
        if (d != NO_EXIT) return PLAYER_DOOR | d;
    }

    /* Leaving through a screen edge that has an exit. */
    if (dx < 0 && player.x + HIT_X0 <= 0 && r->exits[DIR_LEFT] != NO_EXIT) return DIR_LEFT;
    if (dx > 0 && player.x + HIT_X1 >= SCREEN_W - 1 && r->exits[DIR_RIGHT] != NO_EXIT) return DIR_RIGHT;
    if (dy < 0 && player.y + HIT_Y0 <= 0 && r->exits[DIR_UP] != NO_EXIT) return DIR_UP;
    if (dy > 0 && player.y + HIT_Y1 >= SCREEN_H - 1 && r->exits[DIR_DOWN] != NO_EXIT) return DIR_DOWN;

    if (!blocked(player.x + dx, player.y + dy)) {
        player.x += dx;
        player.y += dy;
    }
    return 0xFF;
}

uint8_t player_interact(void) {
    int16_t px = player.x + 8, py = player.y + 12;
    uint8_t i;

    switch (player.dir) {
    case DIR_DOWN:  py = player.y + HIT_Y1 + 4; break;
    case DIR_UP:    py = player.y + HIT_Y0 - 4; break;
    case DIR_LEFT:  px = player.x + HIT_X0 - 4; break;
    default:        px = player.x + HIT_X1 + 4; break;
    }

    for (i = 0; i < npc_count; i++) {
        const npc_def_t *n = &rooms[cur_room].npcs[i];
        if (px >= n->x && px < n->x + ACTOR_WIDTH && py >= n->y && py < n->y + ACTOR_HEIGHT)
            return TALK(n->npc);
    }
    return world_hotspot(px, py);
}

static void draw16(uint8_t oam, int16_t x, int16_t y, uint8_t tile, uint8_t prop) {
    uint8_t sx = (uint8_t)(x + 8), sy = (uint8_t)(y + 16);
    set_sprite_prop(oam, prop);
    set_sprite_prop(oam + 1, (prop & S_SPLIT) ? (prop & 0xE8) | 2 : prop);
    if (prop & S_FLIPX) {
        set_sprite_tile(oam, tile + 2);
        set_sprite_tile(oam + 1, tile);
    } else {
        set_sprite_tile(oam, tile);
        set_sprite_tile(oam + 1, tile + 2);
    }
    move_sprite(oam, sx, sy);
    move_sprite(oam + 1, sx + 8, sy);
}

/* Draw the player and NPCs. Lower OAM slots win when sprites overlap, so
   actors are given slots from the bottom of the screen upwards: whoever
   stands lower (closer to the viewer) is drawn in front. */
void actors_draw(void) {
    uint8_t order[MAX_NPCS + 1];
    int16_t ys[MAX_NPCS + 1];
    uint8_t count = npc_count + 1, i, j;

    frame_count++;

    /* Index 0 = player, 1.. = NPCs; insertion sort by y, descending. */
    for (i = 0; i < count; i++) {
        int16_t y = i ? rooms[cur_room].npcs[i - 1].y : player.y;
        j = i;
        while (j && ys[j - 1] < y) {
            ys[j] = ys[j - 1];
            order[j] = order[j - 1];
            j--;
        }
        ys[j] = y;
        order[j] = i;
    }

    if (carrying != NO_CRATE) {
        /* Held above the head; OAM 0-1 so it is drawn over everything. */
        draw16(0, player.x, player.y - 9, SPR_CRATE, 5 + carrying);
    } else {
        move_sprite(0, 0, 0);
        move_sprite(1, 0, 0);
    }

    for (i = 0; i < count; i++) {
        uint8_t oam = 2 + i * 2;
        if (order[i] == 0) {
            uint8_t tile = dir_base[player.dir] + walk_cycle[(player.anim >> 3) & 3] * 4;
            draw16(oam, player.x, player.y, tile, player.dir == DIR_LEFT ? S_FLIPX : 0);
        } else {
            uint8_t k = order[i] - 1;
            const npc_def_t *n = &rooms[cur_room].npcs[k];
            /* Slow two-frame idle, offset per NPC so they do not breathe in sync. */
            uint8_t f = (n->palette & S_BANK1) ? 0 : ((uint8_t)(frame_count + k * 23) >> 5) & 1;
            draw16(oam, n->x, n->y, (n->palette & S_BANK1) ? npc_tile[k] : n->frame + f * 4, n->palette);
        }
    }
}

void sprite16(uint8_t oam, int16_t x, int16_t y, uint8_t tile, uint8_t prop) {
    draw16(oam, x, y, tile, prop);
}

void actors_hide(void) {
    uint8_t i;
    for (i = 0; i < 40; i++) move_sprite(i, 0, 0);
}
