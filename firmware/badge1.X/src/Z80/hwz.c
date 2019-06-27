#include "hwz.h"
#include <xc.h>
#include <plib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/appio.h>

#include "sim.h"
#include "simglb.h"
#include "../hw.h"

extern const uint8_t rom_image[65536];
extern const uint8_t rd_image[131072];
extern const uint8_t rd_image2[ROMDISK2_SIZE];
uint8_t drive, sector, track,disk_temp_pointer;

uint8_t fl_rdsr(void);
uint32_t fl_rdid(void);

#ifdef	USE_RAM_IMAGE
extern const uint8_t ram_image[65536];
#endif

#ifdef	USE_RAMDISK
uint8_t ram_disk[RAMDISK_SIZE];
#endif

void reload_cpm_warm (void)
{
uint16_t i;
#ifdef	USE_RAM_IMAGE	
	for (i=0xD400;i<(0xD400+0x1EFF);i++) ram[i] = ram_image[i];
#endif
}

//-------------------device at 0x68-----------------
uint8_t rxm_sta (void)
{
/*
if (U3BSTAbits.URXDA==1) return 0xFF;
	else return 0x00;
 */
}
uint8_t rxm_read (void)
{
/*
return U3BRXREG;
 */
}
void txm_write (uint8_t data)
{
/*
U3BTXREG = data;
while (U3BSTAbits.UTXBF==1);
 */
}

void set_drive (uint8_t dat)
{
drive = dat;
disk_temp_pointer = 0;
}
void set_sector (uint8_t dat)
{
sector = dat;
disk_temp_pointer = 0;
}
void set_track (uint8_t dat)
{
track = dat;
disk_temp_pointer = 0;
}

uint8_t read_disk_byte (void)
{
uint8_t temp;
uint32_t  base,ptr;
base = (((uint32_t )(track))*16) + sector;
if (drive==0)
	{
	base = base*128;
#ifdef USE_RAMDISK
	ptr = base + disk_temp_pointer;
	if (ptr<RAMDISK_SIZE)
		temp = ram_disk[ptr];
#endif
#ifndef	USE_RAMDISK
	temp = 0xA5;
#endif
	}
if (drive==1)
	{
	base = base*128;
#ifdef	USE_ROMDISK
	temp = rd_image[base + disk_temp_pointer];
#endif
	}
if (drive==2)
	{
#ifdef USE_EEPROM
	if (disk_temp_pointer==0) read_sector(disk_temp,base);
	temp = disk_temp[disk_temp_pointer];
#endif
	base = base*128;
#ifdef	USE_ROMDISK2
	temp = rd_image2[base + disk_temp_pointer];
#endif
	}

if (drive==3)
	{
	if (disk_temp_pointer==0) fl_read_128(base+(CPM1_DISK1_OFFSET),disk_temp);
	temp = disk_temp[disk_temp_pointer];
	}
if (drive==4)
	{
	}
if (drive==5)
	{
	}
if (drive==6)
	{
	}

disk_temp_pointer++;
return temp;
}

void write_disk_byte (uint8_t dat)
{
uint8_t temp;
uint32_t base;
uint32_t  ptr;
base = (((unsigned int)(track))*16) + sector;
if (drive==0)
	{
#ifdef	USE_RAMDISK
	base = base*128;
	ptr = base + disk_temp_pointer;
	if (ptr<RAMDISK_SIZE)
		ram_disk[ptr] = dat;
#endif
	}
if (drive==1)
	{
	//rom disk, no writes allowed
	}
if (drive==2)
	{
#ifdef USE_EEPROM
	disk_temp[disk_temp_pointer] = dat;
	if (disk_temp_pointer==127) 
		{
		ee_wren();
		write_sector(disk_temp,base);
		}
#endif
	}
if (drive==3)
	{
	disk_temp[disk_temp_pointer] = dat;
	if (disk_temp_pointer==127) 
		{
		fl_write_128(base+(CPM1_DISK1_OFFSET),disk_temp);
		}
	}
if (drive==4)
	{
	}
if (drive==5)
	{
	}
if (drive==6)
	{
	}
disk_temp_pointer++;
}
