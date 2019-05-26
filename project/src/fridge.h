//Header globale
#include "myheader.h"
#include "useful_fun.h"

#ifndef FRIDGE_H
#define FRIDGE_H

//msg recupero
#define MSG_FRIDGE_REC_TYPE 5
#define MSG_FRIDGE_REC_ID 6
#define MSG_FRIDGE_REC_NOME 7
#define MSG_FRIDGE_REC_STATO 8
#define MSG_FRIDGE_REC_INTERRUTTORE 9
#define MSG_FRIDGE_REC_TERM 10
#define MSG_FRIDGE_REC_TIME 11
#define MSG_FRIDGE_REC_DELAY 12
#define MSG_FRIDGE_REC_PERC 13
#define MSG_FRIDGE_REC_TSTART 14


//msg cod op
#define MSG_FRIDGE_SETINTERRUTTORE 710003
#define MSG_FRIDGE_SETTERMOSTATO 710004
#define MSG_FRIDGE_SETDELAY 710005
#define MSG_FRIDGE_SETPERC 710006
#define MSG_FRIDGE_GETSTATO 710051 //deve essere il primo di get -> c_umano
#define MSG_FRIDGE_GETTIME 710052
#define MSG_FRIDGE_GETTERMOSTATO 710053
#define MSG_FRIDGE_GETDELAY 710054
#define MSG_FRIDGE_GETPERC 710055



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
void salva_dati(t_frigo *frigo, time_t t_start, int allarme, int idf); //salva i dati del frigo e li invia al frigo con id = valore
void send_info_fridge(char **msg, t_frigo *frigo); //invia la risposta di info
void invia_ackp(int myid, char **msg, int queue_risposta);

#endif
