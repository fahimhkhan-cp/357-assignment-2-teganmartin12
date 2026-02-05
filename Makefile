CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
MAIN = assignment2
OBJS = assignment2.o

all : $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

assignment2.o : assignment2.c
	$(CC) $(CFLAGS) -c assignment2.c

clean :
	rm *.o $(MAIN) core