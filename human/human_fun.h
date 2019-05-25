//Header globale
#include "myheader.h"

#ifndef USEFUL_FUN_H
#define  USEFUL_FUN_H

#define MSG_WINDOW_OPEN 610001
#define MSG_WINDOW_CLOSE 610002
#define MSG_GETTIME 10003
#define MSG_BULB_SWITCH_I 510002

void crea_queue(int id, int * queue);
void concat_int(msgbuf * messaggio, int n);
void concat_string(msgbuf * messaggio, char * str);
void crea_messaggio_base(msgbuf * messaggio, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op);
int itoa(int n, char **str);

int str_split(char * str, char *** rt);

int protocoll_parser(char * str, char *** rt);

#endif
