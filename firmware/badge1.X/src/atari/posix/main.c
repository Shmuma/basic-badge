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
    static uint32_t frame = 0;
    
    if (line == 0)
        frame++;
  
    printf("SC %04d, %03d: ", frame, line);
    for (c = 0; c < FB_WIDTH; c++) {
      printf("%02x", tia.fb[c]);
    }
    printf("\n");
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


int main(int argc, char** argv) {
    char* file_name = ROM_FILE;
    int16_t rc;
    uint32_t step = 0;

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
        if (rc < 0)
            tia_mpu_cycles(-rc);
        mpu_clock_pia();
        registers();
        printf("%d: rc = %d\n", step++, rc);
        if (rc > 0)
            break;
    }
    return 0;
}
