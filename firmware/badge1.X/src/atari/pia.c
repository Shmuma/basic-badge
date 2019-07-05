#include "atari.h"

struct pia_state pia;


void init_pia() {
    pia.pb.val = 0b00001011;        // both amateur, color, nothing pressed
}


void poke_pia(uint16_t address, uint8_t val) {

}

uint8_t peek_pia(uint16_t address) {
    if (address == SWCHB)
        return pia.pb.val;
    return 0xFF;
}
