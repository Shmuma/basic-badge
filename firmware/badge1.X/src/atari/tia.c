#include "atari.h"
#include "tia.h"

struct tia_state tia = {0};


INLINE void do_vsync(uint8_t start);
INLINE void do_vblank(uint8_t start);
INLINE void do_wsync();                 // draw the next of the line
void draw_pixels(uint8_t count);        // draw given amount of pixels


void init_tia() {
    memset(&tia, 0, sizeof(tia));
}


// start or stop vsync. In our case, we ignore start/stop bit and always zero 
// current scanline
INLINE void do_vsync(uint8_t start) {
#ifdef TRACE_TIA
    printf("SV TIA: VSYNC: %d\n", start);
#endif
    tia.vsync_enabled = start;
    tia.scanline = 0;
    tia.p0_mask = tia.p1_mask = 0;
}

INLINE void do_vblank(uint8_t start) {
#ifdef TRACE_TIA
    printf("SV TIA: VBLANK: %d\n", start);
#endif
    tia.draw_enabled = !start;
    tia.scanline = 0;
}


INLINE void do_wsync() {
#ifdef TRACE_TIA
    printf("SV TIA: WSYNC: draw %d pixels\n", CLK_HOR - tia.color_clock);
#endif
    if (tia.color_clock > 0) {
        // draw to the rest of scanline
        draw_pixels(CLK_HOR - tia.color_clock);
    }
}

// need to be called after execution of MPU opcode with amount of CPU cycles
void tia_mpu_cycles(uint8_t cycles) {
    uint8_t addr, val;
    draw_pixels(cycles*3);
    
    addr = tia.queue_addr;
    val = tia.queue_val;
    if (!addr)
        return;

    if (addr == WSYNC)
        do_wsync();
    else if (addr >= COLUP0 && addr <= COLUBK) {
        tia.colu[addr - COLUP0] = val & ~1;
    }
    else if (addr == NUSIZ0)
        tia.nusiz0.val = val;
    else if (addr == NUSIZ1)
        tia.nusiz1.val = val;
    else if (addr == PF0)
        tia.pf = (tia.pf & ~0xFF) | (val >> 4);
    else if (addr == PF1)
        tia.pf = (tia.pf & ~0xFF0) | (invert_bits_byte(val) << 4);
    else if (addr == PF2)
        tia.pf = (tia.pf & ~0xFF000) | (val << 12);
    else if (addr == CTRLPF)
        tia.ctrlpf.val = val;
    else if (addr == REFP0)
        tia.ref_p0 = val;
    else if (addr == REFP1)
        tia.ref_p1 = val;
    else if (addr == RESP0)
        tia.p0_pos = tia.color_clock;
    else if (addr == RESP1)
        tia.p1_pos = tia.color_clock;
    else if (addr == GRP0)
        tia.p0 = val;
    else if (addr == GRP1)
        tia.p1 = val;
    else if (addr == HMP0)
        tia.hmp0 = FOURBITS_2COMPL_TO_INT(val >> 4);
    else if (addr == HMP1)
        tia.hmp1 = FOURBITS_2COMPL_TO_INT(val >> 4);
    else if (addr == HMOVE) {
        tia.p0_pos -= tia.hmp0;
        if (tia.p0_pos < CLK_HORBLANK)
            tia.p0_pos += CLK_HOR - CLK_HORBLANK;
        else if (tia.p0_pos >= CLK_HOR)
            tia.p0_pos += CLK_HORBLANK - CLK_HOR;
        
        tia.p1_pos -= tia.hmp1;
        if (tia.p1_pos < CLK_HORBLANK)
            tia.p1_pos += CLK_HOR - CLK_HORBLANK;
        else if (tia.p1_pos >= CLK_HOR)
            tia.p1_pos += CLK_HORBLANK - CLK_HOR;
    }
    else if (addr == HMCLR) {
        tia.hmp0 = tia.hmp1 = 0;
    }
                
    tia.queue_addr = 0;
}

void poke_tia(uint16_t addr, uint8_t val) {
    if (addr == VSYNC)
        do_vsync(val != 0);
    else if (addr == VBLANK) {
        do_vblank(val & 1<<1);
        // TODO: handle INP latches/ground
    }
    else {
        tia.queue_addr = addr;
        tia.queue_val = val;
    }
}


uint8_t peek_tia(uint16_t addr) {
    if (addr == TIA_RD_INPT4)
        return (1-tia.fire.bits.p0) << 7;
    else if (addr == TIA_RD_INPT5)
        return (1-tia.fire.bits.p1) << 7;
    return 0xFF;
}

// check the playfield bit at this pixel offset (0..80)
static inline uint8_t _check_pf(uint8_t pixel_ofs) {
    return (tia.pf & (1 << (pixel_ofs>>2))) != 0;
}

INLINE uint8_t _mask_clocks_from_psize(uint8_t psize) {
    if (psize == NUSIZ_DOUBLE)
        return 2;
    if (psize == NUSIZ_QUAD)
        return 4;
    return 1;
}


void draw_pixels(uint8_t count) {
    uint8_t ofs, col = 0, pf_col, draw_player, player_col;
  
    while (count--) {
        if (tia.p0_pos == tia.color_clock) {
            tia.p0_mask = 1 << (tia.ref_p0 ? 0 : 7);
            tia.p0_mask_cnt = tia.p0_mask_clocks = _mask_clocks_from_psize(tia.nusiz0.bits.psize_count);
        }
        if (tia.p1_pos == tia.color_clock) {
            tia.p1_mask = 1 << (tia.ref_p1 ? 0 : 7);
            tia.p1_mask_cnt = tia.p1_mask_clocks = _mask_clocks_from_psize(tia.nusiz1.bits.psize_count);
        }        
        
        if (tia.color_clock >= CLK_HORBLANK) {
            if (tia.draw_enabled && !tia.vsync_enabled) {
                ofs = tia.color_clock - CLK_HORBLANK;
                col = tia.colu[3];      // COLUBK
                draw_player = 0;
                if (tia.p0_mask) {
                    draw_player = tia.p0_mask & tia.p0;
                    if (--tia.p0_mask_cnt == 0) {
                        if (tia.ref_p0)
                            tia.p0_mask <<= 1;
                        else
                            tia.p0_mask >>= 1;
                        tia.p0_mask_cnt = tia.p0_mask_clocks;
                    }
                    if (draw_player)
                        player_col = tia.colu[0];
                }
                else if (tia.p1_mask) {
                    draw_player = tia.p1_mask & tia.p1;
                    if (--tia.p1_mask_cnt == 0) {
                        if (tia.ref_p1)
                            tia.p1_mask <<= 1;
                        else
                            tia.p1_mask >>= 1;
                        tia.p1_mask_cnt = tia.p1_mask_clocks;
                    }
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
        
#ifdef TRACE_TIA
        printf("TIA: col=%d, scan=%d, colubk=%02X, clr_stored=%02X, p0=%02X, p0_pos=%d, p0m=%02X, p0_cnt=%d\n", 
                tia.color_clock, tia.scanline, tia.colu[3], col, tia.p0, tia.p0_pos,
                tia.p0_mask, tia.p0_mask_cnt);
#endif        
        if (++tia.color_clock >= CLK_HOR) {
            tia.color_clock = 0;
            if (tia.draw_enabled && !tia.vsync_enabled) {
                tia_line_ready(tia.scanline++);
            }
        }
    }
}


void tia_fire(uint8_t p0, uint8_t set) {
    if (p0)
        tia.fire.bits.p0 = set != 0;
    else
        tia.fire.bits.p1 = set != 0;
}