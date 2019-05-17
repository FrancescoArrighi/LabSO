##OBJECTS = main.o fridge.o msg_exchange.o useful_fun.o hub.o timer.o
SRCDIR = src
SOURCES  := $(wildcard $(SRCDIR)/*.c)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(SRCDIR)/%.o)

CC = gcc
CFLAGS = -std=gnu90
#Linker Falgs ang CC

#brevi info testuali
info = qualcosa

help :
	@echo $(info)

build : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS)

main.o : myheader.h main.c
	$(CC) $(CFLAGS) -c main.c

controller.o : controller.h controller.c
	$(CC) $(CFLAGS) -c controller.c

hub.o : hub.h hub.c
	$(CC) $(CFLAGS) -c hub.c

timer.o : timer.h timer.c
	$(CC) $(CFLAGS) -c timer.c

bulb.o : bulb.h bulb.c
	$(CC) $(CFLAGS) -c bulb.c

window.o : window.h window.c
	$(CC) $(CFLAGS) -c window.c

fridge.o : fridge.h fridge.c
	$(CC) $(CFLAGS) -c fridge.c

msg_exchange.o : msg_exchange.h msg_exchange.c
	$(CC) $(CFLAGS) -c msg_exchange.c

useful_fun.o : useful_fun.h useful_fun.c
	$(CC) $(CFLAGS) -c useful_fun.c

clean :
	@rm $(SRCDIR)/*.o a.out



.PHONY : build clean help
