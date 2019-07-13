#ifndef PIA_H
#define	PIA_H

struct pia_state {
    // those values are non-zeros in case of scheduled PIA write
    uint8_t reset_val;
    uint16_t reset_clocks;
    
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
    
    uint8_t pa_dir;                     // port A direction bits
    union {
        uint8_t val;
        struct {
            uint8_t p1_up:1;
            uint8_t p1_dn:1;
            uint8_t p1_lt:1;
            uint8_t p1_rt:1;
            uint8_t p0_up:1;
            uint8_t p0_dn:1;
            uint8_t p0_lt:1;
            uint8_t p0_rt:1;
        } bits;
    } pa;
    
    uint8_t     timer_val;              // count of intervals left in timer
    uint16_t    interval_clocks;        // size of interval set
    uint8_t     interval_left;          // counter of interval in MPU cycles
};

// PIA addresses
#define SWCHA   0x0280          // Port A IO
#define SWACNT  0x0281          // Port A directions
#define SWCHB   0x0282          // Console switches
#define SWBCNT  0x0283          // Port B directions -- hardwired to output
#define INTIM   0x0284          // Timer output
#define TIM1T   0x0294          // set 1 clock interval
#define TIM8T   0x0295          // set 8 clock interval
#define TIM64T  0x0296          // set 64 clock interval
#define T1024T  0x0297          // set 1024 clock interval

#define PIA_START   SWCHA
#define PIA_END     T1024T

void init_pia();
void poke_pia(uint16_t, uint8_t);
uint8_t peek_pia(uint16_t);
void mpu_clock_pia(uint8_t);
void pia_pa_clear();
void pia_pa_set(uint8_t dir, uint8_t is_p0);
void pia_pb_clear();
void pia_reset();
void pia_select();

#define DIR_UP      0
#define DIR_DN      1
#define DIR_LT      2
#define DIR_RT      3

#endif	/* PIA_H */

