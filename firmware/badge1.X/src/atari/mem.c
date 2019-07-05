#include "atari.h"

uint8_t memory[128];

#ifdef ATARI_POSIX
const uint8_t *rom = NULL;
#else
uint8_t rom_data[ROM_SIZE];
const uint8_t* rom = rom_data;
#endif

long rom_size = ROM_SIZE;

#define RAM_ADDR    0x0080
#define RAM_ENDS    0x00FF
// mirror of RAM
#define RAM2_OFS    0x0100
#define RAM2_ADDR   (RAM2_OFS + RAM_ADDR)
#define RAM2_ENDS   (RAM2_OFS + RAM_ENDS)

#define ROM_ADDR (0xF000)

uint8_t
peek(uint16_t address)
{
  if (address < RAM_ADDR) {
#ifdef TRACE_MEM
    printf("peek tia: %02X\n", address);
#endif
    return peek_tia(address);
  }

  if (address >= RAM2_ADDR && address <= RAM2_ENDS)
      address -= RAM2_OFS;
  
  if (address <= RAM_ENDS) {
#ifdef TRACE_MEM
    printf("peek mem: %02X -> %02X\n", address, memory[address - RAM_ADDR]);
#endif
    return memory[address - RAM_ADDR];
  }

  // TODO: need to handle RIOT address range
  
  address -= ROM_ADDR;
  if (address < rom_size) {
#ifdef TRACE_MEM
    printf("peek rom: %02X -> %02X\n", address + ROM_ADDR, rom[address]);
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
        printf("poke tia: %02X <- %02X\n", address, value);
#endif
        poke_tia(address, value);
        return; 
    }
  
    if (address >= RAM2_ADDR && address <= RAM2_ENDS)
        address -= RAM2_OFS;

    if (address <= RAM_ENDS) {
#ifdef TRACE_MEM
        printf("poke mem: %x <- %x\n", address, value);
        if (address == 0x80 && value == 1)
            printf("Break!\n");
#endif    
        memory[address - RAM_ADDR] = value;
        return;
    }
}

// read reset vector from memory
uint16_t reset_vector() {
    return peek(0xFFFC) | (peek(0xFFFD) << 8);
}
