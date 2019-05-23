//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef WINDOW_H
#define WINDOW_H

//Operazioni di una Window
#define MSG_WINDOW_OPEN 510001
#define MSG_WINDOW_CLOSE 510002
#define MSG_WINDOW_GETTIME 510003
#define MSG_WINDOW_GETINFO 510004

//Campi per Info
#define WINDOW_INF_NOME 6
#define WINDOW_INF_STATO 7
#define WINDOW_INF_TIME 8

//Campo per Ripristino
#define WINDOW_REC_STATO 5
#define WINDOW_REC_TSTART 6

//Campo GetTime
#define WINDOW_TIME 5

void window(int id, int recupero, char * nome);
int tempo_window_on(int s, time_t t);
void concat_dati_window(msgbuf * m, int s, time_t t);
int controllo_window(char ** str, int id);
void apri_window(int * s, time_t * t);
void chiudi_window(int * s);
int equal_window(msgbuf * msg1, msgbuf * msg2);

#endif
