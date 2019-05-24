//Header globale
#include "myheader.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una Bulb
#define MSG_BULB_SWITCH_S 410001
#define MSG_BULB_SWITCH_I 410002
#define MSG_BULB_GETTIME 410003
#define MSG_BULB_GETINFO 410004

//Campi per Info
#define BULB_INF_NOME 6
#define BULB_INF_STATO 7
#define BULB_INF_INTERRUTTORE 8
#define BULB_INF_TIME 9

//Campo per Ripristino
#define BULB_REC_STATO 5
#define BULB_REC_INTERRUTTORE 6
#define BULB_REC_TSTART 7

//Campo GetTime
#define BULB_TIME 5

void bulb(int id, int recupero, char * nome);
void concat_dati_bulb(msgbuf * m, int s, int i, time_t t);
int controllo_bulb(char ** str, int id);
void inverti_interruttore(int * s, int * i, time_t *t);
void inverti_stato(int * s, int * i, time_t *t);
int tempo_bulb_on(int s, time_t t);
int equal_bulb(msgbuf * msg1, msgbuf * msg2);
#endif
