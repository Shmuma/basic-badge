#include "atari.h"
#include "atari_badge.h"
#include "../disp.h"
#include "../menu.h"

extern struct register_file reg;
extern struct tia_state tia;

struct menu_t root_menu = {
    .title = "Atari emulator",
    .items_count = 3,
    .items = (struct menu_item_t[3]){
        {.id = 1, .title = "Browse flash"},
        {.id = 2, .title = "Receive flash"},
        {.id = 3, .title = "Some complex item", .children_count = 3, 
            .children = (struct menu_item_t[3]){
                {.id = 31, .title = "Item 1"},
                {.id = 32, .title = "Item 2"},
                {.id = 33, .title = "Item 3"},
            }
        },
    },
};


void tia_line_ready(uint8_t line) {
    uint8_t x;
    uint32_t c;

    tft_set_write_area(0, line, FB_WIDTH*2, 1);
    TFT_24_7789_Write_Command(0x2C);
    
    for (x = 0; x < FB_WIDTH; x++) {
        c = COLOR_NTSC(tia.fb[x]);
        TFT_24_7789_Write_Data3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
        TFT_24_7789_Write_Data3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
    }
}

void atari_menu() {
    menu_run(&root_menu);
}

// start atari emulator
void atari_start() {
    int16_t rc;
    
    atari_init();
    
    while (1) {
        rc = mpu();
        if (rc > 0)
            atari_init();
        else
            tia_mpu_cycles(-rc);
        
    }
}


void atari_init() {
    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0);
    
    init_tia();
    reg.PC = reset_vector();
}
        