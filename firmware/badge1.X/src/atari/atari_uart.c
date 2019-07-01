#include "atari.h"
#include "atari_uart.h"
#include "../disp.h"


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
    
    proto_receive();
}


void proto_receive() {
}
