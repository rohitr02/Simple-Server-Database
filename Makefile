OUTPUT=pa3
CFLAGS=-g -Wall -Werror -fsanitize=undefined -fsanitize=address -std=c99
LFLAGS = -pthread

all: $(OUTPUT)

pa3: pa3.c pa3.h LinkedList.h
	gcc $(CFLAGS) -o $@ $< $(LFLAGS)

echoes: echoes.c
	gcc $(CFLAGS) -o echoes echoes.c $(LFLAGS)

clean:
	rm -f *.o $(OUTPUT)