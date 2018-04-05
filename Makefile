CFILES=$(wildcard *.c)
COBJS=$(subst .c,.o,$(CFILES))
TARGET=udp

all: $(COBJS)
	gcc -o $(TARGET)  $<

clean:
	rm $(TARGET)