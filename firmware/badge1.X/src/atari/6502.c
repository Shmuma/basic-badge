/*
 * Copyright (C) 2015 Darron Broad, 2019 Max Lapan
 * All rights reserved.
 * 
 * This file is part of Ape65.
 * 
 * Ape65 is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation. 
 * 
 * Ape65 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. 
 * 
 * You should have received a copy of the GNU General Public License along
 * with Ape65. If not, see http://www.gnu.org/licenses/
 * 
 * Copied by Max Lapan into Hackaday Badge Atari Emulator.
 * 
 * Extension over original Ape65 version:
 * 1. Opcode timings
 * 2. extended with non-standard opcodes (TODO)
 */

/*
 * 6502 MPU
 */

#include "atari.h"

struct register_file reg = {0};

/*
 * Opcode [0..255] to Instruction [0..55]
 */
const uint8_t _instruction[256] = {
10, 34, FF, FF, FF, 34,  2, FF, 36, 34,  2, FF, FF, 34,  2, FF, // 000...015 00-0f
 9, 34, FF, FF, FF, 34,  2, FF, 13, 34, FF, FF, FF, 34,  2, FF, // 016...031 10-1f
28,  1, FF, FF,  6,  1, 39, FF, 38,  1, 39, FF,  6,  1, 39, FF, // 032...047 20-2f
 7,  1, FF, FF, FF,  1, 39, FF, 44,  1, FF, FF, FF,  1, 39, FF, // 048...063 30-3f
41, 23, FF, FF, FF, 23, 32, FF, 35, 23, 32, FF, 27, 23, 32, FF, // 064...079 40-4f
11, 23, FF, FF, FF, 23, 32, FF, 15, 23, FF, FF, FF, 23, 32, FF, // 080...095 50-5f
42,  0, FF, FF, FF,  0, 40, FF, 37,  0, 40, FF, 27,  0, 40, FF, // 096...111 60-6f
12,  0, FF, FF, FF,  0, 40, FF, 46,  0, FF, FF, FF,  0, 40, FF, // 112...127 70-7f
FF, 47, FF, FF, 49, 47, 48, FF, 22, FF, 53, FF, 49, 47, 48, FF, // 128...143 80-8f
 3, 47, FF, FF, 49, 47, 48, FF, 55, 47, 54, FF, FF, 47, FF, FF, // 144...159 90-9f
31, 29, 30, FF, 31, 29, 30, FF, 51, 29, 50, FF, 31, 29, 30, FF, // 160...175 a0-af
 4, 29, FF, FF, 31, 29, 30, FF, 16, 29, 52, FF, 31, 29, 30, FF, // 176...191 b0-bf
19, 17, FF, FF, 19, 17, 20, FF, 26, 17, 21, FF, 19, 17, 20, FF, // 192...207 c0-cf
 8, 17, FF, FF, FF, 17, 20, FF, 14, 17, FF, FF, FF, 17, 20, FF, // 208...223 d0-df
18, 43, FF, FF, 18, 43, 24, FF, 25, 43, 33, FF, 18, 43, 24, FF, // 224...239 e0-ef
 5, 43, FF, FF, FF, 43, 24, FF, 45, 43, FF, FF, FF, 43, 24, FF, // 240...255 f0-ff
};

/*
 * Opcode [0..255] to Addressing mode [0..12]
 */
const uint8_t _mode[256] = {
 0,  9, FF, FF, FF,  3,  3, FF,  0,  2,  1, FF, FF,  6,  6, FF, // 000...015 00-0f
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 016...031 10-1f
 6,  9, FF, FF,  3,  3,  3, FF,  0,  2,  1, FF,  6,  6,  6, FF, // 032...047 20-2f
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 048...063 30-3f
 0,  9, FF, FF, FF,  3,  3, FF,  0,  2,  1, FF,  6,  6,  6, FF, // 064...079 40-4f
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 080...095 50-5f
 0,  9, FF, FF, FF,  3,  3, FF,  0,  2,  1, FF, 12,  6,  6, FF, // 096...111 60-6f
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 112...127 70-7f
FF,  9, FF, FF,  3,  3,  3, FF,  0, FF,  0, FF,  6,  6,  6, FF, // 128...143 80-8f
11, 10, FF, FF,  4,  4,  5, FF,  0,  8,  0, FF, FF,  7, FF, FF, // 144...159 90-9f
 2,  9,  2, FF,  3,  3,  3, FF,  0,  2,  0, FF,  6,  6,  6, FF, // 160...175 a0-af
11, 10, FF, FF,  4,  4,  5, FF,  0,  8,  0, FF,  7,  7,  8, FF, // 176...191 b0-bf
 2,  9, FF, FF,  3,  3,  3, FF,  0,  2,  0, FF,  6,  6,  6, FF, // 192...207 c0-cf
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 208...223 d0-df
 2,  9, FF, FF,  3,  3,  3, FF,  0,  2,  0, FF,  6,  6,  6, FF, // 224...239 e0-ef
11, 10, FF, FF, FF,  4,  4, FF,  0,  8, FF, FF, FF,  7,  7, FF, // 240...255 f0-ff
};

