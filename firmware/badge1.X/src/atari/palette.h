#ifndef PALETTE_H
#define	PALETTE_H

#define COL_NONE    255

extern const uint32_t ntsc[];

#define COLOR_NTSC(v) (ntsc[(v)>>1])

#endif	/* PALETTE_H */

