CC = gcc
CFLAGS = -g -O2 -Wall


all:	blackjack

blackjack:	blackjack.o
	$(CC) -o blackjack blackjack.o

blackjack.o:	blackjack.c
	$(CC) $(CFLAGS) -c blackjack.c

clean:	
	rm -f blackjack blackjack.o
