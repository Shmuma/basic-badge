#ifndef __BADGE_H
#define __BADGE_H

#include <stdint.h>
#include "disp.h"

#define CELL_SIZE           4             // how large one item of the field
#define FIELD_WIDTH         (TFT_WIDTH/CELL_SIZE)
#define FIELD_HEIGHT        (TFT_HEIGHT/CELL_SIZE)
#define FIELD_BYTES         (FIELD_WIDTH/8)

#define GRASS_WIDTH         (FIELD_WIDTH-4)
#define GRASS_HEIGHT        (FIELD_HEIGHT-4)

#define COLOR_GRASS     0x00FF00
#define COLOR_ROAD      0x0000F0
#define COLOR_PLAYER    0xFFFF00
#define COLOR_ENEMY     0xFF0000

#define ENEMIES_MAX     10

void xonix_main(void);
void xonix_step(void);

void xonix_init(void);
void xonix_wake(void);
void xonix_init_field(void);
void xonix_init_enemies(void);
void xonix_draw_field(void);
void xonix_draw_enemies(void);
void xonix_undraw_enemies(void);
void xonix_step_enemies(void);
void xonix_add_enemy(void);

uint8_t is_grass(uint8_t x, uint8_t y);

#endif
