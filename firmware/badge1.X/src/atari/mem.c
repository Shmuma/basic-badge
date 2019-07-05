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
    uint8_t res;
    
    if (address < RAM_ADDR) {
        res = peek_tia(address);
#ifdef TRACE_MEM
        printf("peek tia: %02X -> %02X\n", address, res);
#endif
        return res;
    }

    if (address >= RAM2_ADDR && address <= RAM2_ENDS)
        address -= RAM2_OFS;
  
    if (address <= RAM_ENDS) {
#ifdef TRACE_MEM
        printf("peek mem: %02X -> %02X\n", address, memory[address - RAM_ADDR]);
#endif
        return memory[address - RAM_ADDR];
    }

    if (address >= PIA_START && address <= PIA_END) {
        res = peek_pia(address);
#ifdef TRACE_MEM
        printf("peek pia: %04X -> %02X\n", address, res);
#endif        
        return res;
    }
  
    if (address >= ROM_ADDR) {
        address -= ROM_ADDR;
        address %= rom_size;
#ifdef TRACE_MEM
        printf("peek rom: %02X -> %02X\n", address + ROM_ADDR, rom[address]);
#endif
        return rom[address];
    }
    
#ifdef TRACE_MEM
    printf("peek at wrong address: %04X\n", address);
#endif   
    
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
#endif    
        memory[address - RAM_ADDR] = value;
        return;
    }
    
    if (address >= PIA_START && address <= PIA_END) {
#ifdef TRACE_MEM
        printf("poke pia: %04X <- %02X\n", address, value);
#endif        
        poke_pia(address, value);
        return;
    }

    
#ifdef TRACE_MEM
    printf("poke at wrong address: %04X <- %02X\n", address, value);
#endif   
}

// read reset vector from memory
uint16_t reset_vector() {
    return peek(0xFFFC) | (peek(0xFFFD) << 8);
}

// read interrupt vector from memory
uint16_t interrupt_vector() {
    return peek(0xFFFE) | (peek(0xFFFF) << 8);
}
