CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
SOURCES = oscar.c
HEADERS = oscar.h
OBJECTS = $(SOURCES:.c=.o)
PROGS = myoscar

all: $(PROGS)

$(PROGS): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGS) $(OBJECTS)

myoscar.o: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOURCES) $(HEADERS)

clean:
	rm -f makefile~ *.o *.c~ $(PROGS)
