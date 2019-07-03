#ifndef	__BADGE_H
#define	__BADGE_H

#include <stdint.h>
#include "tune_player.h"
#include "splash.h"
#include "tetrapuzz.h"
#include "post.h"
#include "badge_settings.h"
#include "snake.h"
#include "user_program.h"
#include "vt100.h"

#if ENABLE_Z80
#include "Z80/hwz.h"
#endif
#include "hw.h"
#include "disp.h"
#include "xonix.h"
#include <setjmp.h>


const char* get_firmware_string(void);

uint16_t get_user_value (void);
void display_refresh_force (void);
void clr_buffer (void);


/***************************************************************************************************/

#endif
