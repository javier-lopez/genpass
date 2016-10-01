CC?=gcc
CFLAGS?=-O2 -Wall -g
LDFLAGS?=
CFLAGS_EXTRA?=-Wl,-rpath=.

all: ini

OBJS= ini.o

ini: ini.o
	$(CC) -Wall -static -c ini.c $(CFLAGS_EXTRA) -L.

clean:
	rm -f *.o
