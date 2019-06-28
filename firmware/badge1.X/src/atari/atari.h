#ifndef _ATARI_H
#define _ATARI_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef ATARI_POSIX
#include <stdio.h>

#define TRACE_MEM

#define ROM_NAME rom
#define ROM_SIZE 0
#else
extern const uint8_t kernel_01_rom[];
#define ROM_NAME kernel_01_rom
#define ROM_SIZE 4096
#endif

#define false (0)
#define true (1)

#define SLEN (256)
#define FF (0xFF)

#include "6502.h"
#include "mem.h"
#include "tia.h"
#include "palette.h"

#endif
