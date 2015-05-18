#Yunfan Li
#liyunf@onid.oregonstate.edu
#CS344-001
#Assignment #5
#No References
CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
SUFFIX = -lrt
OBJ1 = socket_client.o
OBJ2 = socket_server.o
OBJ3 = myhostdb.o
PROG1 = socket_client
PROG2 = socket_server
PROG3 = myhostdb
SRC1 = socket_client.c
SRC2 = socket_server.c
SRC3 = myhostdb.c

all: $(PROG1) $(PROG2) $(PROG3)
	rm -f makefile~ *.c~

$(PROG1): $(OBJ1)
	$(CC) -o $@ $< $(SUFFIX)

$(PROG2): $(OBJ2)
	$(CC) -o $@ $< $(SUFFIX)

$(PROG3): $(OBJ3)
	$(CC) -o $@ $< $(SUFFIX)

$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -c $<

$(OBJ2): $(SRC2)
	$(CC) $(CFLAGS) -c $<

$(OBJ3): $(SRC3)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f makefile~ *.o *.c~ $(PROG1) $(PROG2) $(PROG3)

r:
	rm /dev/shm/HOSTDB__liyunf