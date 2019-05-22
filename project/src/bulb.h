//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una BULB
#define MSG_BULB_SWITCH_S 410001
#define MSG_BULB_SWITCH_I 410002
#define MSG_BULB_GETTIME 410003
#define MSG_BULB_GETINFO 410004
#define MSG_BULB_PRINTTIME 411003
#define MSG_BULB_PRINTINFO 411004
#define MSG_BULB_DEL 410005
#define MSG_BULB_KILL 410006

// Campi per ripristino
#define BULB_INF_TIME 5
#define BULB_INF_INTERRUTTORE 6
#define BULB_INF_STATO 7

void bulb(int id, int recupero);

#endif
