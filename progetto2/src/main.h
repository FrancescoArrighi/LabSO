#include "myheader.h"
#include "useful_fun.h"
#include "deposito.h"
#ifndef MAIN_H
#define MAIN_H



int equal_fridge(msgbuf msg_example, msgbuf messaggio);

int equal_window(msgbuf msg_example, msgbuf messaggio);

void del(char ** cmd, int n, int_list * figli, int queue ,int deposito);

void lik(char ** cmd, int n, int_list * figli, int queue, int deposito);

void list(char ** cmd, int n, int queue, int deposito);

void add(char ** cmd, int n, int q_dep, int new_id);

void swtch(char ** cmd, int n, int queue, int deposito);

void info(char ** cmd, int n, int queue, int deposito, int_list * figli);

void controller(int myid, int id_deposito);

int main();

#endif
