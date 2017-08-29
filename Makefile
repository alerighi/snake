CC=gcc 
CFLAGS=-O3 -lncurses -Wno-unused-result
BINNAME=snake
SOURCE=snake.c
MANPAGE=snake.6
PREFIX=/usr/local

$(BINNAME): $(SOURCE)
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm $(BINNAME)

install: $(BINNAME)
	install -s $(BINNAME) $(PREFIX)/bin/
	install $(MANPAGE) $(PREFIX)/share/man/man6 
