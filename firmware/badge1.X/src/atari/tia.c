#include "atari.h"
#include "tia.h"

struct tia_state tia = {0};

void draw_pixels(uint8_t count);        // draw given amount of pixels
void tia_handle_input_capacitors();

#ifdef TRACE_TIA
extern uint32_t frame;
#endif

void init_tia() {
    memset(&tia, 0, sizeof(tia));
    tia.inpt_pos[0] = tia.inpt_pos[1] = tia.inpt_pos[2] = tia.inpt_pos[3] = \
            (TIA_MAX_INPUT_POS - TIA_MIN_INPUT_POS) >> 1;
            //0xC0-4;     // value for Pong debugging
}


static inline uint8_t _normalize_clock_pos(uint8_t pos) {
    if (pos < CLK_HORBLANK)
        return pos + CLK_HOR - CLK_HORBLANK;
    if (pos >= CLK_HOR)
        return pos + CLK_HORBLANK - CLK_HOR;
    return pos;
}

// reflect 20 bits of input
static inline uint32_t reflect_playfield(uint32_t x) {
    // center our playfield in 32 bits
    x <<= 6;
    // swap bits in 32 bits
    x = (x & 0x55555555) << 1  | (x & 0xAAAAAAAA) >> 1;
    x = (x & 0x33333333) << 2  | (x & 0xCCCCCCCC) >> 2;
    x = (x & 0x0F0F0F0F) << 4  | (x & 0xF0F0F0F0) >> 4;
    x = (x & 0x00FF00FF) << 8  | (x & 0xFF00FF00) >> 8;
    x = (x & 0x0000FFFF) << 16 | (x & 0xFFFF0000) >> 16;
    // move our playfield back
    return x >> 6;
}


// fills playfield
static void fill_pf() {
    uint8_t ofs = 0;
    uint8_t col_lt, col_rt;
    uint64_t pf = tia.pf.full;
    
    if (!pf) {
        memset(tia.fb+ofs, tia.colu_bk, FB_WIDTH-ofs);
        return;
    }
    
#ifdef TRACE_TIA
    printf("fill_pf: ofs=%d, pf=%llX\n", ofs, pf);
#endif
    
    if (tia.ctrlpf.bits.pf_score == 0)
        col_lt = col_rt = tia.colu_pf;
    else {
        col_lt = tia.colu_p0;
        col_rt = tia.colu_p1;
    }
    
    while (ofs < PF_RIGHT) {
        tia.fb[ofs] = tia.fb[ofs+1] = tia.fb[ofs+2] = tia.fb[ofs+3] = \
            (pf & 1) ? col_lt : tia.colu_bk;
        ofs += 4;
        pf >>= 1;
    }

        while (ofs < FB_WIDTH) {
        tia.fb[ofs] = tia.fb[ofs+1] = tia.fb[ofs+2] = tia.fb[ofs+3] = \
            (pf & 1) ? col_rt : tia.colu_bk;
        ofs += 4;
        pf >>= 1;
    }
    
#ifdef TRACE_TIA
    peek_fb_line(tia.scanline);
#endif
}

