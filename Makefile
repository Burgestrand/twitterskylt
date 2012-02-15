# Compiler.
CC=gcc

# Compilation flags.
CFLAGS=-DNODEBUG -ggdb -O0 -c -Wall -pedantic -std=c99

# Linker flags.
LDFLAGS=

# Source files.
SOURCES=util.c twitter.c formatting.c cleaning.c bsd-strlcpy.c
OBJECTS=$(SOURCES:.c=.o)

# Compilation target.
TARGET=twitterskylt
TEST_TARGET=tests

# Makefile specials
VPATH=src/

all: $(SOURCES) $(TARGET)
test: $(SOURCES) $(TEST_TARGET)

$(TARGET): $(OBJECTS) main.o
	$(CC) $(LDFLAGS) $(OBJECTS) main.o -o $@

.PHONY: $(TEST_TARGET)
$(TEST_TARGET): $(OBJECTS) tests.o
	$(CC) $(LDFLAGS) $(OBJECTS) tests.o -o $@
	./tests

.PHONY: clean
clean:
	rm $(OBJECTS) main.o tests.o $(TARGET) $(TEST_TARGET)

.o:
	$(CC) $(CFLAGS) $< -o $@
