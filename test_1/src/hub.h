#include "myheader.h"
#include "useful_fun.h"
#ifndef HUB_H
#define HUB_H

int override(int_list * figli, int myid, int win_stato, int bulb_stato, int fridge_stato, int fridge_delay, int fridge_termos);

void stampa_info_hub(msgbuf *messaggio);

void hub(int id, int recupero, char * nome);

#endif