// need to be called after execution of MPU opcode with amount of CPU cycles
// returns extra cycles spent on hardware
uint8_t tia_mpu_cycles(uint8_t cycles) {
    uint8_t addr, val, res = 0;
    
    if (tia.pg_idx > 0) {
        switch (tia.pg_idx) {
            case 1:         // PG0
                if (tia.vdelp0)
                    tia.p0_d = tia.pg_val;
                else
                    tia.p0 = tia.pg_val;
                if (tia.vdelp1)
                    tia.p1 = tia.p1_d;
                break;
            case 2:
                if (tia.vdelp1)
                    tia.p1_d = tia.pg_val;
                else
                    tia.p1 = tia.pg_val;
                if (tia.vdelp0)
                    tia.p0 = tia.p0_d;
                break;
        }
        tia.pg_idx = 0;
    }
    
    draw_pixels(cycles*3);
    
    addr = tia.queue_addr;
    val = tia.queue_val;
    if (!addr)
        return 0;

    switch (addr) {
        case VSYNC:
#ifdef TRACE_TIA
            printf("SV TIA: VSYNC: %d\n", val != 0);
#endif
            tia.vsync_enabled = val & 0b10;
            tia.scanline = 0;
            tia.p0_mask = tia.p1_mask = 0;
            break;
        case VBLANK:
#ifdef TRACE_TIA
            printf("SV TIA: VBLANK: %02X\n", val);
#endif
            tia.draw_enabled = !(val & (1 << 1));
            tia.scanline = 0;
            tia.inpt45_latched = val & (1 << 6);
            // switch from grounded to free, start charging capacitors
            if (tia.inpt03_grounded && !(val & 0x80)) {
                tia.inpt_scanline = 0;
            }
            tia.inpt03_grounded = val & 0x80;
            if (tia.inpt03_grounded) {
                tia.fire.bits.inpt = 0;
            }
            break;
        case WSYNC:
#ifdef TRACE_TIA
            printf("SV TIA: WSYNC: draw %d pixels\n", CLK_HOR - tia.color_clock);
#endif
            if (tia.color_clock > 0) {
                res = (CLK_HOR - tia.color_clock) / 3;
                draw_pixels(CLK_HOR - tia.color_clock);
            }
            break;
        case COLUP0:
            tia.colu_p0 = val & ~1;
            break;
        case COLUP1:
            tia.colu_p1 = val & ~1;
            break;
        case COLUPF:
            tia.colu_pf = val & ~1;
            break;
        case COLUBK:
            tia.colu_bk = val & ~1;
            break;
        case CTRLPF:
            // if first bit was changed, need to reflect right side of the playfield
            if ((val & 1) ^ (tia.ctrlpf.val & 1)) {
                tia.pf.parts.right = reflect_playfield(tia.pf.parts.right);
            }
            tia.ctrlpf.val = val;
            break;
        case NUSIZ0:
            tia.nusiz0.val = val;
            break;
        case NUSIZ1:
            tia.nusiz1.val = val;
            break;
        case PF0: {
            uint32_t tmp = (tia.pf.parts.left & ~0xFF) | (val >> 4);
            if (tia.pf.parts.left != tmp) {
                tia.pf.parts.left = tmp;
                if (tia.ctrlpf.bits.pf_ref)
                    tia.pf.parts.right = reflect_playfield(tia.pf.parts.left);
                else
                    tia.pf.parts.right = tia.pf.parts.left;
#ifdef TRACE_TIA
                printf("Set PF0: ref=%d, %llX, left %X, right %X\n", 
                        tia.ctrlpf.bits.pf_ref, tia.pf.full, tia.pf.parts.left,
                        tia.pf.parts.right);
#endif            
            }
            break;
        }
        case PF1: {
            uint32_t tmp = (tia.pf.parts.left & ~0xFF0) | (invert_bits_byte(val) << 4);
            if (tia.pf.parts.left != tmp) {
                tia.pf.parts.left = tmp;
                if (tia.ctrlpf.bits.pf_ref)
                    tia.pf.parts.right = reflect_playfield(tia.pf.parts.left);
                else
                    tia.pf.parts.right = tia.pf.parts.left;
#ifdef TRACE_TIA
                printf("Set PF1: ref=%d, %llX, left %X, right %X\n", 
                        tia.ctrlpf.bits.pf_ref, tia.pf.full, tia.pf.parts.left,
                        tia.pf.parts.right);
#endif            
            }
            break;
        }
        case PF2: {
            uint32_t tmp = (tia.pf.parts.left & ~0xFF000) | (val << 12);
            if (tia.pf.parts.left != tmp) {
                tia.pf.parts.left = tmp;
                if (tia.ctrlpf.bits.pf_ref)
                    tia.pf.parts.right = reflect_playfield(tia.pf.parts.left);
                else
                    tia.pf.parts.right = tia.pf.parts.left;
#ifdef TRACE_TIA
                printf("Set PF2: ref=%d, %llX, left %X, right %X\n", 
                        tia.ctrlpf.bits.pf_ref, tia.pf.full, tia.pf.parts.left,
                        tia.pf.parts.right);
#endif            
            }
            break;
        }
        case REFP0:
            tia.ref_p0 = val & 0b1000;
            break;
        case REFP1:
            tia.ref_p1 = val & 0b1000;
            break;
        case RESP0:
            tia.p0_pos = tia.color_clock + POS_CLOCK_OFS;
            if (tia.resmp0)
                tia.m0_pos = tia.p0_pos;
            break;
        case RESP1:
            tia.p1_pos = tia.color_clock + POS_CLOCK_OFS;
            if (tia.resmp1)
                tia.m1_pos = tia.p1_pos;
            break;
        case RESM0:
            tia.m0_pos = tia.color_clock + POS_CLOCK_OFS-1;
            break;
        case RESM1:
            tia.m1_pos = tia.color_clock + POS_CLOCK_OFS-1;
            break;
        case RESBL:
            tia.bl_pos = tia.color_clock + POS_CLOCK_OFS;
            break;
        case GRP0:
            tia.pg_idx = 1;
            tia.pg_val = val;
            break;
        case GRP1:
            tia.pg_idx = 2;
            tia.pg_val = val;
            break;
        case ENAM0:
            if (!tia.resmp0)
                tia.enam0 = (val >> 1) & 1;
            break;
        case ENAM1:
            if (!tia.resmp1)
                tia.enam1 = (val >> 1) & 1;
            break;
        case ENABL:
            if (tia.vdelbl)
                tia.enabl_d = (val >> 1) & 1;
            else
                tia.enabl = (val >> 1) & 1;
            break;
        case HMP0:
            tia.hmp0 = FOURBITS_2COMPL_TO_INT(val >> 4);
            break;
        case HMP1:
            tia.hmp1 = FOURBITS_2COMPL_TO_INT(val >> 4);
            break;
        case HMM0:
            tia.hmm0 = FOURBITS_2COMPL_TO_INT(val >> 4);
            break;
        case HMM1:
            tia.hmm1 = FOURBITS_2COMPL_TO_INT(val >> 4);
            break;
        case HMBL:
            tia.hmbl = FOURBITS_2COMPL_TO_INT(val >> 4);
            break;
        case VDELP0:
            if ((val & 1) != tia.vdelp0)
                tia.p0_d = tia.p0;
            tia.vdelp0 = val & 1;
            break;
        case VDELP1:
            if ((val & 1) != tia.vdelp1)
                tia.p1_d = tia.p1;
            tia.vdelp1 = val & 1;
            break;
        case VDELBL:
            tia.vdelbl = val & 1;
            break;
        case RESMP0:
            if (val & 0b10) {
                tia.enam0 = 0;
                tia.m0_pos = tia.p0_pos;
                tia.resmp0 = 1;
            }
            else
                tia.resmp0 = 0;
            break;
        case RESMP1:
            if (val & 0b10) {
                tia.enam1 = 0;
                tia.m1_pos = tia.p1_pos;
                tia.resmp1 = 1;
            }
            else
                tia.resmp1 = 0;
            break;
        case HMOVE:
            tia.p0_pos = _normalize_clock_pos(tia.p0_pos - tia.hmp0);
            tia.p1_pos = _normalize_clock_pos(tia.p1_pos - tia.hmp1);
            if (tia.resmp0)
                tia.m0_pos = tia.p0_pos;
            else
                tia.m0_pos = _normalize_clock_pos(tia.m0_pos - tia.hmm0);
            if (tia.resmp1)
                tia.m1_pos = tia.p1_pos;
            else
                tia.m1_pos = _normalize_clock_pos(tia.m1_pos - tia.hmm1);
            tia.bl_pos = _normalize_clock_pos(tia.bl_pos - tia.hmbl);
            break;
        case HMCLR:
            tia.hmp0 = tia.hmp1 = tia.hmbl = tia.hmm0 = tia.hmm1 = 0;
            break;
        case CXCLR:
            tia.cx.val = 0;
            break;
    }
    tia.queue_addr = 0;
    return res;
}

