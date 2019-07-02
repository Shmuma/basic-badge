#ifndef ATARI_UART_H
#define	ATARI_UART_H

// byte used for initial handshake
#define PROTO_HANDSHAKE 0x42
// byte sent after every 128 bytes got
#define PROTO_ACK       0x40        

void atari_receive_roms();
void proto_receive();

#endif	/* ATARI_UART_H */

