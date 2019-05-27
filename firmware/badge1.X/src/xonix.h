#ifndef __BADGE_H
#define __BADGE_H

#include "disp.h"

#define CELL_SIZE           3             // how large one item of the field
#define FIELD_WIDTH         (TFT_WIDTH/CELL_SIZE)
#define FIELD_HEIGHT        (TFT_HEIGHT/CELL_SIZE)
#define FIELD_BYTES         ((FIELD_WIDTH+7)/8)

void xonix_main(void);
void xonix_step(void);

void xonix_init(void);
void xonix_init_field(void);
void xonix_draw_field(void);

#endif
