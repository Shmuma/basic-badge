#ifndef PIA_H
#define	PIA_H

struct pia_state {
    union {
        uint8_t val;
        struct {
            uint8_t reset:1;            // game reset, 0 if pressed
            uint8_t select:1;           // game select, 0 if pressed
            uint8_t unused_1:1;
            uint8_t color_bw:1;         // 1 - color, 0 - bw
            uint8_t unused_2:2;
            uint8_t p0_diff:1;          // 0 - amateur, 1 - pro
            uint8_t p1_diff:1;          // 0 - amateur, 1 - pro
        } bits;
    } pb;
};

// PIA addresses
#define SWCHA   0x0280          // Port A IO
#define SWACNT  0x0281          // Port A directions
#define SWCHB   0x0282          // Console switches
#define SWBCNT  0x0283          // Port B directions -- hardwired to output
#define INTIM   0x0284          // Timer output
#define TIM1T   0x0294          // set 1 clock interval
#define TIM8T   0x0295          // set 1 clock interval
#define TIM64T  0x0296          // set 1 clock interval
#define T1024T  0x0297          // set 1 clock interval

#define PIA_START   SWCHA
#define PIA_END     T1024T

void init_pia();
void poke_pia(uint16_t, uint8_t);
uint8_t peek_pia(uint16_t);


#endif	/* PIA_H */

