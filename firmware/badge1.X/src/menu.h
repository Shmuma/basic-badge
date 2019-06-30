#ifndef MENU_H
#define	MENU_H

#include <stdint.h>
#include "disp.h"

struct menu_t {
    uint16_t id;
    const char* title;
    const uint8_t items_count;
    const struct menu_t* items;
};

uint16_t menu_run(const struct menu_t* menu);

#define MENU_FRAME_FG	EGA_BRED
#define MENU_FRAME_BG	EGA_BLACK
#define MENU_TITLE_FG	EGA_BLACK
#define MENU_TITLE_BG	EGA_WHITE
#define MENU_HEADER_FG	EGA_WHITE
#define MENU_HEADER_BG	EGA_DGRAY

#define MENU_ITEM_FG	EGA_WHITE
#define MENU_ITEM_BG	EGA_BBLUE
#define MENU_ACTIVE_FG	EGA_BLACK
#define MENU_ACTIVE_BG	EGA_BCYAN

#define MENU_WIDTH      32

#endif	/* MENU_H */

