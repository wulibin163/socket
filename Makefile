
all:
	gcc -o udp udp.c
	gcc -o tcp tcp.c
	gcc -o reuse reuse.c

clean:
	rm -f udp tcp reuse
