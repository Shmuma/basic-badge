all: atari

DEFS=-DATARI_POSIX

%.o: ../%.c
	gcc -o $@ -g -O0 -I.. -c ${DEFS} $<

main.o: main.c
	gcc -o main.o -g -O0 -I.. -c ${DEFS} main.c

atari: main.o 6502.o mem.o tia.o
	gcc -o atari -g -O0 $^ 

clean:
	rm -f atari *.o
