//Header globale
#include "myheader.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una Bulb
#define MSG_BULB_SWITCH_I 510002

//Campo per Ripristino
#define BULB_REC_STATO 7
#define BULB_REC_INTERRUTTORE 8
#define BULB_REC_TSTART 9
#define BULB_REC_NOME 10

//Campo GetTime
#define BULB_TIME 5 // quando viene chiesto il tempo viene concatenato solo il tempo d'utilizzo

void bulb(int id, int recupero, char * nome);
void concat_dati_bulb(msgbuf * m, int s, int i, time_t t, char * nb);
int controllo_bulb(char ** str, int id);
void inverti_interruttore(int * s, int * i, time_t *t);
void inverti_stato(int * s, int * i, time_t *t);
int tempo_bulb_on(int s, time_t t);

#endif
