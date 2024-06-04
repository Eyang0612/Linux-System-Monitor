#Compiler settings 
CC = gcc

#Compiler flags
CFLAGS = -Wall -Werror


all: mySystemStats

mySystemStats: mySystemStats.o
	$(CC) $(CFLAGS)  -o $@ $<

#generate object files
mySystemStats.o: mySystemStats.c
	$(CC) $(CFLAGS) -c $< -o $@


.PHONY: all clean help

clean:
	rm mySystemStats.o

help:
	echo "Usage: make"

