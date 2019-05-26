
#ifndef HUMAN_FUN_H
#define HUMAN_FUN_H

//Header umano

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//dispositivi
#define DEFAULT 0
#define CONTROLLER 1
#define DEPOSITO 2
#define HUB 3
#define TIMER 4
#define BULB 5
#define WINDOW 6
#define FRIDGE 7

//Size funzioni
#define MSG_SIZE 250
#define MSG_FIFO 3
//valori di verità
#define TRUE 1
#define FALSE 0

#define MSG_ACKP 1
#define MSG_ACKN 0
#define BUF_SIZE 150

#define MSG_INF 1014 //codice messaggio

#define MSG_WINDOW_OPEN 610001
#define MSG_WINDOW_CLOSE 610002
#define MSG_GETTIME 10003
#define MSG_BULB_SWITCH_I 510002

#define MSG_FRIDGE_SETINTERRUTTORE 710003
#define MSG_FRIDGE_SETTERMOSTATO 710004
#define MSG_FRIDGE_SETDELAY 710005
#define MSG_FRIDGE_SETPERC 710006
#define MSG_FRIDGE_GETSTATO 710051 //deve essere il primo di get -> c_umano
#define MSG_FRIDGE_GETTIME 710052
#define MSG_FRIDGE_GETTERMOSTATO 710053
#define MSG_FRIDGE_GETDELAY 710054
#define MSG_FRIDGE_GETPERC 710055

typedef struct msgbuf{
    long int msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;

void crea_queue(int id, int * queue);
void concat_int(msgbuf * messaggio, int n);
void concat_string(msgbuf * messaggio, char * str);
void crea_messaggio_base(msgbuf * messaggio, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op);
int itoa(int n, char **str);

int str_split(char * str, char *** rt);

int protocoll_parser(char * str, char *** rt);
int is_integer(char * str);
#endif
