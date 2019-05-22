#ifndef MYHEADER_H
#define MYHEADER_H

//Librerie che possiamo usare
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

#define BUF_SIZE 150

//valori di verità
#define TRUE 1
#define FALSE 0

//dispositivi
#define DEFAULT 0
#define CONTROLLER 1
#define HUB 2
#define TIMER 3
#define BULB 4
#define WINDOW 5
#define FRIDGE 6

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

//Size funzioni
#define MSG_SIZE 100

//messaggio base
#define MSG_TYPE_DESTINATARIO 0
#define MSG_TYPE_MITTENTE 1
#define MSG_ID_DESTINATARIO 2
#define MSG_ID_MITTENTE 3
#define MSG_OP 4

//messaggio info
#define MSG_INF 1 //codice messaggio
#define MSG_INF_ID_PADRE 5
#define MSG_INF_CONTROLDV_NFIGLI 6

#endif
