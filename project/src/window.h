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

//Campi per ripristino
#define WINDOW_INF_STATO 5
#define WINDOW_INF_TIME 6
#define WINDOW_INF_TSTART 6
//Campo Time
#define WINDOW_TIME 5

void window(int id, int recupero);
int tempo_window_on(int s, time_t t);
void concat_dati_window(msgbuf * m, int s, time_t t);
int controllo_window(char ** str, int id);
void apri_window(int * s, time_t * t);
void chiudi_window(int * s);
#endif
