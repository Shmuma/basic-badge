#include <stdio.h>
#include <stdlib.h>

#include "atari.h"

//#define ROM_FILE "pong.bin"
//#define ROM_FILE "../roms/kernel_01.bin"
//#define ROM_FILE "../roms/kernel_13.bin"
#define ROM_FILE "../../../../../../Atari-roms/asm/kernel_pf.bin"

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


int main() {
    int16_t rc;
    uint32_t step = 0;

    init_tia();
    if (!read_rom(ROM_FILE)) {
        printf("Error reading rom %s\n", ROM_FILE);
        return -1;
    }
    printf("Rom loaded\n");
    reg.PC = reset_vector();
    printf("Reset vector: %04X\n", reg.PC);

    while (1) {
        registers();
        rc = mpu();
        if (rc < 0)
            tia_mpu_cycles(-rc);
        registers();
        printf("%d: rc = %d\n", step++, rc);
        if (rc > 0)
            break;
    }
    return 0;
}
