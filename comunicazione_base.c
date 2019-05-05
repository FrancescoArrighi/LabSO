//Header globale
#include "global.h"

typedef struct msgbuf{
    long msg_type;
    char msg_text[100];
} msgbuf;

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
