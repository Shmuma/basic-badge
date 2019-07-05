#include "atari.h"
#include "tia.h"

struct tia_state tia = {0};


void init_tia() {
    memset(&tia, 0, sizeof(tia));
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
    else if (addr == CTRLPF)
        tia.ctrlpf.val = val;
    else if (addr == RESP0)
        tia.p0_mask = 1 << 7;
    else if (addr == RESP1)
        tia.p1_mask = 1 << 7;
    else if (addr == GRP0)
        tia.p0 = val;
    else if (addr == GRP1)
        tia.p1 = val;
}


uint8_t peek_tia(uint16_t addr) {
    return 0xFF;
}

// check the playfield bit at this pixel offset (0..80)
INLINE uint8_t _check_pf(uint8_t pixel_ofs) {
    uint8_t pf_ofs = pixel_ofs >> 2;

    if (pixel_ofs < PF0_MAX_CLK) {
        // reversed (4-7 bits)
        return tia.pf0 & (1 << pf_ofs);
    }
    else if (pixel_ofs < PF1_MAX_CLK) {
         // direct (7-0)
        return tia.pf1 & (1 << (7-(pf_ofs-4)));   
    }
    else if (pixel_ofs < PF2_MAX_CLK) {
        // reversed (0-7)
        return tia.pf2 & (1 << (pf_ofs-4-8));
    }
    return 0;
}


void draw_pixels(uint8_t count) {
    uint8_t ofs, col, pf_col, draw_player, player_col;
  
    while (count--) {
        if (tia.color_clock >= CLK_HORBLANK) {
            if (tia.scanline >= SCN_VIS_START && tia.scanline < SCN_VIS_END) {
                ofs = tia.color_clock - CLK_HORBLANK;
                col = tia.colu[3];      // COLUBK
                draw_player = 0;
                if (tia.p0_mask) {
                    draw_player = tia.p0_mask & tia.p0;
                    tia.p0_mask >>= 1;
                    if (draw_player)
                        player_col = tia.colu[0];
                }
                if (tia.p1_mask) {
                    draw_player = tia.p1_mask & tia.p1;
                    tia.p1_mask >>= 1;
                    if (draw_player)
                        player_col = tia.colu[1];
                }
                
                if (ofs < PF_RIGHT) {
                    if (_check_pf(ofs)) {
                        col = tia.ctrlpf.bits.pf_score ? tia.colu[0] : tia.colu[2];
                        if (tia.ctrlpf.bits.pf_prio)
                            draw_player = 0;
                    }
                }
                else { // right side of the field
                    if (tia.ctrlpf.bits.pf_ref) {
                        // reflect the playfield
                        if (_check_pf(PF_RIGHT - (ofs - PF_RIGHT))) {
                            col = tia.ctrlpf.bits.pf_score ? tia.colu[1] : tia.colu[2];
                            if (tia.ctrlpf.bits.pf_prio)
                                draw_player = 0;                            
                        }
                    }
                    else {
                        // direct playfield
                        if (_check_pf(ofs - PF_RIGHT)) {
                            col = tia.ctrlpf.bits.pf_score ? tia.colu[1] : tia.colu[2];
                            if (tia.ctrlpf.bits.pf_prio)
                                draw_player = 0;
                        }
                    }
                }
                if (draw_player)
                    col = player_col;
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

