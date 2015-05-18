CC = gcc
CFLAGS = -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
SOURCES = rm_ws.c
OBJECTS = $(SOURCES:.c=.o)
PROGS = rm_ws

all: $(PROGS)

$(PROGS): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGS) $(OBJECTS)

rm_ws.o: $(SOURCES)
	$(CC) $(CFLAGS) -c $(SOURCES)

clean:
	rm -f *.o *.c~ $(PROGS)
