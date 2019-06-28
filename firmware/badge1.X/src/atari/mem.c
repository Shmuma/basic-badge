#include "atari.h"

uint8_t memory[128];
const uint8_t *rom = NULL;
long rom_size = 0;

#define RAM_ADDR (0x0080)
#define RAM_ENDS (0x00FF)
#define ROM_ADDR (0xF000)

uint8_t
peek(uint16_t address)
{
  if (address < RAM_ADDR) {
#ifdef TRACE_MEM
    printf("peek tia: %x\n", address);
#endif
    return peek_tia(address);
  }
  
  if (address <= RAM_ENDS) {
#ifdef TRACE_MEM
    printf("peek mem: %x -> %x\n", address, memory[address]);
#endif
    return memory[address - RAM_ADDR];
  }

  // TODO: need to handle RIOT address range
  
  address -= ROM_ADDR;
  if (address < rom_size) {
#ifdef TRACE_MEM
    printf("peek rom: %x -> %x\n", address + ROM_ADDR, memory[address]);
#endif
    return rom[address];
  }

  return 0xFF;
}

void
poke(uint16_t address, uint8_t value)
{
  static uint8_t x = 0, y = 0;

  if (address < RAM_ADDR) {
#ifdef TRACE_MEM
    printf("poke tia: %x <- %x\n", address, value);
#endif
    poke_tia(address, value);
    return; 
  }
  
  if (address <= RAM_ENDS) {
#ifdef TRACE_MEM
    printf("poke mem: %x <- %x\n", address, value);
#endif    
    memory[address - RAM_ADDR] = value;
    return;
  }
	/*
	if (address == 0xF001) {
                putcharacter(value);
                return;
        }
        if (address == 0xF005) {
                char str[SLEN];
                snprintf(str, SLEN, "\033[%d;%dH", x = value, y);
                write(STDOUT_FILENO, str, strlen(str));
                return;
        }
        if (address == 0xF006) {
                char str[SLEN];
                snprintf(str, SLEN, "\033[%d;%dH", x, y = value);
                write(STDOUT_FILENO, str, strlen(str));
                return;
        }
	*/
}


#ifdef ATARI_POSIX
int read_rom(const char* file_name) {
  FILE* f = fopen(file_name, "rb");
  
  if (!f)
    return 0;

  fseek(f, 0, SEEK_END);
  rom_size = ftell(f);
  fseek(f, 0, SEEK_SET);
  rom = malloc(rom_size);
  fread((uint8_t*)rom, rom_size, 1, f);
  fclose(f);
  return 1;
}
#endif


void set_rom(const uint8_t* data, uint16_t size) {
    rom = data;
    rom_size = size;
}