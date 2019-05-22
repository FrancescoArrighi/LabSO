//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una Bulb
#define MSG_BULB_SWITCH_S 410001
#define MSG_BULB_SWITCH_I 410002
#define MSG_BULB_GETTIME 410003
#define MSG_BULB_GETINFO 410004

//Campi per ripristino
#define BULB_INF_STATO 5
#define BULB_INF_INTERRUTTORE 6
#define BULB_INF_TIME 7
//Richiesta di Time
#define BULB_TIME 5

void bulb(int id, int recupero);
void concat_dati_bulb(msgbuf * m, int s, int i, time_t t);
int controllo_bulb(char ** str, int id);
void inverti_interruttore(int * s, int * i, time_t *t);
void inverti_stato(int * s, int * i, time_t *t);
int tempo_bulb_on(int s, time_t t);
#endif
