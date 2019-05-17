//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef FRIDGE_H
#define FRIDGE_H

//msg info/recupero
#define MSG_FRIDGE_STATO 5
#define MSG_FRIDGE_INTERRUTTORE 6
#define MSG_FRIDGE_TERM 7
#define MSG_FRIDGE_TIME 8
#define MSG_FRIDGE_DELAY 9
#define MSG_FRIDGE_PERC 10
#define MSG_FRIDGE_NOME 11

//msg recupero
#define MSG_FRIDGE_TSTART 12
#define MSG_FRIDGE_ID 13

//msg cod op
#define FRIDGE_INFO 1
#define FRIDGE_KILL 2
#define FRIDGE_SET_STATO 610001
#define FRIDGE_SET_INTERRUTTORE 610002
#define FRIDGE_SET_TERMOSTATO 610003
#define FRIDGE_SET_DELAY 610004
#define FRIDGE_SET_PERC 610005
#define FRIDGE_RECUPERO 610006

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

void fridge(int id, int recupero);
void concat_dati(char* prl, t_frigo *frigo); //funzione che concatena i dati del frigo (quelli di info)
int controlla_fridge(char ** str, int id);
char * plus_only_n (char * a);
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
