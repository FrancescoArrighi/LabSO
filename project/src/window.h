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
#define WINDOW_GETINFO 510004
#define WINDOW_PRINTTIME 511003
#define WINDOW_PRINTINFO 511004
#define WINDOW_DEL 510005
#define WINDOW_KILL 510006

#define WINDOW_INF_STATO 5
#define WINDOW_INF_INTERRUTTORE 6
#define WINDOW_INF_TIME 7

void window(int id, int recupero);

#endif