void poke_tia(uint16_t addr, uint8_t val) {
    tia.queue_addr = addr;
    tia.queue_val = val;
}

uint8_t peek_tia(uint16_t addr) {
    switch (addr)  {
        case TIA_RD_CXM0P:
            return tia.cx.bits.m0p1 << 7 | tia.cx.bits.m0p0 << 6;
        case TIA_RD_CXM1P:
            return tia.cx.bits.m1p0 << 7 | tia.cx.bits.m1p1 << 6;
        case TIA_RD_CXP0FB:
            return tia.cx.bits.p0pf << 7 | tia.cx.bits.p0bl << 6;
        case TIA_RD_CXP1FB:
            return tia.cx.bits.p1pf << 7 | tia.cx.bits.p1bl << 6;
        case TIA_RD_CXM0FB:
            return tia.cx.bits.m0pf << 7 | tia.cx.bits.m0bl << 6;
        case TIA_RD_CXM1FB:
            return tia.cx.bits.m1pf << 7 | tia.cx.bits.m1bl << 6;
        case TIA_RD_CXBLPF:
            return tia.cx.bits.blpf << 7;
        case TIA_RD_CXPPMM:
            return tia.cx.bits.p0p1 << 7 | tia.cx.bits.m0m1 << 6;
        case TIA_RD_INPT0:
            return (tia.fire.bits.inpt & 0b0001) << 7;
        case TIA_RD_INPT1:
            return (tia.fire.bits.inpt & 0b0010) << 6;
        case TIA_RD_INPT2:
            return (tia.fire.bits.inpt & 0b0100) << 5;
        case TIA_RD_INPT3:
            return (tia.fire.bits.inpt & 0b1000) << 4;
        case TIA_RD_INPT4:
            return ~(tia.fire.bits.p0 << 7);
        case TIA_RD_INPT5:
            return ~(tia.fire.bits.p1 << 7);
        default:
            return 0xFF;
    }
}

