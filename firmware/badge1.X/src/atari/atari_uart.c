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


void proto_receive() {
    uint16_t i, len;
    uint8_t v;
    static char buf[30];
    
    while (!rx_sta());
    len = rx_read();
    while (!rx_sta());
    len += rx_read() << 8;
    
    video_set_color(EGA_BGREEN, EGA_BLACK);
    video_clrscr();
    video_gotoxy(0, 0);
    snprintf(buf, sizeof(buf), "Receiving %d bytes of ROM...", len);
    stdio_write(buf);
    video_gotoxy(0, 1);
    
    for (i = 0; i < len; i++) {
        while (!rx_sta());
        v = rx_read();
        fl_write(i, v);
        if (i % 128 == 0) {
            stdio_c('.');
            tx_write(PROTO_ACK);
        }
    }
    
    video_gotoxy(0, 3);
    stdio_write("Data received and written into flash");
}
