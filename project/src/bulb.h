//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una BULB
#define BULB_SWITCH_S 410001
#define BULB_SWITCH_I 410002
#define BULB_GETTIME 410003
#define BULB_GETINFO 410004
#define BULB_PRINTTIME 411003
#define BULB_PRINTINFO 411004
#define BULB_DEL 410005
#define BULB_KILL 410006

void bulb(int id, int recupero);

#endif
