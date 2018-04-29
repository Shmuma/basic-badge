#include "hw.h"
#include "tetrapuzz.h"
#include "box_game.h"

extern volatile unsigned long ticks;

//FIXME: these should probably not be globals
unsigned long wait_until;
unsigned char drop_timer_flag = 0;
unsigned int state;

char sstr[3];

void tetrapuzz(void)
	{
	tetrapuzz_init();
	
	while(1)
		{
		
		//This is just a rolling increment kind of fake random
		//because it gets called more often than the boxes are dropped
		BOX_inc_random();
		
		//Service button inputs as necessary
		unsigned char get_stat = stdio_get(sstr);
		if (get_stat!=0)
			{
			if (sstr[0]=='w') {	BOX_rotate(1);	}
			if (sstr[0]=='a') {	BOX_lt();	}
			if (sstr[0]=='d') {	BOX_rt();	}
			if (sstr[0]=='s') {	BOX_dn();	}
			}

		//Service the loop on a non-blocking delay here
		if (ticks > wait_until)
			{
			wait_until = ticks+BOX_get_delay();
			tetrapuzz_loop();
			}
		}
	}
 
void tetrapuzz_init(void)
	{
	//Pull TMR1 value for a bit of not-really-but-kinda-random number
	int timer1val = *(char*)0xBF800610;
	BOX_seed_random((unsigned char) timer1val&0xF);

	BOX_clearscreen();
	BOX_pregame();
	drop_timer_flag = 0;
	wait_until = ticks;
	BOX_start_game();
	}

void tetrapuzz_loop(void)
	{
	BOX_dn();
	
	if (BOX_end_game())
		{
		//Print game ending information
		BOX_show_gameover();
		//Loop until a button is pushed
		while (stdio_get(sstr) == 0) {;;}
		//Start game over
		tetrapuzz_init();
		}
	}

void tetrapuzz_pause(void)
	{
	//FIXME: we probably don't need a pause mode.
	/* deprecated camera badge code: 
	if(butpress)
		{
		BOX_start_game();
		state=s_run;
		return(0);
		}
	*/
	}

void tetrapuzz_gameover(void)
	{
	//FIXME: we probably don't need this gameover mode.
	/* deprecated camera badge code:
	}
	if(butpress) {
		BOX_start_game();
		state=s_run;
	}
	*/
	}