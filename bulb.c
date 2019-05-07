//Header globale
#include "global.h"

//Struct
typedef struct msgbuf{
    long msg_type;
    char msg_text[100];
} msgbuf;

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

void sighandle_flag1(int sig){
  flag = 1;
}

void sighandle_flag2(int sig){
  flag = 2;
}

void bulb(int id, int recupero){ //recupero booleano
  int stato = FALSE;
  int interruttore = FALSE;
  time_t t_start = 0;
  char nome[] = "BULB-ID";


  int queue;
  msgbuf messaggio;

  if(fork() == 0){// codice figlio
    flag = 0;
    crea_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [ON]  => SIGUSR1\n  [OFF] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "ON");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        strcpy(messaggio.msg_text, "OFF");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
    }
  }
  else if(fork() == 0){// codice figlio
    flag = 0;
    crea_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [getTime] => SIGUSR1\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "time");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
      else if(flag == 2){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "RIP");
        messaggio.msg_type = 1;
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
    }
  }

  crea_queue(id, &queue);

  if(recupero){
    printf("start\n" );
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      stato = messaggio.msg_text[0]-'0';
      interruttore = messaggio.msg_text[1]-'0';
      char ** rt;
      str_split(messaggio.msg_text, &rt);
      t_start = atoi(rt[1]);
    }
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 0, 0)) != -1) {
    //printf("accendi? %d, (%s)\n", strcmp(messaggio.msg_text, "accendi"), messaggio.msg_text);
    printf("ok\n" );
    if(strcmp(messaggio.msg_text, "accendi") == 0){
      stato = TRUE;
      t_start = time(NULL);
      //printf("accendi\n" );
    }
    else if(strcmp(messaggio.msg_text, "spegni") == 0){
      stato = FALSE;
      //printf("spegni\n" );
    }
    else if(strcmp(messaggio.msg_text, "ON") == 0){
      interruttore = TRUE;
      if(!stato){
        stato = TRUE;
        t_start = time(NULL);
      }
      //printf("ON\n" );
    }
    else if(strcmp(messaggio.msg_text, "OFF") == 0){
      interruttore = FALSE;
      if(stato){
        stato = FALSE;
      }
      //printf("OFF\n" );
    }
    else if(strcmp(messaggio.msg_text, "time") == 0){
      if(stato == TRUE){
        printf("%2lf\n", difftime(time(NULL),t_start));
      }
      else{
        printf("Scusa non riesco a leggere, accendi la luce\n");
      }
      //printf("time\n" );
    }
    else if(strcmp(messaggio.msg_text, "RIP") == 0){
      messaggio.msg_type = 10;
      messaggio.msg_text[0] = '0' + stato;
      messaggio.msg_text[1] = '0' + interruttore;
      messaggio.msg_text[2] = ' ';
      messaggio.msg_text[3] = '\0';
      char str[20];
      sprintf(str, "%d" , t_start);
      strcat(messaggio.msg_text, str);
      msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
    }
    printf("\n\ninterruttore: %d\n", interruttore);
    printf("stato: %d\n", stato);
    printf("time: %d\n", t_start);
  }
  printf("Errore lettura queue BULB\n");
}
