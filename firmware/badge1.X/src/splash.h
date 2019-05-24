#ifndef		__SLPASH_H
#define		__SLPASH_H

/*
 * Graphic from poster located here:
 * https://cdn.hackaday.io/images/5980891518077065617.png
 * 
 * Napkin calculations:
 *  Original spacing is 347px letters with 32px spaces
 *  Combining character with spaces calls for 8.44% space
 *  240px screen, 4 characters
 *  Characters should be 54px (18px subchars each) with 5px spacing and 3px on top
 */

#include <stdint.h>

#define CIPHER_X0           42
#define CIPHER_Y0           2
#define CIPHER_CHAR_WIDTH   18
#define CIPHER_SPACE        5
#define CIPHER_COLOR        0xEE3530
#define CIPHER_BACKGROUND   0x000000
#define CIPHER_CURSOR       0xFFFFFF

void show_splash(void);
void draw_vert_line(uint16_t x, uint8_t y, uint8_t height, uint32_t color);
void animate_splash(void);
uint8_t overlaps_logo(int16_t x, uint8_t row);

#endif
