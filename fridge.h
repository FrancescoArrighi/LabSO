//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef FRIDGE_H
#define FRIDGE_H

#define N_FIGLI 4

typedef struct frigo{
  int id; // id dispositivo
  int stato;
  int interruttore;
  int termostato;
  int time;
  int delay;
  int percentuale;
  char * nome;
} Frigo;

void fridge(int id, int recupero);
void send_prl_dati(char* prl, Frigo *frigo); //funzione che concatena i dati del frigo (quelli di info)
char * plus_only_n (char * a);
int controlla_validita(char ** str, int id);
void header_risposta(char **msg, char *header); //funzione che prende header della richiesta (msg) e restituisce header della risposta (header)
void apri_frigo(Frigo *frigo, time_t *t_start, int *allarme, int delay_recupero); // delay_recupero: indica il delay rimenente del frigo recuperato
void chiudi_frigo(Frigo *frigo, time_t *t_start, int *allarme);
void set_stato(int valore, Frigo *frigo, time_t *t_start, int *allarme); // setta stato del frigo al valore <valore> e inizializza un timer
void set_interruttore(int valore, Frigo *frigo, time_t *t_start, int *allarme);
void set_termostato(int valore, Frigo *frigo);
void set_delay(int valore, Frigo *frigo);
void set_perc(int valore, Frigo *frigo);
void duplicate(int valore, Frigo *frigo, time_t t_start); //salva i dati del frigo e li invia al frigo con id = valore


#endif
