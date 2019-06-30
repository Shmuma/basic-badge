#include "atari.h"
#include "atari_badge.h"
#include "../disp.h"
#include "../menu.h"
#include <stdio.h>

extern struct register_file reg;
extern struct tia_state tia;

void atari_start();
void atari_init();

#define MENU_BROWSE_FLASH       1
#define MENU_RECEIVE_FLASH      2
#define MENU_RUN_BUILTIN        3


struct menu_t root_menu = {
    .title = "Atari emulator",
    .items_count = 4,
    .items = (struct menu_t[4]){
        {.id = MENU_BROWSE_FLASH,   .title = "Browse flash"},
        {.id = MENU_RECEIVE_FLASH,  .title = "Receive flash"},
        {.id = MENU_RUN_BUILTIN,    .title = "Run built-in ROM"},
        {.title = "Some complex item", .items_count = 3, 
            .items = (struct menu_t[3]){
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
    uint16_t res_id;
    static char buf[10];
    
    while (1) {
        res_id = menu_run(&root_menu);
        if (!res_id)
            return;

        if (res_id == MENU_RUN_BUILTIN) {
            enable_display_scanning(0);
            atari_start();
            enable_display_scanning(1);
        }
        else {
            video_set_color(EGA_WHITE, EGA_BLACK);
            video_clrscr();
            video_gotoxy(0, 0);
            snprintf(buf, sizeof(buf), "Id: %d", res_id);
            stdio_write(buf);
            wait_ms(2000);
        }
    }
}

// start atari emulator
void atari_start() {
    int16_t rc;
    
    atari_init();
    
    while (!brk_key) {
        rc = mpu();
        if (rc > 0)
            atari_init();
        else
            tia_mpu_cycles(-rc);
    }
    brk_key = 0;
}


void atari_init() {
    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0);
    
    init_tia();
    reg.PC = reset_vector();
}
        