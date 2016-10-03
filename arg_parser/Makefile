CC?=gcc
CFLAGS?=-O2 -Wall -g
LDFLAGS?=
CFLAGS_EXTRA?=-Wl,-rpath=.

all: arg_parser_example

OBJS= arg_parser.o

arg_parser_example: arg_parser.o
	$(CC) -Wall -static -o arg_parser_example arg_parser_example.c arg_parser.o $(CFLAGS_EXTRA) -L.

clean:
	rm -f *.o arg_parser_example
