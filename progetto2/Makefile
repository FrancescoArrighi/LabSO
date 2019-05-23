##OBJECTS = main.o fridge.o msg_exchange.o useful_fun.o hub.o timer.o
SRCDIR = src
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:.c=.o)

CC = gcc
CFLAGS = -std=gnu90
#Linker Falgs ang CC

#brevi info testuali
info = qualcosa

help :
	@echo $(info)

build : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)

useful_fun.o : useful_fun.h useful_fun.c
	$(CC) $(CFLAGS) -c useful_fun.c

controller.o : controller.h controller.c
	$(CC) $(CFLAGS) -c controller.c

main.o : main.h main.c
	$(CC) $(CFLAGS) -c main.c

hub.o : hub.h hub.c
	$(CC) $(CFLAGS) -c hub.c

deposito.o : deposito.h deposito.c
	$(CC) $(CFLAGS) -c deposito.c

clean :
	@rm $(SRCDIR)/*.o a.out



.PHONY : build clean help
