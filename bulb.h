//Header globale
#include "myheader.h"
#include "msg_exchange.h"
#include "useful_fun.h"

#ifndef BULB_H
#define BULB_H

//Operazioni di una Bulb
#define BULB_INVERTI_S 410001
#define BULB_INVERTI_I 410002
#define BULB_GETTIME 410003
#define BULB_DEL 410004
#define BULB_KILL 410005

void bulb(int id, int recupero);

#endif
