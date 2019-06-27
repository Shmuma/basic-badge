#ifndef		__HWZ_H
#define		__HWZ_H

#include <stdint.h>
#include "../badge_settings.h"

#define	CUT_ROMDISK2

#ifdef	CUT_ROMDISK2
#define	ROMDISK2_SIZE	131072-(18*1024)
#endif
#ifndef	CUT_ROMDISK2
#define	ROMDISK2_SIZE	131072
#endif

#define	RAMDISK_SIZE	(1024*22)

#define	CPM1_DISK1_OFFSET	1*4096
#define	CPM1_DISK2_OFFSET	2*4096
#define	CPM1_DISK3_OFFSET	3*4096

uint8_t rx_sta (void);
uint8_t rx_read (void);
void tx_write (uint8_t data);
void set_drive (uint8_t dat);
void set_sector (uint8_t dat);
void set_track (uint8_t dat);
uint8_t read_disk_byte (void);
void write_disk_byte (uint8_t dat);

unsigned char	SPI_dat (uint8_t data);
uint8_t ee_rs (void);
void ee_wren (void);
void ee_wrdi (void);
void init_diskb(void);

uint8_t rxk_sta (void);
uint8_t rxk_read (void);


void wait_1ms (void);
void wait_ms (uint32_t count);
void wait_cyc (uint16_t cyc);
void init_termint (void);



uint8_t init_first_x_sects (uint8_t i, uint8_t verify);

#define		IO_CPM_MODE		0
#define		IO_BASIC_MODE	1

#endif