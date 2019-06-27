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
    tia.colu[addr - COLUP0] = val;
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
      tia_line_ready();
      if (++tia.scanline >= SCN_VERT) {
    	tia.scanline = 0;
      }
    }
  }
}


#ifdef ATARI_POSIX
void line_ready() {
  uint8_t r, c;
  
  printf("Frame ready!\n");
  for (r = 0; r < FB_HEIGHT; r++) {
    for (c = 0; c < FB_WIDTH; c++) {
      printf("%02x ", tia.fb[r][c]);
    }
    printf("\n");
  }
}
#endif
