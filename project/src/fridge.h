//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef FRIDGE_H
#define FRIDGE_H

//
#define FRIDGE_VALORE 5

//msg info
#define FRIDGE_INF_STATO 6
#define FRIDGE_INF_INTERRUTTORE 7
#define FRIDGE_INF_TERM 8
#define FRIDGE_INF_TIME 9
#define FRIDGE_INF_DELAY 10
#define FRIDGE_INF_PERC 11
#define FRIDGE_INF_NOME 12

//msg recupero
#define FRIDGE_REC_STATO 5
#define FRIDGE_REC_INTERRUTTORE 6
#define FRIDGE_REC_TERM 7
#define FRIDGE_REC_TIME 8
#define FRIDGE_REC_DELAY 9
#define FRIDGE_REC_PERC 10
#define FRIDGE_REC_NOME 11
#define FRIDGE_REC_TSTART 12
#define FRIDGE_REC_ID 13

//msg cod op
#define FRIDGE_INFO 1
#define FRIDGE_KILL 2
#define FRIDGE_SET_STATO 710001
#define FRIDGE_SET_INTERRUTTORE 710002
#define FRIDGE_SET_TERMOSTATO 710003
#define FRIDGE_SET_DELAY 710004
#define FRIDGE_SET_PERC 710005
#define FRIDGE_RECUPERO 710006

#define FRIDGE_GET_STATO 51
#define FRIDGE_GET_TIME 52
#define FRIDGE_GET_TERMOSTATO 53
#define FRIDGE_GET_DELAY 54
#define FRIDGE_GET_PERC 55



typedef struct _frigo{
  int id; // id dispositivo
  int stato;
  int interruttore;
  int termostato;
  int time;
  int delay;
  int percentuale;
  char * nome;
} t_frigo;

void fridge(int id, int recupero, char *nome);
void concat_dati(msgbuf *messaggio, t_frigo *frigo); //funzione che concatena i dati del frigo (quelli di info)
int controlla_fridge(char ** str, int id);
void apri_frigo(t_frigo *frigo, time_t *t_start, int *allarme, int delay_recupero); // delay_recupero: indica il delay rimenente del frigo recuperato
void chiudi_frigo(t_frigo *frigo, time_t *t_start, int *allarme);
void set_stato(int valore, t_frigo *frigo, time_t *t_start, int *allarme); // setta stato del frigo al valore <valore> e inizializza un timer
void set_interruttore(int valore, t_frigo *frigo, time_t *t_start, int *allarme);
void set_termostato(int valore, t_frigo *frigo);
void set_delay(int valore, t_frigo *frigo);
void set_perc(int valore, t_frigo *frigo);
void duplicate(t_frigo *frigo, time_t t_start); //salva i dati del frigo e li invia al frigo con id = valore
void send_info_fridge(char **msg, t_frigo *frigo); //invia la risposta di info

#endif
