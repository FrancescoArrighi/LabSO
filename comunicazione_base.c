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


//valori di verità
#define TRUE 1
#define FALSE 0

//dispositivi
#define CONTROLLER 0
#define HUB 1
#define TIMER 2
#define BULB 3
#define WINDOW 4
#define FRIDGE 5

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

struct msgbuf{
    long msg_type;
    char msg_text[100];
};

typedef struct msgbuf msgbuf;

void padre(){
    key_t key;
    int queue;
    if((key = ftok("/tmp/", 0)) == -1){ // crea la chiave
        printf("errore 1\n");
        exit(1);
    }
    if((queue = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste
        printf("errore 2\n");
        exit(1);
    }
    msgbuf messaggio;
    if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 0, 0)) == -1){
        printf("errore 3\n");
        exit(1);
    }
    printf("il vaso è stato portato in salvo:\n|%s|\n",messaggio.msg_text);
}

void figlio(){
    key_t key;
    int queue;
    if((key = ftok("/tmp/", 0)) == -1){ // crea la chiave
        printf("errore 0\n");
        exit(1);
    }
    if((queue = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste
        printf("errore 1\n");
        exit(1);
    }
    msgbuf messaggio;
    strcpy(messaggio.msg_text, "ciao sono un messaggio primo!1!!!111");
    messaggio.msg_type = 1;
    if((msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0)) == -1){ // invio messaggio
        printf("errore 2\n");
        exit(1);
    }
    printf("il vaso è stato spedito\n");
}


int main() {
    if(fork() > 0){
        padre();
    }
    else{
        figlio();
    }
    
    return 0;
}
