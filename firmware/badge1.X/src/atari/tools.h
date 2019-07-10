#ifndef TOOLS_H
#define	TOOLS_H

static inline uint8_t invert_bits_byte(uint8_t x) {
    x = (x & 0x55) <<  1 | (x & 0xAA) >>  1;
    x = (x & 0x33) <<  2 | (x & 0xCC) >>  2;
    x = (x & 0x0F) <<  4 | (x & 0xF0) >>  4;
    return x;
}


// convert 4 bits 2-complement to signed
#define FOURBITS_2COMPL_TO_INT(x) (-((x) & (1<<3)) + ((x) & 0b111))

#endif	/* TOOLS_H */

