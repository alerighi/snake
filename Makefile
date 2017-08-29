CC=gcc 
CFLAGS=-O3 -lncurses
BINNAME=snake
SOURCE=snake.c
PREFIX=/usr/local/

$(BINNAME): $(SOURCE)
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm $(BINNAME)

install: $(BINNAME)
	install -s $(BINNAME) $(PREFIX)/bin/