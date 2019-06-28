#include "atari.h"

struct tia_state tia = {0};


void init_tia() {
  // not much here for now
}


void poke_tia(uint16_t addr, uint8_t val) {
  if (addr == WSYNC) {
    // Q: should we check for color_clock overflow?
    do_wsync();
  }
  else if (addr >= COLUP0 && addr <= COLUBK) {
    tia.colu[addr - COLUP0] = val & ~1;
  }
}


uint8_t peek_tia(uint16_t addr) {
  return 0xFF;
}


void do_wsync() {
  // draw to the rest of scanline
  draw_pixels(CLK_HOR - tia.color_clock);
}


void draw_pixels(uint8_t count) {
  while (count--) {
    if (tia.color_clock >= CLK_HORBLANK) {
      if (tia.scanline >= SCN_VIS_START && tia.scanline < SCN_VIS_END) {
        // TODO: need to combine image, but have read only about background :)
        tia.fb[tia.color_clock - CLK_HORBLANK] = tia.colu[3];
      }
    }
    if (++tia.color_clock >= CLK_HOR) {
      tia.color_clock = 0;
      if (tia.scanline >= SCN_VIS_START && tia.scanline < SCN_VIS_END)
        tia_line_ready(tia.scanline - SCN_VIS_START);
      if (++tia.scanline >= SCN_VERT) {
    	tia.scanline = 0;
      }
    }
  }
}


#ifdef ATARI_POSIX
void tia_line_ready(uint8_t line) {
    uint8_t c;
  
    printf("SC%03d: ", line);
    for (c = 0; c < FB_WIDTH; c++) {
      printf("%02x ", tia.fb[c]);
    }
    printf("\n");
}
#endif
