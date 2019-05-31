#include "xonix.h"

#define DEFAULT_DELAY 100

// bitmask representation of cells in the field
uint8_t field[FIELD_HEIGHT][FIELD_BYTES];


void xonix_main() {
    uint32_t wait_until = millis() + DEFAULT_DELAY;
    
    xonix_init();
    
    while (1) {
        if (millis() >= wait_until) {
            wait_until = millis() + DEFAULT_DELAY;
            xonix_step();
        }
        
        // read input
        xonix_draw_field();
    }
}

void xonix_init() {
    xonix_init_field();
    xonix_draw_field();
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


void xonix_draw_field() {
    uint8_t cur_bit = 0, start = 0, end = 0;
    uint8_t mask, val, same, r, c;

    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0x000000);
    
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
                    tft_fill_area(r*CELL_SIZE, start*CELL_SIZE, 
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
            tft_fill_area(r*CELL_SIZE, start*CELL_SIZE, 
                    (end-start)*CELL_SIZE, CELL_SIZE, 
                    cur_bit ? COLOR_GRASS : COLOR_ROAD);
        }
    }
}


void xonix_step() {
}

