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

// Playfield clock bounds
#define PF0_MAX_CLK     (4*4)
#define PF1_MAX_CLK     (PF0_MAX_CLK + 8*4)
#define PF2_MAX_CLK     (PF1_MAX_CLK + 8*4)
#define PF_RIGHT        80

struct tia_state {
  uint8_t draw_enabled, vsync_enabled;
  uint8_t queue_addr, queue_val;
  int16_t scanline;
  uint8_t color_clock;
  uint8_t p0_pos, p1_pos;       // position of P0 and P1
  uint8_t p0_mask;      // mask of p0 to be drawn
  uint8_t p1_mask;

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
  uint8_t pf0:4;
  uint8_t pf1, pf2;
  uint8_t p0, p1;                   // graphics for P0 and P1
  uint8_t ref_p0, ref_p1;           // reflect players
  
  uint8_t fb[FB_WIDTH];
};

#define VSYNC       0x00
#define VBLANK      0x01
#define WSYNC       0x02
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
#define GRP0        0x1B
#define GRP1        0x1C

void init_tia();
void tia_mpu_cycles(uint8_t cycles);
void poke_tia(uint16_t, uint8_t);
uint8_t peek_tia(uint16_t);

void tia_line_ready(uint8_t line);

#endif
