#include "atari.h"
#include "atari_badge.h"
#include "atari_uart.h"
#include "../disp.h"
#include "../menu.h"

#include <stdio.h>
#include <xc.h>

extern struct register_file reg;
extern struct tia_state tia;
extern struct pia_state pia;

uint8_t settings_debug_info = ONSCREEN_DEBUG;
uint32_t last_debug_ms = 0;

void atari_start();
void atari_init();
void atari_load_rom(uint8_t sector);
void atari_every_frame();
void atari_keyboard();
const char* get_menu_text_debug();
void show_debug_info();

#define MENU_BROWSE_FLASH           1
#define MENU_FLASH_SECTOR0         10
#define MENU_RECEIVE_ROMS           2
//#define MENU_RUN_BUILTIN            3
#define MENU_SETTINGS_MENU          4
#define MENU_SETTINGS_TOGGLE_DEBUG 40

const char const * menu_text_debug_enable = "Enable debug info";
const char const * menu_text_debug_disable = "Disable debug info";


const char* get_menu_text_debug() {
    return settings_debug_info ? menu_text_debug_disable : menu_text_debug_enable;
}


struct menu_t root_menu = {
    .title = "Atari emulator",
    .items_count = 3,
    .items = (struct menu_t[]){
        {.id = MENU_BROWSE_FLASH,   .title = "Run from flash", .items_count = 10,
            .items = (struct menu_t[]){
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
//        {.id = MENU_RUN_BUILTIN,    .title = "Run built-in ROM"},
        {.id = MENU_SETTINGS_MENU,  .title = "Settings >>>", .items_count = 3, 
            .items = (struct menu_t[]){
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

    if (line >= FB_HEIGHT)
        return;
    
    if (line == 0) {
        atari_every_frame();
        if (settings_debug_info)
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

//        if (res_id == MENU_RUN_BUILTIN) {
////            rom = ROM_NAME;
////            atari_start();
//        }
//        else 
        if (res_id >= MENU_FLASH_SECTOR0 && res_id <= MENU_FLASH_SECTOR0+9) {
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
        else {
            tia_mpu_cycles(-rc);
            mpu_clock_pia(-rc);
        }
    }
    brk_key = 0;
    enable_display_scanning(1);
}


void atari_init() {
    last_debug_ms = 0;
    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0);
    
    init_tia();
    init_pia();
    reg.PC = reset_vector();
}


// called at every frame
void show_debug_info() {
    static char buf[40];
    static uint8_t frame_idx = 0;
    
    uint32_t dt;
    uint8_t i;
    
    frame_idx++;
    frame_idx %= 10;
    if (!frame_idx) {
        if (last_debug_ms) {
            dt = (millis() - last_debug_ms)/10;
            snprintf(buf, sizeof(buf), "fps=%.1f (%d ms), i0=%d   ", 
                    1000.0/dt, dt, tia.inpt_pos[0]);
            for (i = 0; i < sizeof(buf) && buf[i]; i++) 
                tft_print_char(buf[i], i*8, FB_HEIGHT+1, 0xFFFFFF, 0);
        }
        last_debug_ms = millis();
    }
    
    
}


void atari_load_rom(uint8_t sector) {
    fl_read_4k(((uint32_t)sector) << 12, rom_data);
    rom = rom_data;
}


void atari_every_frame() {
    // check keyboard
    pia_pa_clear();
    pia_pb_clear();
    tia_fire(1, 0);
    tia_fire(0, 0);
    atari_keyboard();
}


void atari_keyboard() {
    static int8_t keys[3];
    uint8_t keys_count;
    
    keys_count = stdio_get(keys);
    while (keys_count-- > 0) {
        switch (keys[keys_count]) {
            case K_UP:
                pia_pa_set(DIR_UP, 1);
                break;
            case K_DN:
                pia_pa_set(DIR_DN, 1);
                break;
            case K_LT:
                pia_pa_set(DIR_LT, 1);
                break;
            case K_RT:
                pia_pa_set(DIR_RT, 1);
                break;
            case '-':
                pia_reset();
                break;
            case K_DEL:
                pia_select();
                break;
            case '=':
                tia_fire(1, 1);
                break;
            case 'q':
                tia_pod_move(0, -2);
                break;
            case 'a':
                tia_pod_move(0, 2);
                break;
            case 'w':
                tia_pod_set(0, TIA_MIN_INPUT_POS);
                break;
            case 's':
                tia_pod_set(0, (TIA_MIN_INPUT_POS+TIA_MAX_INPUT_POS)>>1);
                break;
            case 'x':
                tia_pod_set(0, TIA_MAX_INPUT_POS);
                break;
            case 'e':
                tia_pod_move(1, -2);
                break;
            case 'd':
                tia_pod_move(1, 2);
                break;
            case 'r':
                tia_pod_set(1, TIA_MIN_INPUT_POS);
                break;
            case 'f':
                tia_pod_set(1, (TIA_MIN_INPUT_POS+TIA_MAX_INPUT_POS)>>1);
                break;
            case 'v':
                tia_pod_set(1, TIA_MAX_INPUT_POS);
                break;
        }
    }
    // allow keys to repeat
    reset_last_key();
}
