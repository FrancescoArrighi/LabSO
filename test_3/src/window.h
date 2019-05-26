//Header globale
#include "myheader.h"
#include "useful_fun.h"

#ifndef WINDOW_H
#define WINDOW_H

//Campi per Ripristino
#define WINDOW_REC_STATO 7
#define WINDOW_REC_TSTART 8
#define WINDOW_REC_NOME 9

//Campo GetTime
#define WINDOW_TIME 5 // quando viene chiesto il tempo viene concatenato solo il tempo d'utilixxo

void window(int id, int recupero, char * nome);
int tempo_window_on(int s, time_t t);
void concat_dati_window(msgbuf * m, int s, time_t t, char * nw);
int controllo_window(char ** str, int id);
void apri_window(int * s, time_t * t);
void chiudi_window(int * s);
int equal_window(msgbuf * msg1, msgbuf * msg2);

#endif
