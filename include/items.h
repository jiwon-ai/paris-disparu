#ifndef ITEMS_H
#define ITEMS_H

#include <stdint.h>
#include <gb/gb.h>

/* Item pictures (tools/items.py): 24x32, six 8x16 sprites on OAM 24-29
   (the portrait slots), tiles loaded on demand to VRAM bank 1 ITEM_TILE.
   box_close() hides them. Code and data in ROM bank 4. */
extern uint8_t new_item;     /* ITEM_* just obtained (0: none): main shows it */
void item_show(uint8_t it, uint8_t x, uint8_t y) BANKED;   /* OAM x, y of the top-left */
uint8_t item_note(void) BANKED;   /* picture over the open caption, or a "You got"
                                     caption of its own (returns 1 then) */

#endif
