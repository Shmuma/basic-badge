#ifndef _TIA_H
#define _TIA_H

// Working with NTSC standard
#define FB_WIDTH 	160
#define FB_HEIGHT   224
//#define FB_HEIGHT	(192+17)

#define CLK_HORBLANK 	68
#define CLK_HOR         (CLK_HORBLANK + FB_WIDTH)

// VSYNC is not counted in scanlines
//#define SCN_VSYNC       3   
//#define SCN_VBLANK      37
//#define SCN_OVERSCAN	30
//#define SCN_VIS_START   SCN_VBLANK
//#define SCN_VIS_END     (SCN_VIS_START + FB_HEIGHT)
//#define SCN_VERT        (SCN_VIS_END + SCN_OVERSCAN)

#define PF_RIGHT        80

struct tia_state {
  uint8_t draw_enabled, vsync_enabled;
  uint8_t queue_addr, queue_val;
  int16_t scanline;
  uint8_t color_clock;
  uint8_t p0_pos, p1_pos, bl_pos;    // position of objects (as offset from visible)
  uint8_t p0_mask;              // mask of p0 to be drawn
  uint8_t p0_mask_cnt, p0_mask_clocks;       // how frequently we shift the mask, driven by NUSIZP0
  uint8_t p1_mask;              // mask of p1 to be drawn
  uint8_t p1_mask_cnt, p1_mask_clocks;       // how frequently we shift the mask, driven by NUSIZP1
  uint8_t bl_clocks;                        // how many clocks of the ball to draw
  
  union {
      uint8_t val;
      struct {
          uint8_t p0:1;
          uint8_t p1:1;
      }bits;
  } fire;                       // fire button states, 0 - depressed, 1 - pressed
  
  uint8_t fire_buttons;         // zero and first bits
  
  uint8_t colu[4];          // P0, P1, PF, BK
  union {
    struct {
        uint8_t pf_ref:1;         // reflect playfield
        uint8_t pf_score:1;       // colorize playfields by player
        uint8_t pf_prio:1;        // playfield is higher on prio than player
        uint8_t :1;
        uint8_t ballsize:2;       // size of the ball
    } bits;
    uint8_t val;
  } ctrlpf;

  uint32_t pf;                      // pf register combined as pf0, pf1, pf2 
                                    // in LSB (left-to right order)
  uint8_t p0, p1;                   // graphics for P0 and P1
  uint8_t ref_p0, ref_p1;           // reflect players
  int8_t hmp0, hmp1, hmm0, hmm1, hmbl;  // horizontal move registers (decoded into signed val)
  
  union {
      struct {
          uint8_t psize_count:3;          // player size and counts
          uint8_t :1;
          uint8_t msize:2;                // missile size
      } bits;
      uint8_t val;
  } nusiz0;

  union {
      struct {
          uint8_t psize_count:3;          // player size and counts
          uint8_t :1;
          uint8_t msize:2;                // missile size
      } bits;
      uint8_t val;
  } nusiz1;
  
  uint8_t enam0, enam1, enabl;
  
  uint8_t fb[FB_WIDTH];
};

// Write ports
#define VSYNC       0x00
#define VBLANK      0x01
#define WSYNC       0x02
#define NUSIZ0      0x04
#define NUSIZ1      0x05
#define COLUP0      0x06
#define COLUP1      0x07
#define COLUPF      0x08
#define COLUBK      0x09
#define CTRLPF      0x0A
#define REFP0       0x0B
#define REFP1       0x0C
#define PF0         0x0D
#define PF1         0x0E
#define PF2         0x0F
#define RESP0       0x10
#define RESP1       0x11
#define RESM0       0x12
#define RESM1       0x13
#define RESBL       0x14
#define GRP0        0x1B
#define GRP1        0x1C
#define ENAM0       0x1D
#define ENAM1       0x1E
#define ENABL       0x1F
#define HMP0        0x20
#define HMP1        0x21
#define HMM0        0x22
#define HMM1        0x23
#define HMBL        0x24
#define HMOVE       0x2A
#define HMCLR       0x2B

// two copies, 8 clocks space
#define NUSIZ_TWO_8     0b001
#define NUSIZ_TWO_24    0b010
#define NUSIZ_THREE_8   0b011
#define NUSIZ_TWO_56    0b100
#define NUSIZ_THREE_24  0b110

#define NUSIZ_SINGLE    0b000
#define NUSIZ_DOUBLE    0b101
#define NUSIZ_QUAD      0b111

// Read ports
#define TIA_RD_INPT4    0x0C
#define TIA_RD_INPT5    0x0D


void init_tia();
void tia_mpu_cycles(uint8_t cycles);
void poke_tia(uint16_t, uint8_t);
uint8_t peek_tia(uint16_t);

void tia_line_ready(uint8_t line);

void tia_fire(uint8_t p0, uint8_t set);

#endif
