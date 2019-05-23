#include "myheader.h"
#ifndef MAIN_H
#define MAIN_H


int equal_bulb(msgbuf msg_example, msgbuf messaggio);

int equal_fridge(msgbuf msg_example, msgbuf messaggio);

int equal_window(msgbuf msg_example, msgbuf messaggio);

//ritorna TRUE se ci sono dispositivi con valori diversi FALSE altrimenti; msg_example contiene il primo messaggio info utilizzato per il controllo (se il risultato è TRUE significa che ogni dispositivo ha lo stesso stato dell'example)

void del(char ** cmd, int n, int_list * figli, int queue ,int deposito);

void lik(char ** cmd, int n, int_list * figli, int queue, int deposito);

void list(char ** cmd, int n, int queue, int deposito);

void add(char ** cmd, int n, int q_dep, int new_id);

void swtch(char ** cmd, int n, int queue, int deposito);

void info(char ** cmd, int n, int queue, int deposito, int_list * figli);

void controller(int myid, int id_deposito);


int main();

#endif
