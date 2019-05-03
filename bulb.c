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

//Struct
typedef struct {
    int first;
    int second;
} pair_int;

typedef struct {
  int id;
  pid_t pid;
  int fd[2];
} device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct {
    int_node * head;
    int n;
} int_list;

typedef struct {
    int integer;
    device * val;
} pair_int_device;

typedef struct device_node{
    device * val;
    struct device_node * next;
} device_node;

typedef struct {
    device_node * head;
    int n;
} device_list;



void crea_queue(int id, int * queue){
  key_t key;
  if((key = ftok("/tmp/", id)) == -1){ // crea la chiave
      printf("errore 1\n");
      exit(1);
  }
  if(((*queue) = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste
      printf("errore 2\n");
      exit(1);
  }
}

inf flag;

void sighandle_accendi(int sig){
  flag = 1;
}

void sighandle_spegni(int sig){
  flag = 2;
}

void bulb(int id, int recupero){ //recupero booleano
  int acceso;
  long long int tempo_utilizzo;
  char nome[] = "BULB-ID";


  int queue;
  msgbuf messaggio;

  if(fork() == 0){// codice figlio
    flag = 0;
    crea_queue(id,queue);
    signal(SIGUSR1, sighandle_accendi);
    signal(SIGUSR2, sighandle_spegni);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "accendi");
        messaggio.msg_type = 1;
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        strcpy(messaggio.msg_text, "spegni");
        messaggio.msg_type = 1;
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
    }
  }

  crea_queue(id, &queue);

  if(recupero){
      //codice recupero
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 0, 0)) != -1) {
    if(strcmp(messaggio.msg_text, "accendi")){
      acceso = TRUE;
    }
    else if(strcmp(messaggio.msg_text, "spegni")){
      acceso = FALSE;
    }
    else if(strcmp(messaggio.msg_text, "get_time")){

    }
  }
  printf("Errore lettura queue BULB\n");
}