/*
 * Opcode [0..255] to Machine Cycles. If value is negative, extra cycle for 
 * machine boundary has to be added
 */
const int8_t _cycles[256] = {
 7,  6, FF, FF, FF,  3,  5, FF,  3,  2,  2, FF, FF,  4,  6, FF, // 000...015 00-0f
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 016...031 10-1f
 6,  6, FF, FF,  3,  3,  5, FF,  4,  2,  2, FF,  4,  4,  6, FF, // 032...047 20-2f
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 048...063 30-3f
 6,  6, FF, FF, FF,  3,  5, FF,  3,  2,  2, FF,  3,  4,  6, FF, // 064...079 40-4f
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 080...095 50-5f
 6,  6, FF, FF, FF,  3,  5, FF,  4,  2,  2, FF,  5,  4,  6, FF, // 096...111 60-6f
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 112...127 70-7f
FF,  6, FF, FF,  3,  3,  3, FF,  2, FF,  2, FF,  4,  4,  4, FF, // 128...143 80-8f
 2,  6, FF, FF,  4,  4,  4, FF,  2,  5,  2, FF, FF,  4, FF, FF, // 144...159 90-9f
 2,  6,  2, FF,  3,  3,  3, FF,  2,  2,  2, FF,  4,  4,  4, FF, // 160...175 a0-af
 2, -5, FF, FF,  4,  4,  4, FF,  2, -4,  2, FF, -4, -4, -4, FF, // 176...191 b0-bf
 2,  6, FF, FF,  3,  3,  5, FF,  2,  2,  2, FF,  4,  4,  6, FF, // 192...207 c0-cf
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 208...223 d0-df
 2,  6, FF, FF,  3,  3,  5, FF,  2,  2,  2, FF,  4,  4,  6, FF, // 224...239 e0-ef
 2, -5, FF, FF, FF,  4,  6, FF,  2, -4, FF, FF, FF, -4,  7, FF, // 240...255 f0-ff
};

// equals 1 if last call to address() crossed page boundary
uint8_t _address_page_crossed = 0;

/*
 * Push to stack
 */
static inline void
push(uint8_t data)
{
	poke(0x0100 + reg.SP, data);
	reg.SP--;
}

/*
 * Pull from stack
 */
static inline uint8_t
pull(void)
{
	++reg.SP;
	return peek(reg.SP + 0x0100);
}

#if 0
/*
 * Non-maskable interrupt request pin
 */
void
_NMI(void)
{
	uint16_t v = 65530;

	push(reg.PC >> 8);
	push(reg.PC & 0xFF);
	push(reg.SR);
	reg.SR |= INTERRUPT;

	reg.PC = peek(v++);
	reg.PC |= peek(v) << 8;
}

/*
 * Interrupt request pin
 */
void
_IRQ(void)
{
	uint16_t v = 65534;

	push(reg.PC >> 8);
	push(reg.PC & 0xFF);
	push(reg.SR);
	reg.SR |= INTERRUPT;

	reg.PC = peek(v++);
	reg.PC |= peek(v) << 8;
}
#endif

/*
 * Get operand address for mode
 */
