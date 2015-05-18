#Yunfan Li
#liyunf@onid.oregonstate.edu
#CS344-001
#Assignment #2
#No References
CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
OSUFFIX = -lrt -pthread
OBJ1 = posixmsg_client.o
OBJ2 = posixmsg_server.o
#OBJ3 = fifo_client.o
PROG1 = posixmsg_client
PROG2 = posixmsg_server
#PROG3 = fifo_client
SRC1 = posixmsg_client.c
SRC2 = posixmsg_server.c
#SRC3 = fifo_client.c

all: $(PROG1) $(PROG2) #$(PROG3)
	rm -f makefile~ *.c~

$(PROG1): $(OBJ1)
	$(CC) -o $@ $< $(OSUFFIX)

$(PROG2): $(OBJ2)
	$(CC) -o $@ $< $(OSUFFIX)

#$(PROG3): $(OBJ3)
#	$(CC) -o $@ $<

$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -c $<

$(OBJ2): $(SRC2)
	$(CC) $(CFLAGS) -c $<

#$(OBJ3): $(SRC3)
#	$(CC) $(CFLAGS) -c $<

clean:
	rm -f makefile~ *.o *.c~ $(PROG1) $(PROG2) #$(PROG3)

r:
	rm /dev/mqueue/ServerQueue___liyunf /dev/mqueue/ClientWriterQueue___liyunf* /dev/mqueue/ClientReaderQueue___liyunf*