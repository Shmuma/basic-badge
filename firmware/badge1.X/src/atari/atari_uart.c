#include "atari.h"
#include "atari_uart.h"
#include "../disp.h"

#include <stdio.h>


void atari_receive_roms() {
    uint8_t b;
    
    video_set_color(EGA_BGREEN, EGA_BLACK);
    video_clrscr();
    video_gotoxy(0, 0);
    stdio_write("Waiting for handshake on UART3");
    video_gotoxy(0, 1);
    stdio_write("Press BRK to interrupt...");
    
    serial_flush(); 
    while (!rx_sta() && !brk_key);
    if (brk_key) {
        brk_key = 0;
        return;
    }
    b = rx_read();
    
    if (b != PROTO_HANDSHAKE) {
        video_gotoxy(0, 2);
        video_set_color(EGA_BRED, EGA_BLACK);
        stdio_write("Wrong handshake!");
        while (!brk_key);
        brk_key = 0;
        return;
    }
    
    tx_write(PROTO_HANDSHAKE);
    proto_receive();
}


inline uint8_t get_byte() {
    while (!rx_sta());
    return rx_read();
}


void proto_receive() {
    uint32_t ofs;
    uint16_t i, len, sector = 0;
    uint8_t v, sum;
    static char buf[30];

    video_set_color(EGA_BGREEN, EGA_BLACK);
    video_clrscr();

    while (sector < 10) {
        len = get_byte();
        len += get_byte() << 8;
        tx_write(PROTO_ACK);
        if (len == 0)
            break;

        video_gotoxy(0, sector);
        snprintf(buf, sizeof(buf), "Receiving %d bytes...", len);
        stdio_write(buf);
        video_gotoxy(0, sector + 1);

        sum = 0;
        for (i = 0; i < len; i++, ofs++) {
            v = get_byte();
            sum += v;
            rom_data[i] = v;
            if ((i % 128) == 0) {
                stdio_c('.');
            }
        }

        video_gotoxy(0, sector);
        snprintf(buf, sizeof(buf), "Sector %d received, sum = %02x", 
                sector, sum);
        stdio_write(buf);
        fl_erase_4k(sector << 12);
        // TODO: this flash could be written in 256 bytes chunk in one burst
        fl_write_4k(sector << 12, rom_data);
        
        sector += 1;
    }
    video_gotoxy(0, sector);
    stdio_write("Press BRK to exit");
    while (!brk_key);
    brk_key = 0;
}