static uint16_t
address(uint8_t mode)
{
	uint16_t addr = 0; /* Address */
    _address_page_crossed = 0;
    
	switch (mode) {

	case IMPLIED:
		break;

	case ACCUMULATOR:
		break;

	case IMMEDIATE:
		addr = reg.PC++;
		break;

	case ZEROPAGE:
		addr = peek(reg.PC++);
		break;

	case ZEROPAGEX:
		addr = (peek(reg.PC++) + reg.XR) & 0xFF;
		break;

	case ZEROPAGEY:
		addr = (peek(reg.PC++) + reg.YR) & 0xFF;
		break;

	case ABSOLUTE:
        addr = peek2(reg.PC);
        reg.PC += 2;
		break;

	case ABSOLUTEX:
        addr = peek2(reg.PC);
        reg.PC += 2;
		addr += reg.XR;
        _address_page_crossed = ((addr & 0xFF) < reg.XR);
		break;

	case ABSOLUTEY:
        addr = peek2(reg.PC);
        reg.PC += 2;
		addr += reg.YR;
        _address_page_crossed = ((addr & 0xFF) < reg.YR);
		break;

	case INDIRECTX:
		{
		uint16_t i;

		i = (reg.XR + peek(reg.PC++)) & 0xFF;
        addr = peek2(i);
		}
		break;

	case INDIRECTY:
		{
		uint16_t i;

		i = peek(reg.PC++);
        addr = peek2(i);		
		addr += reg.YR;
        _address_page_crossed = ((addr & 0xFF) < reg.YR);
		}
		break;

	case RELATIVE:
		{
		int8_t r; /* +/- */

		r = peek(reg.PC++);

		addr = reg.PC + r;
		}
		break;

	case ABSOLUTEI:
		{
		uint8_t l;
		uint16_t h;

		l = peek(reg.PC++);
		h = peek(reg.PC++) << 8;

		addr = peek(h + l++);
		addr |= peek(h + l) << 8;
		}
	default:break;
	}

	return addr;
}

/*
 * Update carry flag
 */
static inline void
carry(uint8_t val)
{
	if (val)
		reg.SR |= CARRY;
	else
		reg.SR &= ~CARRY;
}

/*
 * Update zero and negative flags
 */
static inline void
zero_negative(uint8_t val)
{
	/* Set/Reset zero flag */
	if (val)
		reg.SR &= ~ZERO;
	else
		reg.SR |= ZERO;

	/* Set/Reset sign flag */
	if (val & NEGATIVE)
		reg.SR |= NEGATIVE;
	else
		reg.SR &= ~NEGATIVE;
}

/*
 * Update overflow flag (after ADC or SBC)
 */
static inline void
overflow(uint8_t val1, uint8_t val2, uint8_t val3)
{
	if (0x80 & (val1 ^ val2) & (val1 ^ val3))
		reg.SR |= OVERFLOW;
	else
		reg.SR &= ~OVERFLOW;
}

/*
 * Execute instruction
 * 
 * Returns
 * 	-N          OKAY, N equals to amount of processor clocks
 * 	ILLEGAL		ILLEGAL OPCODE OR ADDRESSING MODE ENCOUNTERED
 * 	BREAK		BREAK INTERRUPT OR RETURN FROM INTERRUPT
 */
