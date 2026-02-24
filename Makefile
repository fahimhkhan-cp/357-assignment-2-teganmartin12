CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
MAIN = assignment2_new
OBJS = assignment2_new.o

all : $(MAIN)

$(MAIN) : $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

assignment2.o : assignment2_new.c
	$(CC) $(CFLAGS) -c assignment2_new.c

clean :

	rm *.o $(MAIN) core
