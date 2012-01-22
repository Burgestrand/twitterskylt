# Compiler.
CC=gcc

# Compilation flags.
CFLAGS=-DDEBUG -ggdb -O0 -c -Wall -pedantic -std=c99

# Linker flags.
LDFLAGS=

all: main
	$(CC) $(LDFLAGS) main.o util.o twitter.o -o twitterskylt

main: util.o twitter.o
	$(CC) $(CFLAGS) src/main.c

util.o:
	$(CC) $(CFLAGS) src/util.c

twitter.o:
	$(CC) $(CFLAGS) src/twitter.c

clean:
	rm twitterskylt
	rm *.o
