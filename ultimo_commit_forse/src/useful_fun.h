//Header globale
#include "myheader.h"

#ifndef USEFUL_FUN_H
#define  USEFUL_FUN_H

#define MSG_GETTIME 10003

int override(int_list * figli, int myid, int win_stato, int bulb_stato, int fridge_stato, int fridge_delay, int fridge_termos);

void rimuovi_maiuscole(char * str);

//ritorna TRUE o FAlSE in base se è riuscito o meno a leggere l'integer
int get_int(int n, int * val, int_list * list);

int_list * create_int_list();

int insert_int(int val, int n, int_list * list);

int rm_int(int n, int_list * list);

int str_split(char * str, char *** rt);

int protocoll_parser(char * str, char *** rt);

void crea_queue(int id, int * queue);

void bulb_info_r(char ** str);

int itoa(int n, char **str);

int controlla_validita(char ** str, int id);

int codice_messaggio(char ** msg);

void concat_int(msgbuf * messaggio, int n);

void concat_string(msgbuf * messaggio, char * str);

void crea_messaggio_base(msgbuf * messaggio, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op);

void ricomponi_messaggio(char ** cmd, int n, msgbuf * messaggio);

void invia_broadcast(msgbuf * messaggio, int_list * queue);

void recupero_in_cascata(int queue);

void svuota_msg_queue(int queue, int p);

int leggi(int queue, msgbuf * messaggio, int p, float t);

char * percorso_file(int id, int tipo);
int stampa_info_bulb(msgbuf * m);
int stampa_info_window(msgbuf * m);
int is_integer(char * str);

#endif