INLINE uint8_t _mask_clocks_from_psize(uint8_t psize) {
    if (psize == NUSIZ_DOUBLE)
        return 2;
    if (psize == NUSIZ_QUAD)
        return 4;
    return 1;
}


static inline uint8_t _is_player_clock(uint8_t nusiz, uint8_t pos) {
    uint8_t ofs = 0;
    
    if (tia.color_clock == pos)
        return 1;
    switch (nusiz) {
        case NUSIZ_TWO_8:
        case NUSIZ_THREE_8:
            ofs = 8+8;
            break;
        case NUSIZ_TWO_24:
        case NUSIZ_THREE_24:
            ofs = 24+8;
            break;
        case NUSIZ_TWO_56:
            ofs = 56+8;
            break;
        default:
            return 0;
    }

    pos = _normalize_clock_pos(pos + ofs);
    if (tia.color_clock == pos)
        return 1;
    if (nusiz == NUSIZ_THREE_8 || nusiz == NUSIZ_THREE_24) {
        if (tia.color_clock == _normalize_clock_pos(pos + ofs))
            return 1;        
    }
    return 0;
}


static inline uint8_t update_min_pos(uint8_t dist, uint8_t pos) {
    if (tia.color_clock < pos)
        return min(dist, pos - tia.color_clock);
    return dist;
}


// count amount of clocks we can draw without any object (player, missile or ball)
// GIVEN that no tia registers will be touched
static inline uint8_t clocks_to_object() {
    uint8_t res = FB_WIDTH - (tia.color_clock - CLK_HORBLANK);
    
    if (tia.enabl)
        res = update_min_pos(res, tia.bl_pos);
    
    if (tia.enam0)
        res = update_min_pos(res, tia.m0_pos);

    if (tia.enam1)
        res = update_min_pos(res, tia.m1_pos);
    
    if (tia.p0) {
        res = update_min_pos(res, tia.p0_pos);
        switch (tia.nusiz0.bits.psize_count) {
            case NUSIZ_THREE_8:
                res = update_min_pos(res, _normalize_clock_pos(tia.p0_pos + 8*4));
            case NUSIZ_TWO_8:
                res = update_min_pos(res, _normalize_clock_pos(tia.p0_pos + 8*2));
                break;
            case NUSIZ_THREE_24:
                res = update_min_pos(res, _normalize_clock_pos(tia.p0_pos + (8+24)*2));
            case NUSIZ_TWO_24:
                res = update_min_pos(res, _normalize_clock_pos(tia.p0_pos + 8+24));
                break;
            case NUSIZ_TWO_56:
                res = update_min_pos(res, _normalize_clock_pos(tia.p0_pos + 8+56));
                break;                
        }
    }

    if (tia.p1) {
        res = update_min_pos(res, tia.p1_pos);
        switch (tia.nusiz1.bits.psize_count) {
            case NUSIZ_THREE_8:
                res = update_min_pos(res, _normalize_clock_pos(tia.p1_pos + 8*4));
            case NUSIZ_TWO_8:
                res = update_min_pos(res, _normalize_clock_pos(tia.p1_pos + 8*2));
                break;
            case NUSIZ_THREE_24:
                res = update_min_pos(res, _normalize_clock_pos(tia.p1_pos + (8+24)*2));
            case NUSIZ_TWO_24:
                res = update_min_pos(res, _normalize_clock_pos(tia.p1_pos + 8+24));
                break;
            case NUSIZ_TWO_56:
                res = update_min_pos(res, _normalize_clock_pos(tia.p1_pos + 8+56));
                break;                
        }        
    }
    
    return res;
}


