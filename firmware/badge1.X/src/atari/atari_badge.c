#include "atari.h"
#include "atari_badge.h"
#include "atari_uart.h"
#include "../disp.h"
#include "../menu.h"
#include <stdio.h>
#include <xc.h>

extern struct register_file reg;
extern struct tia_state tia;

uint8_t settings_debug_info = ONSCREEN_DEBUG;

void atari_start();
void atari_init();
void atari_load_rom(uint8_t sector);
const char* get_menu_text_debug();
void show_debug_info();

#define MENU_BROWSE_FLASH           1
#define MENU_FLASH_SECTOR0         10
#define MENU_RECEIVE_ROMS           2
#define MENU_RUN_BUILTIN            3
#define MENU_SETTINGS_MENU          4
#define MENU_SETTINGS_TOGGLE_DEBUG 40

const char const * menu_text_debug_enable = "Enable debug info";
const char const * menu_text_debug_disable = "Disable debug info";


const char* get_menu_text_debug() {
    return settings_debug_info ? menu_text_debug_disable : menu_text_debug_enable;
}


struct menu_t root_menu = {
    .title = "Atari emulator",
    .items_count = 4,
    .items = (struct menu_t[4]){
        {.id = MENU_BROWSE_FLASH,   .title = "Run from flash", .items_count = 10,
            .items = (struct menu_t[10]){
                {.id = MENU_FLASH_SECTOR0+0, .title = "Run sector 0"},
                {.id = MENU_FLASH_SECTOR0+1, .title = "Run sector 1"},
                {.id = MENU_FLASH_SECTOR0+2, .title = "Run sector 2"},
                {.id = MENU_FLASH_SECTOR0+3, .title = "Run sector 3"},
                {.id = MENU_FLASH_SECTOR0+4, .title = "Run sector 4"},
                {.id = MENU_FLASH_SECTOR0+5, .title = "Run sector 5"},
                {.id = MENU_FLASH_SECTOR0+6, .title = "Run sector 6"},
                {.id = MENU_FLASH_SECTOR0+7, .title = "Run sector 7"},
                {.id = MENU_FLASH_SECTOR0+8, .title = "Run sector 8"},
                {.id = MENU_FLASH_SECTOR0+9, .title = "Run sector 9"},
            }
        },
        {.id = MENU_RECEIVE_ROMS,   .title = "Receive ROMs via UART3"},
        {.id = MENU_RUN_BUILTIN,    .title = "Run built-in ROM"},
        {.id = MENU_SETTINGS_MENU,  .title = "Settings >>>", .items_count = 3, 
            .items = (struct menu_t[3]){
                {.id = MENU_SETTINGS_TOGGLE_DEBUG, .title_func = &get_menu_text_debug},
                {.id = 32, .title = "Item 2"},
                {.id = 33, .title = "Item 3"},
            }
        },
    },
};


void tia_line_ready(uint8_t line) {
    uint8_t x;
    uint32_t c;

    if (line == 0 && settings_debug_info) {
        show_debug_info();
    }

    tft_set_write_area(0, line, FB_WIDTH*2, 1);
    TFT_24_7789_Write_Command(0x2C);

    for (x = 0; x < FB_WIDTH; x++) {
        c = COLOR_NTSC(tia.fb[x]);
#if 0
        TFT_24_7789_Write_Data3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
        TFT_24_7789_Write_Data3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
#endif
        // inlined version of the above, gives +4 fps
        LCD_WR_CLR;
        LCD_PORT = c >> 16;
        LCD_WR_SET;
        LCD_WR_CLR;
        LCD_PORT = (c >> 8) & 0xFF;
        LCD_WR_SET;
        LCD_WR_CLR;
        LCD_PORT = c & 0xFF;
        LCD_WR_SET;
        LCD_WR_CLR;
        LCD_PORT = c >> 16;
        LCD_WR_SET;
        LCD_WR_CLR;
        LCD_PORT = (c >> 8) & 0xFF;
        LCD_WR_SET;
        LCD_WR_CLR;
        LCD_PORT = c & 0xFF;
        LCD_WR_SET;
    }
}

void atari_menu() {
    uint16_t res_id;
    static char buf[10];
    
    while (1) {
        res_id = menu_run(&root_menu);
        if (!res_id)
            break;

        if (res_id == MENU_RUN_BUILTIN) {
            rom = ROM_NAME;
            atari_start();
        }
        else if (res_id >= MENU_FLASH_SECTOR0 && res_id <= MENU_FLASH_SECTOR0+9) {
            atari_load_rom(res_id - MENU_FLASH_SECTOR0);
            atari_start();
        }
        else if (res_id == MENU_SETTINGS_TOGGLE_DEBUG)
            settings_debug_info = 1-settings_debug_info;
        else if (res_id == MENU_RECEIVE_ROMS)
            atari_receive_roms();
        else {
            video_set_color(EGA_WHITE, EGA_BLACK);
            video_clrscr();
            video_gotoxy(0, 0);
            snprintf(buf, sizeof(buf), "Id: %d", res_id);
            stdio_write(buf);
            wait_ms(2000);
        }
    }
    set_cursor_state(1);
}

// start atari emulator
void atari_start() {
    int16_t rc;
    
    enable_display_scanning(0);
    atari_init();
    
    while (!brk_key) {
        rc = mpu();
        if (rc > 0)
            atari_init();
        else
            tia_mpu_cycles(-rc);
    }
    brk_key = 0;
    enable_display_scanning(1);
}


void atari_init() {
    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0);
    
    init_tia();
    init_pia();
    reg.PC = reset_vector();
}


// called at every frame
void show_debug_info() {
    static char buf[20];
    static uint32_t last_ms = 0;
    uint32_t dt;
    uint8_t i;
    
    if (last_ms > 0) {
        dt = millis() - last_ms;
        snprintf(buf, sizeof(buf), "fps=%.1f (%d ms)", 1000.0/dt, dt);
        for (i = 0; i < sizeof(buf) && buf[i]; i++) 
            tft_print_char(buf[i], i*8, 200, 0xFFFFFF, 0);
    }
    
    last_ms = millis();
}


void atari_load_rom(uint8_t sector) {
//    volatile uint8_t sum;
//    uint16_t i;
    fl_read_4k(((uint32_t)sector) << 12, rom_data);
//    fl_read_nk(((uint32_t)sector) << 12, rom_data, 100);
//    fl_read_nk((((uint32_t)sector) << 12) + 1, rom_data+100, 100);
    
//    sum = 0;
//    for (i = 0; i < ROM_SIZE; i++)
//        sum += rom_data[i];
    
    rom = rom_data;
}