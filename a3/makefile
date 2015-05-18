#Yunfan Li
#liyunf@onid.oregonstate.edu
#CS344-001
#Assignment #2
#No References
CC = gcc
DEBUG = -g
CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes -Wdeclaration-after-statement
OBJ1 = myword_freq.o
OBJ2 = fifo_server.o
OBJ3 = fifo_client.o
PROG1 = myword_freq
PROG2 = fifo_server
PROG3 = fifo_client
SRC1 = myword_freq.c
SRC2 = fifo_server.c
SRC3 = fifo_client.c

all: $(PROG1) $(PROG2) $(PROG3)
	rm -f makefile~ *.c~

$(PROG1): $(OBJ1)
	$(CC) -o $@ $<

$(PROG2): $(OBJ2)
	$(CC) -o $@ $<

$(PROG3): $(OBJ3)
	$(CC) -o $@ $<

$(OBJ1): $(SRC1)
	$(CC) $(CFLAGS) -c $<

$(OBJ2): $(SRC2)
	$(CC) $(CFLAGS) -c $<

$(OBJ3): $(SRC3)
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f makefile~ *.o *.c~ $(PROG1) $(PROG2) $(PROG3)

test:
	rm -f total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/latin1.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/latin2.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/latin3.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/latin4.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/latin5.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/jargon.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/words.txt > junk; done ) 2>> total
	( time for((a=1; a<=10; a++)); do ./myword_freq < Homework3/websters.txt > junk; done ) 2>> total
