#include "atari.h"

struct pia_state pia;


void init_pia() {
    pia_pb_clear();
    pia.timer_val = 0;
    pia.interval_clocks = 0;
    pia.interval_left = 0;
}


void poke_pia(uint16_t address, uint8_t val) {
    if (address == TIM1T) {
        pia.timer_val = val;
        pia.interval_clocks = 1;
        pia.interval_left = 0;
    }
    else if (address == TIM8T) {
        pia.timer_val = val;
        pia.interval_clocks = 1 << 3;
        pia.interval_left = 0;
    }
    else if (address == TIM64T) {
        pia.timer_val = val;
        pia.interval_clocks = 1 << 6;
        pia.interval_left = 0;
    }
    else if (address == T1024T) {
        pia.timer_val = val;
        pia.interval_clocks = (uint32_t)(1 << 10);
        pia.interval_left = 0;
    }
    else if (address == SWACNT) {
        pia.pa_dir = val;
    }
}

uint8_t peek_pia(uint16_t address) {
    if (address == SWCHA)
        return pia.pa.val & pia.pa_dir;
    else if (address == SWCHB)
        return pia.pb.val;
    else if (address == INTIM)
        return pia.timer_val;
    return 0xFF;
}


void mpu_clock_pia() {
    if (!pia.interval_clocks)
        return;
    if (pia.interval_clocks == 1)
        pia.timer_val--;
    else {
        if (pia.interval_left == 0) {
            pia.timer_val--;
            pia.interval_left = pia.interval_clocks;
        }
        else
            pia.interval_left--;
    }
}

void pia_pa_clear() {
    pia.pa.val = 0;
}

void pia_pa_set(uint8_t dir, uint8_t is_p0) {
    pia.pa.val |= (1 << (dir + (is_p0 ? 4 : 0)));
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