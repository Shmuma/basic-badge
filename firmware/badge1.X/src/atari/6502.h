/*
 * Copyright (C) 2015 Darron Broad
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
 */

#ifndef _6502_H
#define _6502_H

struct register_file {
	uint16_t PC;	/* Program counter */
	uint8_t SR;	/* Status register */
	uint8_t AC;	/* Accumulator */
	uint8_t XR;	/* X register */
	uint8_t YR;	/* Y register */
	uint8_t SP;	/* Stack pointer */
};

#define ILLEGAL (FF)	/* Illegal opcode */
#define NINSTR (56)	/* 56 instruction types. ADC etc */

/*
 * Addressing modes
 */
#define IMPLIED     (0)
#define ACCUMULATOR (1)
#define IMMEDIATE   (2)
#define ZEROPAGE    (3)
#define ZEROPAGEX   (4)
#define ZEROPAGEY   (5)
#define ABSOLUTE    (6)
#define ABSOLUTEX   (7)
#define ABSOLUTEY   (8)
#define INDIRECTX   (9)
#define INDIRECTY   (10)
#define RELATIVE    (11)
#define ABSOLUTEI   (12) /* Absolute Indirect */
#define NMODES      (13)

/*
 * Status NV.BDIZC
 */
#define NEGATIVE  (128)
#define OVERFLOW  (64)
#define UNKNOWN   (32)
#define BREAK     (16)
#define DECIMAL   (8)
#define INTERRUPT (4)
#define ZERO      (2)
#define CARRY     (1)
#define CARRYOUT  (256)
#define HALFCARRY (16)

/*
 * Interrupt
 */
#if 0
void _IRQ(void);
void _NMI(void);
#endif

/*
 * Simulate
 */
uint16_t mpu(void);

#endif
