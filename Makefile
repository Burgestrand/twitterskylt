# Compiler.
CC=gcc

# Compilation flags.
CFLAGS=-DDEBUG -ggdb -O0 -c -Wall -pedantic -std=c99

# Linker flags.
LDFLAGS=

# Source files.
SOURCES=main.c util.c twitter.c formatting.c cleaning.c
OBJECTS=$(SOURCES:.c=.o)

# Compilation target.
TARGET=twitterskylt

# Makefile specials
VPATH=src/

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm $(TARGET)
	rm $(OBJECTS)