void draw_pixels(uint8_t count) {
    uint8_t ofs, col = COL_NONE;
    uint8_t draw_p0, draw_p1, skip = 0;
  
    while (count--) { 
        if (tia.color_clock >= CLK_HORBLANK) {
            if (skip > 0) {
                skip--;
            }
            else if (tia.draw_enabled && !tia.vsync_enabled) {
                // TODO: calculate player clocks only once per scanline
                if (tia.p0 && _is_player_clock(tia.nusiz0.bits.psize_count, tia.p0_pos)) {
                    tia.p0_mask = 1 << (tia.ref_p0 ? 0 : 7);
                    tia.p0_mask_cnt = tia.p0_mask_clocks = _mask_clocks_from_psize(tia.nusiz0.bits.psize_count);
                }
                if (tia.p1 && _is_player_clock(tia.nusiz1.bits.psize_count, tia.p1_pos)) {
                    tia.p1_mask = 1 << (tia.ref_p1 ? 0 : 7);
                    tia.p1_mask_cnt = tia.p1_mask_clocks = _mask_clocks_from_psize(tia.nusiz1.bits.psize_count);
                }
                if (tia.enabl && tia.bl_pos == tia.color_clock) {
                    tia.bl_clocks = 1 << tia.ctrlpf.bits.ballsize;
                }
                if (tia.enam0 && tia.m0_pos == tia.color_clock) {
                    tia.m0_clocks = 1 << tia.nusiz0.bits.msize;
                }
                if (tia.enam1 && tia.m1_pos == tia.color_clock) {
                    tia.m1_clocks = 1 << tia.nusiz1.bits.msize;
                }

                ofs = tia.color_clock - CLK_HORBLANK;
                col = COL_NONE;
                
                // P0 and P1 sprites could be cached to avoid masks fiddling
                draw_p0 = draw_p1 = 0;
                if (tia.p0_mask) {
                    draw_p0 = tia.p0_mask & tia.p0;
                    if (--tia.p0_mask_cnt == 0) {
                        if (tia.ref_p0)
                            tia.p0_mask <<= 1;
                        else
                            tia.p0_mask >>= 1;
                        tia.p0_mask_cnt = tia.p0_mask_clocks;
                    }
                }
                if (tia.p1_mask) {
                    draw_p1 = tia.p1_mask & tia.p1;
                    if (--tia.p1_mask_cnt == 0) {
                        if (tia.ref_p1)
                            tia.p1_mask <<= 1;
                        else
                            tia.p1_mask >>= 1;
                        tia.p1_mask_cnt = tia.p1_mask_clocks;
                    }
                }

                // check pf to detect collisions
                if (draw_p0 || draw_p1 || tia.bl_clocks > 0 || tia.m0_clocks > 0 || tia.m1_clocks > 0) {
                    if (tia.pf_cur & 1) {
                        if (draw_p0)
                            tia.cx.bits.p0pf = 1;
                        if (draw_p1)
                            tia.cx.bits.p1pf = 1;
                        if (tia.bl_clocks > 0)
                            tia.cx.bits.blpf = 1;
                        if (tia.m0_clocks > 0)
                            tia.cx.bits.m0pf = 1;
                        if (tia.m1_clocks > 0)  
                            tia.cx.bits.m1pf = 1;
                        if (tia.ctrlpf.bits.pf_prio)
                            draw_p0 = draw_p1 = 0;                        
                    }
                }
                
                if (draw_p0)
                    col = tia.colu_p0;
                else if (draw_p1)
                    col = tia.colu_p1;
                if (draw_p0 && draw_p1)
                    tia.cx.bits.p0p1 = 1;
                if (tia.bl_clocks > 0) {
                    // if PF has a priority over player or no player at all, draw ball
                    if (tia.ctrlpf.bits.pf_prio || (draw_p0 == 0 && draw_p1 == 0)) {
                        col = tia.colu_pf;      // COLUPF
                    }
                    tia.bl_clocks--;
                    if (draw_p0)
                        tia.cx.bits.p0bl = 1;
                    if (draw_p1)
                        tia.cx.bits.p1bl = 1;
                    if (tia.m0_clocks > 0)
                        tia.cx.bits.m0bl = 1;
                    if (tia.m1_clocks > 0)
                        tia.cx.bits.m1bl = 1;
                }
                if (tia.m1_clocks > 0) {
                    tia.m1_clocks--;
                    col = tia.colu_p1;
                    if (draw_p0)
                        tia.cx.bits.m1p0 = 1;
                    if (draw_p1)
                        tia.cx.bits.m1p1 = 1;
                    if (tia.m1_clocks > 0)
                        tia.cx.bits.m0m1 = 1;
                }
                // checking M0 last as it has higher priority than M1, so, we'll draw over
                if (tia.m0_clocks > 0) {
                    tia.m0_clocks--;
                    col = tia.colu_p0;
                    if (draw_p0)
                        tia.cx.bits.m0p0 = 1;
                    if (draw_p1)
                        tia.cx.bits.m0p1 = 1;
                }                    
                
                if (col != COL_NONE) {
                    tia.fb[ofs] = col;
                }
                else {
                    if (tia.p0_mask == 0 && tia.p1_mask == 0) {
                        skip = clocks_to_object()-1;
#ifdef TRACE_TIA
                        printf("Can skip %d clocks\n", skip);
#endif                    
                    }
                }
            }
            if ((tia.color_clock & 0b11) == 0b11) {
                tia.pf_cur >>= 1;
            }                
        }

#ifdef TRACE_TIA
        printf("TIA: frm=%d, col=%d, scan=%d, colubk=%02X, clr_stored=%02X, enam0=%d, resmp0=%d, p0=%02X, p0_d=%02X, p0_pos=%d, p0m=%02X, p0_cnt=%d, p1=%02X, p1_d=%02X, p1_pos=%d, p1m=%02X, p1_cnt=%d\n", 
                frame, tia.color_clock, tia.scanline, tia.colu_bk, col, tia.enam0, tia.resmp0,
                tia.p0, tia.p0_d, tia.p0_pos, tia.p0_mask, tia.p0_mask_cnt,
                tia.p1, tia.p1_d, tia.p1_pos, tia.p1_mask, tia.p1_mask_cnt);
#endif        
        
        if (++tia.color_clock >= CLK_HOR) {
            tia.color_clock = 0;
            tia.pf_cur = tia.pf.full;
            if (!tia.vsync_enabled && tia.inpt_scanline < TIA_MAX_INPUT_POS) {
                tia.inpt_scanline++;
            }
            if (!tia.inpt03_grounded)
                tia_handle_input_capacitors();
            if (tia.draw_enabled && !tia.vsync_enabled) {
                tia_line_ready(tia.scanline++);
                fill_pf();
            }
            tia.p0_mask = tia.p1_mask = 0;
            if (tia.vdelbl)
                tia.enabl = tia.enabl_d;
        }    
    }
}


