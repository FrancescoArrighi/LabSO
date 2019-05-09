//Header globale
#include "myheader.h"

#ifndef MSG_EXCHANGE_H
#define MSG_EXCHANGE_H

//Struct: buffer che punta alla coda queue 
typedef struct msgbuf{
    long int msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;

void create_queue(int id, int * queue);
void send_message(int queue, msgbuf * buffer, char * text, long int priorita);

#endif
