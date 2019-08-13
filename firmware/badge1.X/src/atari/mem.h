#ifndef __MEM_H
#define __MEM_H

uint8_t peek(uint16_t);
uint16_t peek2(uint16_t);
void poke(uint16_t, uint8_t);

uint16_t reset_vector();
uint16_t interrupt_vector();

#ifdef ATARI_POSIX
int read_rom(const char* file_name);
#else
extern uint8_t rom_data[];
#endif

extern const uint8_t* rom;
extern long rom_size;

#endif
