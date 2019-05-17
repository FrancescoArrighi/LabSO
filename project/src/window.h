//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef WINDOW_H
#define WINDOW_H

//Operazioni di una Window
#define WINDOW_OPEN 510001
#define WINDOW_CLOSE 510002
#define WINDOW_GETTIME 510003
#define WINDOW_DEL 510004

void window(int id, int recupero);

#endif
