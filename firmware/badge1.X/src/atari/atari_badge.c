#include "atari.h"
#include "atari_badge.h"

extern const uint8_t* kernel_01_rom;
extern struct register_file reg;

void atari_init();


void tia_line_ready() {
    // TODO: draw generated line on display at specified buffer
}

// main function of atari menu 
void atari_menu() {
    atari_init();
    
    while (1) {
    }
}


void atari_init() {
    set_rom(kernel_01_rom, 4096);
    init_tia();
    // TODO: reset vector should be used
    reg.PC = 0xF000;
}