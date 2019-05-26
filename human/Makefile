##OBJECTS = main.o fridge.o msg_exchange.o useful_fun.o hub.o timer.o
OBJECTS  := umano.o human_fun.o

CC = gcc
CFLAGS = -std=gnu90
#Linker Falgs ang CC

build : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)

umano.o : human_fun.h umano.c
	$(CC) $(CFLAGS) -c umano.c
human_fun.o : human_fun.h human_fun.c
	$(CC) $(CFLAGS) -c human_fun.c

clean :
	@rm *.o a.out



.PHONY : build clean
