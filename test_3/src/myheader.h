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

#define MSG_OVERRIDE_RISP 200

#define MSG_OVERRIDE_WINST 5
#define MSG_OVERRIDE_BULBST 6
#define MSG_OVERRIDE_FRIDGEST 7
#define MSG_OVERRIDE_FRIDGEDLY 8
#define MSG_OVERRIDE_FRIDGETERM 9

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

#define MSG_RECUPERO_TIMER_NOME 7
#define MSG_RECUPERO_TIMER_WINST 8
#define MSG_RECUPERO_TIMER_BULBST 9
#define MSG_RECUPERO_TIMER_FRIDGEST 10
#define MSG_RECUPERO_TIMER_FRIDGEDLY 11
#define MSG_RECUPERO_TIMER_FRIDGETSTART 12
#define MSG_RECUPERO_TIMER_FRIDGETERM 13
#define MSG_RECUPERO_TIMER_FIGLO 14
#define MSG_RECUPERO_TIMER_TTIMEOUT 15
#define MSG_RECUPERO_TIMER_TIMEOUTTEXT 16

#define MSG_TIMER_TIMEOUT 410600

#define MSG_RECUPERO_HUB_NOME 7
#define MSG_RECUPERO_HUB_WINST 8
#define MSG_RECUPERO_HUB_BULBST 9
#define MSG_RECUPERO_HUB_FRIDGEST 10
#define MSG_RECUPERO_HUB_FRIDGEDLY 11
#define MSG_RECUPERO_HUB_FRIDGETSTART 12
#define MSG_RECUPERO_HUB_FRIDGETERM 13
#define MSG_RECUPERO_HUB_INIZIOFIGLI 14

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
#define MSG_ADD_DEVICE_NOME 7
#define MSG_ADD_DEVICE_TIMER_DELAY 8
#define MSG_ADD_DEVICE_TIMER_NEXTMSG 9

#define MSG_SALVA_SPEGNI 16
#define MSG_SPEGNI 17

#define BUF_SIZE 150

//Campi per Info - MSG_INF_BULB
#define MSG_BULB_INF_NOME 6 // Subito dopo l'id Padre
#define MSG_BULB_INF_STATO 7
#define MSG_BULB_INF_INTERRUTTORE 8
#define MSG_BULB_INF_TIME 9

//Campi per Info - MSG_INF_WINDOW
#define MSG_WINDOW_INF_NOME 6 // Subito dopo l'id Padre
#define MSG_WINDOW_INF_STATO 7
#define MSG_WINDOW_INF_TIME 8

//Operazioni della Window
#define MSG_WINDOW_OPEN 610001
#define MSG_WINDOW_CLOSE 610002

//Campi per Info - MSG_INF_FRIDGE
//#define MSG_INF_NOME 6

#define MSG_FRIDGE_INF_STATO 7
#define MSG_FRIDGE_INF_INTERRUTTORE 8
#define MSG_FRIDGE_INF_TERM 9
#define MSG_FRIDGE_INF_TIME 10
#define MSG_FRIDGE_INF_DELAY 11
#define MSG_FRIDGE_INF_PERC 12

#define MSG_FRIDGE_VALORE 5

#define MSG_SETSTATO 1050
#define MSG_SETSTATO_VAL 5
#define MSG_ALLON 1051
#define MSG_ALLOFF 1052

#define MSG_FRIDGE_SETSTATO 710002
#define MSG_FRIDGE_SETTERMOSTATO 710004

#define MSG_HUB_FRIDGE_TIMEOUT 610002


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
