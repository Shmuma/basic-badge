all: atari

DEFS=-DATARI_POSIX
CARGS=-g -O0 -I.. -Winline -finline

%.o: ../%.c
	gcc -o $@ ${CARGS} -c ${DEFS} $<

main.o: main.c
	gcc -o main.o ${CARGS} -c ${DEFS} main.c

atari: main.o 6502.o mem.o tia.o
	gcc -o atari -g -O0 $^ 

clean:
	rm -f atari *.o
