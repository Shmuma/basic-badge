#include "atari.h"
#include "atari_badge.h"
#include "../disp.h"

extern const uint8_t* kernel_01_rom;
extern struct register_file reg;
extern struct tia_state tia;

void atari_init();


void tia_line_ready(uint8_t line) {
    uint8_t x;
    uint32_t c;

//    c = COLOR_NTSC(tia.fb[0]);
//    tft_fill_area(0, tia.scanline, FB_WIDTH, 1, c);

//    tft_set_write_area(0, tia.scanline, FB_WIDTH, 1);
//    TFT_24_7789_Write_Command(0x2C);
//    
//    for (x = 0; x < FB_WIDTH; x++) {
//        c = COLOR_NTSC(tia.fb[x]);
//        TFT_24_7789_Write_Data3((c >> 16) & 0xFF, (c >> 8) & 0xFF, c & 0xFF);
//    }
}

// main function of atari menu 
void atari_menu() {
    uint16_t rc;
    
    atari_init();
    
    while (1) {
        rc = mpu();
        if (rc != 0)
            atari_init();
    }
}


void atari_init() {
    tft_fill_area(0, 0, TFT_WIDTH-1, TFT_HEIGHT-1, 0);
    
    set_rom(kernel_01_rom, 4096);
    init_tia();
    // TODO: reset vector should be used
    reg.PC = 0xF000;
}
