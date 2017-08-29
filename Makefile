CC=gcc 
CFLAGS=-O3 -lncurses -Wno-unused-result
BINNAME=snake
SOURCE=snake.c
PREFIX=/usr/local/

$(BINNAME): $(SOURCE)
	$(CC) $< -o $@ $(CFLAGS)

clean:
	rm $(BINNAME)

install: $(BINNAME)
	install -s $(BINNAME) $(PREFIX)/bin/