#include <xc.h>
#include <plib.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include "hw.h"
#include "vt100.h"
#include "menu.h"
#include "post.h"
#include "disp.h"

uint16_t i;
const int8_t post_char_table[4*11] = "1234567890=qwertyuiop;/asdfghjkl\n\0zxcvbnm,.\0";
extern uint8_t handle_display;

void post (void)
	{
	uint8_t retval,index,line,position,color;
	uint32_t flash_id;
	int8_t temp_string[30];
	handle_display = 0;
	set_led(0,1);
	set_led(1,1);
	set_led(2,1);	
	wait_ms(400);
	tft_fill_area(0,0,320,240,0xFF0000);
	wait_ms(1000);
	tft_fill_area(0,0,320,240,0x00FF00);
	wait_ms(1000);
	tft_fill_area(0,0,320,240,0x0000FF);
	wait_ms(1000);
	handle_display = 1;
	term_init();
	set_led(0,0);
	set_led(1,0);
	set_led(2,0);	
	while (1)
		{
		flash_id = fl_rdid();
		if (flash_id==0xC22315)
			video_set_color(EGA_GREEN,EGA_BLACK);
		else
			video_set_color(EGA_RED,EGA_BLACK);
		video_gotoxy(0,15);
		sprintf(temp_string,"FLASH ID is %x\n",flash_id);
		stdio_write(temp_string);
		video_set_color(EGA_WHITE,EGA_BLACK);
		stdio_write("Press LCTRL+ENTER to continue\n");
		wait_ms(200);
		video_set_color(11,1);
		video_gotoxy(1,1);
		stdio_write("u ");
		video_gotoxy(24,1);
		stdio_write("p ");
		video_gotoxy(0,2);
		stdio_write("l r ");
		video_gotoxy(22,2);
		stdio_write("b b ");
		video_gotoxy(1,3);
		stdio_write("d ");
		video_gotoxy(22,3);
		stdio_write("_ d ");
		video_gotoxy(1,4);
		stdio_write("1 2 3 4 5 6 7 9 8 0 = ");
		video_gotoxy(1,5);
		stdio_write("Q W E R T Y U I O P ; ");
		video_gotoxy(1,6);
		stdio_write("? A S D F G H J K L e ");
		video_gotoxy(1,7);
		stdio_write("s Z X C V B N M < > s ");
		if (KEY_BRK==0)
			{
			video_gotoxy(22,2);
			video_set_color(11,4);
			stdio_write("  ");						
			}					
		if (K_SHIFTR==0)
			{
			video_gotoxy(1,7);
			video_set_color(11,4);
			stdio_write("  ");						
			}					
		if (K_SHIFTL==0)
			{
			video_gotoxy(21,7);
			video_set_color(11,4);
			stdio_write("  ");						
			}	
		if (stdio_get(&retval)!=0)
			{
			if ((retval>=' ')&(retval<=0x7F))
				{
				index = 255;
				for (i=0;i<44;i++)
					{
					if (post_char_table[i]==retval)
						index = i;
					}
				if (index<255)
					{
					line = index / 11;
					position = index % 11;
					video_gotoxy (1+(2*position),4+line);
					video_set_color(11,4);
					stdio_write("  ");
					}
				else
					{
					if (retval=='_')
						{
						video_gotoxy(22,3);
						video_set_color(11,4);
						stdio_write("  ");						
						}
					}
				}
			else
				{
				if (retval==K_UP) video_gotoxy(1,1);
				if (retval==K_DN) video_gotoxy(1,3);
				if (retval==K_LT) video_gotoxy(0,2);
				if (retval==K_RT) video_gotoxy(2,2);
				if (retval==K_DEL) video_gotoxy(24,3);
				if (retval==NEWLINE) video_gotoxy(21,6);
				if (retval == K_ECR) break;
				video_set_color(11,4);
				stdio_write("  ");						
				}
			}
		}
	video_clrscr();
	term_init();
	color=0;
	while (1)
		{
		video_gotoxy(0,2);
		if (color==3) color = 0;
		if (color==0)
			{
			set_led(0,1);
			set_led(1,0);
			set_led(2,0);
			stdio_write("red LED  \n");
			stdio_write("generator 1, playing F4, ~349Hz  \n");
			wait_ms(600);
			sound_play_notes(65,0,0,200);
			}
		if (color==1)
			{
			set_led(0,0);
			set_led(1,1);
			set_led(2,0);
			stdio_write("green LED  \n");
			stdio_write("generator 2, playing A4, 440Hz  \n");
			wait_ms(600);
			sound_play_notes(0,69,0,200);
			}
		if (color==2)
			{
			set_led(0,0);
			set_led(1,0);
			set_led(2,1);
			stdio_write("blue LED  \n");
			stdio_write("generator 3, playing C5, ~523Hz  \n");
			wait_ms(600);
			sound_play_notes(0,0,72,200);
			}
		video_set_color(EGA_WHITE,EGA_BLACK);
		stdio_write("Press LCTRL+ENTER to continue\n");
		if (stdio_get(&retval)!=0)
			{
			if (retval == K_ECR) break;
			}
		color++;
		}
	set_led(0,0);
	set_led(1,0);
	set_led(2,0);
	video_clrscr();
	term_init();
	stdio_write("Reset badge now\n");
	while(1);
	}
