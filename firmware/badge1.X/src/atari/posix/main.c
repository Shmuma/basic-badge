#include <stdio.h>
#include <stdlib.h>

#include "atari.h"

// simple coloring
//#define ROM_FILE "../roms/kernel_01.bin"
// coloring with TIA clocks
//#define ROM_FILE "../roms/kernel_13.bin"
// PlayField experiments
//#define ROM_FILE "../../../../../../Atari-roms/asm/kernel_pf.bin"

//#define ROM_FILE "../roms/pong.bin"
// players
//#define ROM_FILE "../../../../../../Atari-roms/asm/kernel_21.bin"
#define ROM_FILE "../../../../../../Atari-roms/atari_roms/space_invaders.bin"

extern struct register_file reg;
extern struct tia_state tia;
uint32_t frame = 0;
#define SHOW_COLORS 1

void atari_every_frame();


static void
registers()
{
  printf("   PC  SR AC XR YR SP NV.BDIZC\r\n");

  uint8_t bit = 0x80;

  printf("; %04X %02X %02X %02X %02X %02X ",
	 reg.PC, reg.SR, reg.AC, reg.XR, reg.YR, reg.SP);
  while (bit) {
    if (reg.SR & bit)
      putchar('1');
    else
      putchar('.');
    bit >>= 1;
  }

  printf("\r\n");
}


void tia_line_ready(uint8_t line) {
    uint8_t c;
    
    
    if (line == 0) {
        atari_every_frame();
        // start the game
        if (frame > 10 && frame < 20)
            pia_reset();
        frame++;
    }
  
    printf("SC %04d, %03d: ", frame, line);
#if SHOW_COLORS
    for (c = 0; c < FB_WIDTH; c++) {
        printf("\x1B[38;5;%dm#", tia.fb[c]);
    }
    printf("\x1B[0m\n");
#else    
    for (c = 0; c < FB_WIDTH; c++) {
      printf("%02x", tia.fb[c]);
    }
    printf("\n");
#endif
}


void atari_every_frame() {
    // check keyboard
    pia_pa_clear();
    pia_pb_clear();
    tia_fire(1, 0);
    tia_fire(0, 0);
}


int read_rom(const char* file_name) {
    FILE* f = fopen(file_name, "rb");

    if (!f)
        return 0;

    fseek(f, 0, SEEK_END);
    rom_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    rom = malloc(rom_size);
    fread((uint8_t*)rom, rom_size, 1, f);
    fclose(f);
    return 1;
}

void test() {
//    uint8_t x = 0b00010110;
//    
//    printf("res: %02X\n", invert_bits_byte(x));
    printf("res: %d\n", FOURBITS_2COMPL_TO_INT(0b1010));
}


int main(int argc, char** argv) {
    char* file_name = ROM_FILE;
    int16_t rc;
    uint32_t step = 0;
    test();

    init_tia();
    init_pia();
    if (argc == 2)
        file_name = argv[1];
    if (!read_rom(file_name)) {
        printf("Error reading rom %s\n", file_name);
        return -1;
    }
    printf("Rom loaded\n");
    // do not understand how this reset vector works, so, just start with fixed address
    reg.PC = reset_vector();
    printf("Reset vector: %04X\n", reg.PC);

    while (1) {
        if (reg.PC == 0xF254)
            printf("Break!\n");
        registers();
        rc = mpu();
        if (rc < 0) {
            tia_mpu_cycles(-rc);
            mpu_clock_pia(-rc);
        }
        registers();
        printf("%d: rc = %d\n", step++, rc);
        if (rc > 0)
            break;
    }
    return 0;
}
