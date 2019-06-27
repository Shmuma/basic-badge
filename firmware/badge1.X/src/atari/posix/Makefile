HEADERS=6502.h

.c.o:
	gcc -o $@ -g -O0 -c $<

atari: main.o 6502.o mem.o tia.o
	gcc -o atari -g -O0 $^ 

clean:
	rm -f atari *.o
