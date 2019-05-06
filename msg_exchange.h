//Header globale
#include "global.h"

typedef struct msgbuf{
    long msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;

void create_queue(int id, int * queue);
void send_message(int queue, msgbuf buffer, char * text);
