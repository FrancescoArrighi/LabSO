#ifndef MYHEADER_H
#define MYHEADER_H

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MYHEADER_H

//Size funzioni
#define MSG_SIZE 250
#define MSG_FIFO 3
//valori di verità
#define TRUE 1
#define FALSE 0

//dispositivi
#define DEFAULT 0
#define CONTROLLER 1
#define DEPOSITO 2
#define HUB 3
#define TIMER 4
#define BULB 5
#define WINDOW 6
#define FRIDGE 7

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

#define NUOVA_OPERAZIONE 4

//messaggio base
#define MSG_TYPE_DESTINATARIO 0
#define MSG_TYPE_MITTENTE 1
#define MSG_ID_DESTINATARIO 2
#define MSG_ID_MITTENTE 3
#define MSG_OP 4

#define MSG_ACKP 1
#define MSG_ACKN 0

//messaggio info
#define MSG_INF 1014 //codice messaggio
#define MSG_INF_IDPADRE 5
#define MSG_INF_NOME 6
#define MSG_INF_CONTROLDV_TYPEF 7
#define MSG_INF_CONTROLDV_NFIGLI 8

#define MSG_OVERRIDE 1010

#define MSG_AGGIUNGI 1011
#define MSG_AGGIUNGI_IDF 5

#define MSG_GET_TERMINAL_TYPE 1012
#define MSG_MYTYPE 1013
#define MSG_MYTYPE_TYPE 5

#define MSG_INF_DEPOSITO 210011
#define MSG_INF_HUB 310011
#define MSG_INF_TIMER 410011
#define MSG_INF_BULB 510011
#define MSG_INF_WINDOW 610011
#define MSG_INF_FRIDGE 710011

#define MSG_RECUPERO_HUB 310001
#define MSG_RECUPERO_TIMER 410001
#define MSG_RECUPERO_BULB 510001
#define MSG_RECUPERO_WINDOW 610001
#define MSG_RECUPERO_FRIDGE 710001

#define MSG_RECUPERO_TYPE 5
#define MSG_RECUPERO_ID 6

#define MSG_RECUPERO_HUB_NOME 7
#define MSG_RECUPERO_HUB_INIZIOFIGLI 8

#define MSG_DEPOSITO_DEL 210012
#define MSG_DEPOSITO_DEL_ID 5

#define MSG_RIMUOVIFIGLIO 15
#define MSG_RIMUOVIFIGLIO_ID 5
#define MSG_RIMUOVIFIGLIO_SPEC 6
#define MSG_RIMUOVIFIGLIO_SPEC_DEP 2    //salva lo stato e lo aggiunge al deposito
#define MSG_RIMUOVIFIGLIO_SPEC_SALVA 1  //salva lo stato ma non lo aggiunge al deposito
#define MSG_RIMUOVIFIGLIO_SPEC_DEL 0

#define MSG_ADD_DEVICE 210013
#define MSG_ADD_DEVICE_TYPE 5
#define MSG_ADD_DEVICE_ID 6

#define MSG_SALVA_SPEGNI 16
#define MSG_SPEGNI 17

#define BUF_SIZE 150

//Struct
typedef struct pair_int{
    int first;
    int second;
} pair_int;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct int_list{
    int_node * head;
    int n;
} int_list;

typedef struct msgbuf{
    long int msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;


#endif
