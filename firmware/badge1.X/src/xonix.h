#ifndef __BADGE_H
#define __BADGE_H

#include <stdint.h>
#include "disp.h"

#define CELL_SIZE           4             // how large one item of the field
#define FIELD_OFFSET        4
#define FIELD_WIDTH         (TFT_WIDTH/CELL_SIZE)
#define FIELD_HEIGHT        (TFT_HEIGHT/CELL_SIZE - FIELD_OFFSET)
#define FIELD_BYTES         (FIELD_WIDTH/8)

#define GRASS_WIDTH         (FIELD_WIDTH-4)
#define GRASS_HEIGHT        (FIELD_HEIGHT-4)

#define COLOR_GRASS     0x00FF00    // green
#define COLOR_HOT_GRASS 0xA040F0    // purple
#define COLOR_ROAD      0x0000F0    // blue
#define COLOR_PLAYER    0xFFFF00    // yellow
#define COLOR_ENEMY     0xFF0000    // red

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

inline uint8_t is_grass(uint8_t x, uint8_t y);
inline void set_hot(uint8_t x, uint8_t y);
inline void clear_hot(void);

inline void xonix_draw_cell(uint8_t r, uint8_t c, uint32_t color);
inline void xonix_draw_player();

void xonix_step_player();
void xonix_player_lost();

// commit functions
void xonix_commit_hot();
void xonix_commit_path();
void xonix_fill_hot(uint8_t x, uint8_t y);
void xonix_fill_hot_core(uint8_t x, uint8_t y);
uint8_t xonix_reclaim_hot();

#endif
