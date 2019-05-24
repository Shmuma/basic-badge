#include "tune_player.h"


const unsigned char mario_array_limits[9] = { 48, 52, 52, 56, 52, 56, 64, 64, 64 };
const unsigned int mario_tempos[4] = {149, 198, 298, 447 };

const unsigned char mario_main0[48] = 
	{
		50,66,76,0,
		50,66,76,0,
		0,0,0,0,
		50,66,76,0,
		0,0,0,0,
		50,66,72,0,
		50,66,76,0,
		0,0,0,0,
		67,71,79,0,
		0,0,0,3,
		0,55,67,0,
		0,0,0,3
	};

const unsigned char mario_main1[52] = 
	{
		48,0,0,0,
		0,0,0,0,
		56,68,75,0,
		0,0,0,2,
		58,65,74,0,
		0,0,0,2,
		60,64,72,0,
		0,0,0,2,
		55,0,0,0,
		55,0,0,0,
		0,0,0,0,
		48,0,0,0,
		0,0,0,0
	};

const unsigned char mario_main2[52] = 
	{
		48,0,0,0,
		0,0,0,0,
		56,68,75,0,
		0,0,0,2,
		56,65,74,0,
		0,0,0,2,
		60,64,72,0,
		0,0,0,2,
		55,0,0,0,
		55,0,0,0,
		0,0,0,0,
		48,0,0,0,
		0,0,0,0
	};



const unsigned char mario_main3[56] = 
	{
		44,68,72,0,
		0,68,72,0,
		0,0,0,0,
		51,68,72,0,
		0,0,0,0,
		0,68,72,0,
		56,70,74,0,
		0,67,76,0,
		55,0,0,0,
		0,0,0,2,
		48,0,0,0,
		0,0,0,2,
		43,0,0,0,
		0,0,0,0
	};

const unsigned char mario_clip0[52] =
	{
		55,64,72,0,
		0,0,0,2,
		52,60,67,0,
		0,0,0,2,
		48,55,64,0,
		0,0,0,2,
		53,60,69,0,
		0,0,0,0,
		55,62,71,0,
		0,0,0,0,
		54,61,70,0,
		53,60,69,0,
		0,0,0,0
	};

const unsigned char mario_clip1[56] =
	{
		52,60,67,1,
		60,67,76,1,
		64,71,79,1,
		65,72,81,0,
		0,0,0,0,
		62,69,77,0,
		64,71,79,0,
		0,0,0,0,
		60,69,76,0,
		0,0,0,0,
		57,64,72,0,
		59,65,74,0,
		55,62,71,0,
		0,0,0,2
	};

const unsigned char mario_clip2[64] =
	{
		48,0,0,0,
		0,0,0,0,
		0,76,79,0,
		55,75,78,0,
		0,74,77,0,
		0,71,75,0,
		60,0,0,0,
		0,72,76,0,
		53,0,0,0,
		0,64,68,0,
		0,65,69,0,
		60,67,72,0,
		60,0,0,0,
		0,60,69,0,
		53,64,72,0,
		0,65,74,0
	};

const unsigned char mario_clip3[64] =
	{
		48,0,0,0,
		0,0,0,0,
		0,76,79,0,
		52,75,78,0,
		0,74,77,0,
		0,71,75,0,
		55,0,0,0,
		60,72,76,0,
		0,0,0,0,
		77,79,84,0,
		0,0,0,0,
		77,79,84,0,
		77,79,84,0,
		0,0,0,0,
		55,0,0,0,
		0,0,0,0
	};

const unsigned char mario_clip4[64] =
	{
		44,68,72,0,
		0,68,72,0,
		0,0,0,0,
		51,68,72,0,
		0,0,0,0,
		0,68,72,0,
		56,70,74,0,
		0,0,0,0,
		55,67,76,0,
		0,64,72,0,
		0,0,0,0,
		48,64,69,0,
		0,60,67,0,
		0,0,0,0,
		43,0,0,0,
		0,0,0,0
	};


void play_music_array(const unsigned char * arr, unsigned char length, const unsigned int * tempo_arr)
	{
	unsigned char i;
	for (i=0; i<(length/4); i++)
		{
		unsigned char index = i*4;
		sound_play_notes(*(arr+index), *(arr+index+1), *(arr+index+2), *(tempo_arr+*(arr+index+3)));
		}
	}

void play_mario_tune(void)
	{
	play_music_array(mario_main0,mario_array_limits[0],mario_tempos);
	
	play_music_array(mario_clip0,mario_array_limits[4],mario_tempos);
	play_music_array(mario_clip1,mario_array_limits[5],mario_tempos);
	play_music_array(mario_clip0,mario_array_limits[4],mario_tempos);
	play_music_array(mario_clip1,mario_array_limits[5],mario_tempos);
	play_music_array(mario_clip2,mario_array_limits[6],mario_tempos);
	play_music_array(mario_clip3,mario_array_limits[7],mario_tempos);
	play_music_array(mario_clip2,mario_array_limits[6],mario_tempos);
	
	play_music_array(mario_main1,mario_array_limits[1],mario_tempos);
	
	play_music_array(mario_clip2,mario_array_limits[6],mario_tempos);
	play_music_array(mario_clip3,mario_array_limits[7],mario_tempos);
	play_music_array(mario_clip2,mario_array_limits[6],mario_tempos);
	
	play_music_array(mario_main2,mario_array_limits[2],mario_tempos);
	play_music_array(mario_clip4,mario_array_limits[7],mario_tempos);
	play_music_array(mario_main3,mario_array_limits[3],mario_tempos);
	play_music_array(mario_clip4,mario_array_limits[7],mario_tempos);
	}
