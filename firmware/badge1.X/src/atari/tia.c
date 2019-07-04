#include "atari.h"
#include "tia.h"

struct tia_state tia = {0};


void init_tia() {
    // not much here for now
}


// start or stop vsync. In our case, we ignore start/stop bit and always zero 
// current scanline
INLINE void do_vsync(uint8_t start) {
#ifdef TRACE_TIA
    printf("TIA: VSYNC: %d\n", start);
#endif
    tia.scanline = 0;
}


INLINE void do_wsync() {
#ifdef TRACE_TIA
    printf("TIA: WSYNC: draw %d pixels\n", CLK_HOR - tia.color_clock);
#endif

    // draw to the rest of scanline
    draw_pixels(CLK_HOR - tia.color_clock);
}

// need to be called after execution of MPU opcode with amount of CPU cycles
void tia_mpu_cycles(uint8_t cycles) {
    draw_pixels(cycles*3);
}

void poke_tia(uint16_t addr, uint8_t val) {
    if (addr == VSYNC) {
        do_vsync(val != 0);
    }
    else if (addr == WSYNC) {
        // Q: should we check for color_clock overflow?
        do_wsync();
    }
    else if (addr >= COLUP0 && addr <= COLUBK) {
        tia.colu[addr - COLUP0] = val & ~1;
    }
    else if (addr == PF0)
        tia.pf0 = val >> 4;
    else if (addr == PF1)
        tia.pf1 = val;
    else if (addr == PF2)
        tia.pf2 = val;
}


uint8_t peek_tia(uint16_t addr) {
    return 0xFF;
}


void draw_pixels(uint8_t count) {
    uint8_t ofs, pf_ofs, col;
  
    while (count--) {
        if (tia.color_clock >= CLK_HORBLANK) {
            if (tia.scanline >= SCN_VIS_START && tia.scanline < SCN_VIS_END) {
                ofs = tia.color_clock - CLK_HORBLANK;
                pf_ofs = ofs >> 2;
                col = tia.colu[3];
                if (ofs < PF0_MAX_CLK) {
                    if (tia.pf0 & (1 << (3-pf_ofs)))
                        col = tia.colu[2];
                }
                else if (ofs < PF1_MAX_CLK) {
                    if (tia.pf1 & (1 << (7-(pf_ofs-4))))
                        col = tia.colu[2];
                }
                else if (ofs < PF2_MAX_CLK) {
                    if (tia.pf2 & (1 << (7-(pf_ofs-4-8))))
                        col = tia.colu[2];
                }
                tia.fb[ofs] = col;
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
