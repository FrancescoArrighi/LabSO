//Header globale
#include "myheader.h"
#include "msg_exchange.h"

#ifndef FRIDGE_H
#define FRIDGE_H

#define N_FIGLI 4

typedef struct contenuto{
  int quantita_alimenti;
  int quantita_bevande;
} contenuto;

void fridge(int id, int recupero);
void aggiungi_contenuto(int *perc, contenuto *contenuto_frigo);
void togli_contenuto(int *perc, contenuto *contenuto_frigo);

#endif