void tia_fire(uint8_t p0, uint8_t set) {
    // if input is latched, reset is being ignored
    if (tia.inpt45_latched && !set)
        return;
    if (p0)
        tia.fire.bits.p0 = set != 0;
    else
        tia.fire.bits.p1 = set != 0;
}


void tia_pod_move(uint8_t pod_idx, int8_t dv) {
    tia.inpt_pos[pod_idx] = min(max(tia.inpt_pos[pod_idx] + dv, TIA_MIN_INPUT_POS), TIA_MAX_INPUT_POS);
}


void tia_pod_set(uint8_t pod_idx, uint8_t val) {
    if (val >= TIA_MIN_INPUT_POS && val <= TIA_MAX_INPUT_POS)
        tia.inpt_pos[pod_idx] = val;
}

void tia_handle_input_capacitors() {
    uint8_t i;
#ifdef TRACE_TIA
    printf("INPT: scanline=%d, bits=%X, poses=[%d %d %d %d]\n",
        tia.inpt_scanline, tia.fire.bits.inpt, tia.inpt_pos[0], 
        tia.inpt_pos[1], tia.inpt_pos[2], tia.inpt_pos[3]);
#endif    
    
    for (i = 0; i < 4; i++) {
        if (tia.inpt_scanline > tia.inpt_pos[i])
            tia.fire.bits.inpt |= 1 << i;
    }
}