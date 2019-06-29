#ifndef __MEM_H
#define __MEM_H

uint8_t peek(uint16_t);
void poke(uint16_t, uint8_t);

uint16_t reset_vector();

#ifdef ATARI_POSIX
int read_rom(const char* file_name);
#endif

#endif
