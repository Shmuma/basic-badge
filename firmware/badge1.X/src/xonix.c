#include "hw.h"
#include "log.h"
#include "xonix.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_DELAY 100

// bitmask representation of cells in the field
uint8_t field[FIELD_HEIGHT][FIELD_BYTES];
// enemies
uint8_t enemies_count = 1;
uint8_t enemies_x[ENEMIES_MAX];
uint8_t enemies_y[ENEMIES_MAX];
uint8_t enemies_d[ENEMIES_MAX];

// player
uint8_t player_x;
uint8_t player_y;
int8_t player_dx = 0;
int8_t player_dy = 1;


void xonix_main() {
    uint32_t wait_until = millis() + DEFAULT_DELAY;
    uint32_t steps_count = 0;
    
    xonix_init();
    
    while (1) {
        if (millis() >= wait_until) {
            xonix_step();
            wait_until = millis() + DEFAULT_DELAY;
            steps_count = (steps_count + 1) % 300; // 30 seconds
            
            if (!steps_count) {
                xonix_add_enemy();
            }
        }
        
        // read input
    }
}

void xonix_init() {
    start_after_wake = &xonix_wake;
    
    xonix_init_field();
    xonix_draw_field();
    
    xonix_init_enemies();
    xonix_draw_enemies();
    player_x = FIELD_WIDTH/2;
    player_y = 0;
    xonix_draw_player();
}


void xonix_wake() {
    xonix_draw_field();
    xonix_draw_enemies();
}

void xonix_init_field() {
    uint8_t r, c, v;
    
    for (r = 0; r < FIELD_HEIGHT; r++) {
        for (c = 0; c < FIELD_BYTES; c++) {
            v = 0xFF;
            if (r < 2 || r >= FIELD_HEIGHT-2)
                v = 0;
            else if (c == 0)
                v = 0b00111111;
            else if (c == FIELD_BYTES-1)
                v = 0b11111100;
            field[r][c] = v;
        }
    }
}


void xonix_init_enemies() {
    uint8_t i;
    for (i = 0; i < enemies_count; i++) {
        enemies_x[i] = (uint8_t)(rand() % GRASS_WIDTH)+2;
        enemies_y[i] = (uint8_t)(rand() % GRASS_HEIGHT)+2;
        enemies_d[i] = (uint8_t)(rand() % 4);
    }
}



void xonix_draw_field() {
    uint8_t cur_bit = 0, start = 0, end = 0;
    uint8_t mask, val, same, r, c;

    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0x0F0000);
    
    for (r = 0; r < FIELD_HEIGHT; r++) {
        cur_bit = start = end = 0;
        for (c = 0; c < FIELD_BYTES; c++) {
            mask = 1 << 7;
            val = field[r][c];
            
            while (mask) {
                if (cur_bit)
                    same = val & mask;
                else
                    same = ~val & mask;
                if (same)
                    end += 1;
                else {
                    tft_fill_area(start*CELL_SIZE, r*CELL_SIZE,
                            (end-start)*CELL_SIZE, CELL_SIZE, 
                            cur_bit ? COLOR_GRASS : COLOR_ROAD);
                    cur_bit = 1-cur_bit;
                    start = end;
                    end = start+1;
                            
                }
                        
                mask >>= 1;
            }
        }

        if (end > start) {
            tft_fill_area(start*CELL_SIZE, r*CELL_SIZE, 
                    (end-start)*CELL_SIZE, CELL_SIZE, 
                    cur_bit ? COLOR_GRASS : COLOR_ROAD);
        }
    }
}

// checks that cell of the field contains grass
inline uint8_t is_grass(uint8_t x, uint8_t y) {
    return field[y][x / 8] & (1 << (7 - x % 8));
    
    // x=0 -> 0, 7
    // x=1 -> 0, 6
    // x=6 -> 0, 1
    // x=7 -> 0, 0
    // x=8 -> 1, 7
}


inline void xonix_draw_cell(uint8_t r, uint8_t c, uint32_t color) {
    tft_fill_area(c*CELL_SIZE, r*CELL_SIZE, CELL_SIZE-1, CELL_SIZE-1, color);
}


void xonix_draw_enemies() {
    uint8_t i;
    
    for (i = 0; i < enemies_count; i++) {
        xonix_draw_cell(enemies_y[i], enemies_x[i], COLOR_ENEMY);
    }
}

void xonix_undraw_enemies() {
    uint8_t i;
    
    for (i = 0; i < enemies_count; i++) {
        xonix_draw_cell(enemies_y[i], enemies_x[i], COLOR_GRASS);
    }
}

void xonix_step_enemies() {
    uint8_t i, x, y;
    int8_t dy, dx;
    char buf[50];
    
    for (i = 0; i < enemies_count; i++) {
        dx = (enemies_d[i] & 0b01) ? 1 : -1;
        dy = (enemies_d[i] & 0b10) ? 1 : -1;
        
        x = enemies_x[i];
        y = enemies_y[i];

#if UART3_LOG_ENABLED        
        sprintf(buf, "x=%d y=%d d=%d", x, y, enemies_d[i]);
        log_str(buf);
#endif   
        
        if (!is_grass(x+dx, y+dy)) {
            if (is_grass(x+dx, y-dy))
                enemies_d[i] ^= 0b10;           // invert y dir
            else if (is_grass(x-dx, y+dy))
                enemies_d[i] ^= 0b01;           // invert x dir
            else
                enemies_d[i] ^= 0b11;           // invert both
            continue;
        }
        
        xonix_draw_cell(y+dy, x+dx, COLOR_ENEMY);
        xonix_draw_cell(y, x, COLOR_GRASS);
        enemies_x[i] = x + dx;
        enemies_y[i] = y + dy;
#if UART3_LOG_ENABLED        
        sprintf(buf, " -> x=%d y=%d d=%d ", enemies_x[i], 
                enemies_y[i], enemies_d[i]);
        log_strln(buf);
#endif        
    }
}

void xonix_add_enemy() {
    if (enemies_count+1 == ENEMIES_MAX)
        return;
    xonix_undraw_enemies();
    enemies_count++;
    xonix_init_enemies();
    xonix_draw_enemies();
}

void xonix_step() {
    xonix_step_enemies();
    xonix_step_player();
}

inline void xonix_draw_player() {
    xonix_draw_cell(player_y, player_x, COLOR_PLAYER);
}


void xonix_step_player() {
    uint8_t x, y;
    
    if (player_dx == 0 && player_dy == 0)
        return;
    
    // if we're reached bounds
    if ((player_dx < 0 && player_x == 0) || \
        (player_dx > 0 && player_x == FIELD_WIDTH-1))
    {
        player_dx = 0;
        return;
    }

    if ((player_dy < 0 && player_y == 0) || \
        (player_dy > 0 && player_y == FIELD_HEIGHT-1))
    {
        player_dy = 0;
        return;
    }
    
    x = player_x + player_dx;
    y = player_y + player_dy;
    
    xonix_draw_cell(y, x, COLOR_PLAYER);
    if (is_grass(player_x, player_y))
        xonix_draw_cell(player_y, player_x, COLOR_GRASS);
    else
        xonix_draw_cell(player_y, player_x, COLOR_ROAD);
    player_x = x;
    player_y = y;
}