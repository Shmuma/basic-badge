#ifndef _TIA_H
#define _TIA_H

// Working with NTSC standard
#define FB_WIDTH 	160
#define FB_HEIGHT	192

#define CLK_HORBLANK 	68
#define CLK_HOR         (CLK_HORBLANK + FB_WIDTH)

// VSYNC is not counted in scanlines
#define SCN_VSYNC       3   
#define SCN_VBLANK      37
#define SCN_OVERSCAN	30
#define SCN_VIS_START   SCN_VBLANK
#define SCN_VIS_END     (SCN_VIS_START + FB_HEIGHT)
#define SCN_VERT        (SCN_VIS_END + SCN_OVERSCAN)


struct tia_state {
  uint16_t scanline;
  uint8_t color_clock;

  uint8_t colu[4]; 	// P0, P1, PF, BK
  
  uint8_t fb[FB_WIDTH];
};

#define VSYNC       0
#define WSYNC 		2
#define COLUP0		6
#define COLUP1		7
#define COLUPF		8
#define COLUBK 		9


void init_tia();
void tia_mpu_cycles(uint8_t cycles);
void poke_tia(uint16_t, uint8_t);
uint8_t peek_tia(uint16_t);

INLINE void do_vsync(uint8_t start);
INLINE void do_wsync();                 // draw the next of the line
void draw_pixels(uint8_t count);        // draw given amount of pixels
void tia_line_ready(uint8_t line);

#endif
