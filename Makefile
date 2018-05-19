CFLAGS=-O3 -std=c11 -lncurses -pedantic -Wextra
BINNAME=snake
SOURCE=snake.c
MANPAGE=snake.6
PREFIX=/usr/local

$(BINNAME): $(SOURCE)
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm -f $(BINNAME)

install: $(BINNAME)
	install -s $(BINNAME) -m 0755 $(PREFIX)/bin/
	mkdir -p $(PREFIX)/share/man/man6/
	install $(MANPAGE) -m 0644 $(PREFIX)/share/man/man6/

uninstall:
	rm -f $(PREFIX)/bin/$(BINNAME)
	rm -f $(PREFIX)/share/man/man6/$(MANPAGE)

.PHONY=clean install uninstall