int16_t
mpu(void)
{
	int16_t rc = 0;	/* Return code */

	uint8_t opcode;
	uint8_t mode;

	uint16_t addr;		/* Address */
	uint16_t acc;		/* Accumulator + Carry */
	uint8_t op1;		/* Operand 1 */
	uint8_t op2;		/* Operand 2 */
    uint8_t _extra_clocks = 0;

	opcode = peek(reg.PC++);
	if (opcode == 0xFF)
		return (ILLEGAL);            
	mode = _mode[opcode];
	if (mode == 0xFF)
		return (ILLEGAL);
    
	switch (_instruction[opcode]) {

	case  0: /* ADC */
		op1 = reg.AC;
		op2 = peek(address(mode));
		acc = op1 + op2 + (reg.SR & 1);
		zero_negative(acc);
		overflow(acc, op1, op2);
		if (reg.SR & DECIMAL) {
			acc = (op1 & 0x0F) + (op2 & 0x0F) + (reg.SR & CARRY);
			if (acc > 0x09)
				acc += 0x06;
			reg.AC = acc & 0x0F;
			acc = (op1 & 0xF0) + (op2 & 0xF0) + (acc & HALFCARRY);
			if (acc > 0x90)
				acc += 0x60;
			reg.AC |= acc & 0xF0;
		} else {
			reg.AC = acc;
		}
		carry((acc & CARRYOUT) != 0);
		break;

	case  1: /* AND */
		reg.AC &= peek(address(mode));
		zero_negative(reg.AC);
		break;

	case  2: /* ASL */
		if (opcode == 0x0A) { /* ASLA */
			acc = reg.AC << 1;
			reg.AC = acc;
		} else {
			addr = address(mode);
			acc = peek(addr) << 1;
			poke(addr, acc);
		}
		zero_negative(acc);
		carry((acc & CARRYOUT) != 0);
		break;

	case  3: /* BCC */
		if (!(reg.SR & CARRY)) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case  4: /* BCS */
		if (reg.SR & CARRY) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case  5: /* BEQ */
		if (reg.SR & ZERO) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case  6: /* BIT */
		op1 = peek(address(mode));
		reg.SR = (reg.SR & 0x3F) | (op1 & 0xC0);
		if (reg.AC & op1)
			reg.SR &= ~ZERO;
		else
			reg.SR |= ZERO;
		break;

	case  7: /* BMI */
		if (reg.SR & NEGATIVE) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case  8: /* BNE */
		if (!(reg.SR & ZERO)) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case  9: /* BPL */
		if (!(reg.SR & NEGATIVE)) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case 10: /* BRK */
		reg.PC++;
		push(reg.PC >> 8);
		push(reg.PC);
		push(reg.SR | BREAK);
		/* PC <= ($FFFE) */
        reg.PC = interrupt_vector();
		reg.SR |= INTERRUPT;
		break;

	case 11: /* BVC */
		if (!(reg.SR & OVERFLOW)) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case 12: /* BVS */
		if (reg.SR & OVERFLOW) {
            addr = address(mode);
            _extra_clocks = 1 + DIFF_PAGES(reg.PC, addr);
			reg.PC = addr;
        }
		else
			reg.PC++;
		break;

	case 13: /* CLC */
		reg.SR &= ~CARRY;
		break;

	case 14: /* CLD */
		reg.SR &= ~DECIMAL;
		break;

	case 15: /* CLI */
		reg.SR &= ~INTERRUPT;
		break;

	case 16: /* CLV */
		reg.SR &= ~OVERFLOW;
		break;

	case 17: /* CMP */
		op1 = peek(address(mode));
		acc = reg.AC - op1;
		zero_negative(acc);
		carry(reg.AC >= op1);
		break;

	case 18: /* CPX */
		op1 = peek(address(mode));
		acc = reg.XR - op1;
		zero_negative(acc);
		carry(reg.XR >= op1);
		break;

	case 19: /* CPY */
		op1 = peek(address(mode));
		acc = reg.YR - op1;
		zero_negative(acc);
		carry(reg.YR >= op1);
		break;

	case 20: /* DEC */
		addr = address(mode);
		acc = peek(addr) - 1;
		poke(addr, acc);
		zero_negative(acc);
		break;

	case 21: /* DEX */
		reg.XR--;
		zero_negative(reg.XR);
		break;

	case 22: /* DEY */
		reg.YR--;
		zero_negative(reg.YR);
		break;

	case 23: /* EOR */
		reg.AC ^= peek(address(mode));
		zero_negative(reg.AC);
		break;

	case 24: /* INC */
		addr = address(mode);
		acc = peek(addr) + 1;
		poke(addr, acc);
		zero_negative(acc);
		break;

	case 25: /* INX */
		reg.XR++;
		zero_negative(reg.XR);
		break;

	case 26: /* INY */
		reg.YR++;
		zero_negative(reg.YR);
		break;

	case 27: /* JMP */
		reg.PC = address(mode);
		break;

	case 28: /* JSR */
		addr = address(mode);
		acc = reg.PC - 1;
		push(acc >> 8);
		push(acc);
		reg.PC = addr;
		break;

	case 29: /* LDA */
		reg.AC = peek(address(mode));
		zero_negative(reg.AC);
		break;

	case 30: /* LDX */
		reg.XR = peek(address(mode));
		zero_negative(reg.XR);
		break;

	case 31: /* LDY */
		reg.YR = peek(address(mode));
		zero_negative(reg.YR);
		break;

	case 32: /* LSR */
		if (opcode == 0x4a) { /* LSRA */
			op1 = reg.AC;
			acc = op1 >> 1;
			reg.AC = acc;
		} else {
			addr = address(mode);
			op1 = peek(addr);
			acc = op1 >> 1;
			poke(addr, acc);
		}
		zero_negative(acc);
		carry((op1 & CARRY) != 0);
		break;

	case 33: /* NOP */
		break;

	case 34: /* ORA */
		reg.AC |= peek(address(mode));
		zero_negative(reg.AC);
		break;

	case 35: /* PHA */
		push(reg.AC);
		break;

	case 36: /* PHP */
		push(reg.SR);
		break;

	case 37: /* PLA */
		reg.AC = pull();
		zero_negative(reg.AC);
		break;

	case 38: /* PLP */
		reg.SR = pull() | UNKNOWN | BREAK;
		break;

	case 39: /* ROL */
		if (opcode == 0x2A) { /* ROLA */
			acc = (reg.AC << 1) | (reg.SR & CARRY);
			reg.AC = acc;
		} else {
			addr = address(mode);
			acc = (peek(addr) << 1) | (reg.SR & CARRY);
			poke(addr, acc);
		}
		zero_negative(acc);
		carry((acc & CARRYOUT) != 0);
		break;

	case 40: /* ROR */
		if (opcode == 0x6A) { /* RORA */
			op1 = reg.AC;
			acc = (op1 >> 1) | ((reg.SR & CARRY) << 7);
			reg.AC = acc;
		} else {
			addr = address(mode);
			op1 = peek(addr);
			acc = (op1 >> 1) | ((reg.SR & CARRY) << 7);
			poke(addr, acc);
		}
		zero_negative(acc);
		carry((op1 & CARRY) != 0);
		break;

	case 41: /* RTI */
		reg.SR = pull() | UNKNOWN | BREAK;
		reg.PC = pull();
		reg.PC |= pull() << 8;
		reg.SR &= ~INTERRUPT;
//		rc = BREAK;
		break;

	case 42: /* RTS */
		reg.PC = pull();
		reg.PC |= pull() << 8;
		reg.PC++;
		break;

	case 43: /* SBC */
		{
		uint8_t carryflag = reg.SR & CARRY;
		op1 = reg.AC;
		op2 = ~peek(address(mode));
		acc = op1 + op2 + carryflag;
		zero_negative(acc);
		overflow(acc, op1, op2);
		carry((acc & CARRYOUT) != 0);
		if (reg.SR & DECIMAL) {
			acc = (op1 & 0x0F) + (op2 & 0x0F) + carryflag;
			carryflag = acc & HALFCARRY;
			if (acc < 0x10)
				acc += 0x0A;
			reg.AC = acc & 0x0F;
			acc = (op1 & 0xF0) + (op2 & 0xF0) + carryflag;
			if (acc < 0x100)
				acc += 0xA0;
			reg.AC |= acc & 0xF0;
		} else {
			reg.AC = acc;
		}
		}
		break;

	case 44: /* SEC */
		reg.SR |= CARRY;
		break;

	case 45: /* SED */
		reg.SR |= DECIMAL;
		break;

	case 46: /* SEI */
		reg.SR |= INTERRUPT;
		break;

	case 47: /* STA */
		poke(address(mode), reg.AC);
		break;

	case 48: /* STX */
		poke(address(mode), reg.XR);
		break;

	case 49: /* STY */
		poke(address(mode), reg.YR);
		break;

	case 50: /* TAX */
		reg.XR = reg.AC;
		zero_negative(reg.XR);
		break;

	case 51: /* TAY */
		reg.YR = reg.AC;
		zero_negative(reg.YR);
		break;

	case 52: /* TSX */
		reg.XR = reg.SP;
		zero_negative(reg.XR);
		break;

	case 53: /* TXA */
		reg.AC = reg.XR;
		zero_negative(reg.AC);
		break;

	case 54: /* TXS */
		reg.SP = reg.XR;
		break;

	case 55: /* TYA */
		reg.AC = reg.YR;
		zero_negative(reg.AC);
	default:break;
	}
    
    if (!rc) {
        rc = _cycles[opcode];
        if (rc < 0)
            rc = -rc + _address_page_crossed;
        rc += _extra_clocks;
        rc = -rc;
    }

	return (rc);
}
