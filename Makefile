CC ?= gcc
CFLAGS ?= -Wall -Wextra -std=c11
LDFLAGS ?=

PKG_CFLAGS := $(shell pkg-config --cflags sdl3-mixer sdl3-ttf 2>/dev/null)
PKG_LIBS := $(shell pkg-config --libs sdl3-mixer sdl3-ttf 2>/dev/null)

CFLAGS += $(PKG_CFLAGS)
LDLIBS := $(PKG_LIBS) $(LDLIBS)

PREFIX ?= $(HOME)/.local
BINDIR ?= $(PREFIX)/bin
PROG = sdl-music-player
SRC = src/main.c

.PHONY: all clean install uninstall

all: $(PROG)

$(PROG): $(SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(PROG) *.o

install: all
	install -d $(BINDIR)
	install -m 755 $(PROG) $(BINDIR)/$(PROG)

uninstall:
	rm -f $(BINDIR)/$(PROG)
