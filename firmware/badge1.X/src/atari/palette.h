#ifndef PALETTE_H
#define	PALETTE_H

extern uint32_t* ntsc;

#define COLOR_NTSC(v) (ntsc[(v)>>1])

#endif	/* PALETTE_H */

