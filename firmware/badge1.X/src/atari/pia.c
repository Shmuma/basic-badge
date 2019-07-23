#include "atari.h"

struct pia_state pia;


void init_pia() {
    pia_pa_clear();
    pia_pb_clear();
    pia.timer_val = 0;
    pia.interval_clocks = 0;
    pia.interval_left = 0;
}


void poke_pia(uint16_t address, uint8_t val) {
    switch (address) {
        case TIM1T:
            pia.reset_val = val;
            pia.reset_clocks = 1;
            break;
        case TIM8T:
            pia.reset_val = val;
            pia.reset_clocks = 1 << 3;
            break;
        case TIM64T:
            pia.reset_val = val;
            pia.reset_clocks = 1 << 6;
            break;
        case T1024T:
            pia.reset_val = val;
            pia.reset_clocks = (uint32_t)(1 << 10);
            break;
        case SWACNT:
            pia.pa_dir = val;
            break;
    }
}

uint8_t peek_pia(uint16_t address) {
    switch (address) {
        case SWCHA:
            return pia.pa.val & ~pia.pa_dir;
        case SWCHB:
            return pia.pb.val;
        case INTIM:
            pia.reached_zero = 0;
            return pia.timer_val;
    }
    return 0xFF;
}


void mpu_clock_pia(uint8_t clocks) {
    if (pia.interval_clocks) {
#ifdef TRACE_PIA
        printf("PIA: clocks %d, int %d, val %02X, left %d -> ",
               clocks, pia.interval_clocks, pia.timer_val, pia.interval_left);
#endif
        if (pia.interval_clocks == 1 || pia.reached_zero)
            pia.timer_val -= clocks;
        else {
            if (pia.interval_left < clocks) {
                if (!pia.timer_val)
                    pia.reached_zero = 1;
                else {                
                    pia.timer_val--;
                    pia.interval_left += pia.interval_clocks - clocks;
                }
            }
            else
                pia.interval_left -= clocks;
        }
#ifdef TRACE_PIA
        printf("val %02X, left %d\n", pia.timer_val, pia.interval_left);
#endif
    }
    if (pia.reset_clocks > 0) {
        pia.timer_val = pia.reset_val;
        pia.interval_clocks = pia.reset_clocks;
        pia.interval_left = 0;
        pia.reset_val = 0;
        pia.reset_clocks = 0;
#ifdef TRACE_PIA
        printf("PIA: reset clocks %d, int %d, val %02X, left %d\n",
               clocks, pia.interval_clocks, pia.timer_val, pia.interval_left);
#endif
    }
}

void pia_pa_clear() {
    pia.pa.val = 0xFF;
}

void pia_pa_set(uint8_t dir, uint8_t is_p0) {
    pia.pa.val &= ~(1 << (dir + (is_p0 ? 4 : 0)));
}

void pia_pb_clear() {
    pia.pb.val = 0b00001011;        // both amateur, color, nothing pressed   
}

void pia_reset() {
    pia.pb.bits.reset = 0;
}

void pia_select() {
    pia.pb.bits.select = 0;
}