CFLAGS:=-O3 -std=c11 -Wall -Wextra -pedantic -D_BSD_SOURCE
LDFLAGS=-lncurses
BINNAME:=snake
SOURCE:=snake.c
PREFIX:=/usr/local

.PHONY=clean install uninstall

$(BINNAME): $(SOURCE)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BINNAME)

install: $(BINNAME)
	install -s $(BINNAME) -m 0755 $(PREFIX)/bin/

uninstall:
	rm -f $(PREFIX)/bin/$(BINNAME)
