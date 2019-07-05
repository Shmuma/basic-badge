#include "atari.h"

struct pia_state pia;


void init_pia() {
    pia.pb.val = 0b00001011;        // both amateur, color, nothing pressed
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
}

uint8_t peek_pia(uint16_t address) {
    if (address == SWCHB)
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


// TODO: methods to change switches of port B