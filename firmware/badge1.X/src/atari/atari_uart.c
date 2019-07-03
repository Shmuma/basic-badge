#include "atari.h"
#include "atari_uart.h"
#include "../disp.h"

#include <stdio.h>


void atari_receive_roms() {
    uint8_t b;
    
    serial_flush(); 
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


inline uint8_t get_byte() {
    while (!rx_sta());
    return rx_read();
}


void proto_receive() {
    uint16_t i, len;
    uint8_t v, sum, error = 0;
    static char buf[30];

    video_set_color(EGA_BGREEN, EGA_BLACK);
    video_clrscr();

    len = get_byte();
    len += get_byte() << 8;
    tx_write(PROTO_ACK);
//    v = get_byte();
    video_gotoxy(0, 0);
    snprintf(buf, sizeof(buf), "Receiving %d bytes...", len);
    stdio_write(buf);
    video_gotoxy(0, 1);
    
    sum = 0;
    
    for (i = 0; i < len && !error; i++) {
        v = get_byte();
        sum += v;
        fl_write(i, v);
        if ((i % 128) == 0) {
            stdio_c('.');
        }
//            tx_write(PROTO_ACK);
//            v = get_byte();
//            if (v != PROTO_ACK)
//                error = 1;
//        }
    }

    if (error) {
        video_set_color(EGA_BRED, EGA_BLACK);
        video_gotoxy(0, 3);
        snprintf(buf, sizeof(buf), "Wrong ACK (%02x) during receive!", v);
        stdio_write(buf);
    }
    else {
        video_gotoxy(0, 3);
        snprintf(buf, sizeof(buf), "Data received, sum = %02x", sum);
        stdio_write(buf);
    }
    video_gotoxy(0, 4);
    stdio_write("Press BRK to exit");
    while (!brk_key);
    brk_key = 0;
}
