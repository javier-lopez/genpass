PREFIX       ?= /usr/local
LIBDIR       ?= $(PREFIX)/lib
INCLUDEDIR   ?= $(PREFIX)/include
MAKE_DIR     ?= install -d
INSTALL_DATA ?= install

CC?=gcc
CFLAGS?=-O2 -Wall -g -fno-builtin-memset
LDFLAGS?=-Wl,-z,now -Wl,-z,relro -Wl,-soname,libscrypt.so.0 -Wl,--version-script=libscrypt.version
CFLAGS_EXTRA?=-Wl,-rpath=.

SHELL=/bin/sh

all: genpass

deps:
	@for dir in *; do \
		if [ -f $$dir/Makefile ]; then \
		echo "processing $$dir/"; \
		$(MAKE) -C $$dir || exit 1; \
		fi; \
	done;

genpass: deps genpass.o
	$(CC)  -o genpass genpass.o arg_parser/arg_parser.o config/ini.o \
		readpass/readpass.o encoders/*.o \
		$(CFLAGS_EXTRA) -L./libscrypt/ -lscrypt
	$(CC) -static -o genpass-static genpass.o           \
		arg_parser/arg_parser.o config/ini.o readpass/readpass.o \
		encoders/*.o \
		$(CFLAGS_EXTRA) -L./libscrypt/ -lscrypt

dist: all
	strip genpass genpass-static

clean:
	@for dir in *; do \
		if [ -f $$dir/Makefile ]; then \
		echo "processing  $$dir/"; \
		$(MAKE) clean -C $$dir; \
		fi; \
	done;
	rm -f *.o genpass genpass-static