#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
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


//Size funzioni
#define MSG_SIZE 100
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


//Struct
typedef struct pair_int{
    int first;
    int second;
} pair_int;

typedef struct device{
    int id;
    int fd[2];
} device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct int_list{
    int_node * head;
    int n;
} int_list;

typedef struct pair_int_device{
    int integer;
    device * val;
} pair_int_device;

typedef struct device_node{
    device * val;
    struct device_node * next;
} device_node;

typedef struct device_list{
    device_node * head;
    int n;
} device_list;

typedef struct msgbuf{
    long int msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;


int str_split(char * str, char *** rt){
    int i = 0, j = 0, t = 0, c;
    int flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == ' ' || str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != ' ' && str[i-1] != '\n'){
            j++;
        }
        if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    (*rt) = (char **) malloc(sizeof(char *) * j);
    j = 0;
    flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == ' ' || str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != ' ' && str[i-1] != '\n'){
            (*rt)[j] = (char *) malloc(sizeof(char *) * (i-t+1));
            for (c = 0; t+c < i; c++) {
                (*rt)[j][c] = str[t+c];
            }
            (*rt)[j][c] = '\0';
            j++;
        }
        if(str[i] == ' '){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}


int protocoll_parser(char * str, char *** rt){
    int i = 0, j = 0, t = 0, c;
    int flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != '\n'){
            j++;
        }
        if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    (*rt) = (char **) malloc(sizeof(char *) * j);
    j = 0;
    flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != '\n'){
            (*rt)[j] = (char *) malloc(sizeof(char *) * (i-t+1));
            for (c = 0; t+c < i; c++) {
                (*rt)[j][c] = str[t+c];
            }
            (*rt)[j][c] = '\0';
            j++;
        }
        if(str[i] == '\n'){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}


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

long long int int_string(long long int a, char ** str){
  int segno = 1;
  if(a < 0){ // a = |a|
    segno = 0;
    a *= -1;
  }

  long long int i, temp = a;
  for(i = 1; temp > 127;  i++){
    temp = temp/127;
  }
  //printf("%d\n", i);
  *str = (char *) malloc(sizeof(char) * i+2);

  temp =  a;
  (*str)[0] = segno+1; // 1 se a < 0; 2 se a >= 0
  i = 1;
  while (temp > 1) {
    (*str)[i] = (temp % 127) + 1;
    temp =  temp/127;
    //printf("%d\n", (*str)[i]);
    i++;
  }
  (*str)[i] = '\0';
  return (i+1);
}

long long int string_int(char * str){
  int segno = str[0]-1;

  long long int i;
  long long int rt = 0;
  for(i = 0; str[i] != '\0'; i++);
  i--;
  while (i > 0) {
    rt *= 127;
    rt += str[i]-1;
    i--;
  }
  if(segno == 0){
    rt *= -1;
  }
  return rt;
}

void bulb_info_r(char ** str){
    if(str[6][0] == '0'){
        printf("Stato: OFF\n" );
    }
    else{
      printf("Stato: ON\n");
    }
    if(str[7][0] == '0'){
        printf("Interruttore: OFF\n" );
    }
    else{
      printf("Interruttore: ON\n");
    }
    printf("Timer: %lld\n", atoi(str[9]));
    printf("Nome: %s\n", str[11]);

}

 int controlla_validita(char ** str, int id){
   int rt = FALSE;
   if(atoi(str[0]) == CONTROLLER || (atoi(str[0]) == DEFAULT && str[5][0] == '0')){
     printf("k1 %lld\n", atoi(str[3]));
     int i;
     for ( i = 0; str[3][i] != '\0'; i++) {
       printf("> %d - %c\n", str[3][i], str[3][i]);
     };
     if(atoi(str[3]) == id || atoi(str[3]) == 0){
       rt = TRUE;
       printf("k2\n" );
     }
   }
   return rt;
}

void risposta(){
  int queue;
  msgbuf messaggio;
  crea_queue(100,&queue);
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) == -1) {
     printf("Errore\n" );
  }
  else{
    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    int i;
    printf("------\n");
    for(i = 0; i < n; i++){
      printf("> %d\n", strlen(msg[i]));
    }
    printf("------\n");
    if(controlla_validita(msg,100)){
      if(msg[5][0] == '0'){
        char * str = malloc(sizeof(char) * 7);
        str[0] = msg[0][0];
        str[1] = '\0';
        strcat(str,msg[5]);
        str[6] = '\0';
        int codice = atoi(str);
        printf("=> %d\n", codice);
        switch (codice) {
          case 1001:
            bulb_info_r(msg);
            break;

          default: printf("Errore 3\n" );
            break;
        }
      }
      else{
        printf("Errore 2\n" );
      }
    }
    else{
      printf("Errore 1\n" );
    }
  }
}

char * plus_n (char * a) {
  char * b = (char *) malloc (sizeof(char) *(strlen(a)+2));
  strcpy(b,a);
  b[strlen(a)] = '\n';
  b[strlen(a)+1] = '\0';

  return b;
}

void send_prl(char * prl, int dest, int src, int dim, int destid, int srcid, int cod){
  char tmp[20];
  int n = 6;

  for (int i = 0; (i < n); i++) {
    if (i==0) { //Tipologia destinatario - prl[0]
      sprintf(tmp, "%d", dest); //itoa(dest,tmp);
    }
    else if (i==1) { //Tipologia mittente - prl[2]
      sprintf(tmp, "%d", src); //itoa(src,tmp);
    }
    else if (i==2) { //Dim prl (3 char) - prl[4]
      sprintf(tmp, "%d", dim); //itoa(dim,tmp);
    }
    else if (i==3) { //ID destinatario - prl[8]
      sprintf(tmp, "%d", destid); //itoa(destid,tmp);
    }
    else if (i==4) { //ID mittente - prl[19]
      sprintf(tmp, "%d", srcid); //itoa(srcid,tmp);
    }
    else if (i==5) { //Codice - prl[30]
      // I codici che iniziano con lo zero sono un problema
      sprintf(tmp, "%d", cod); //itoa(cod,tmp);
    }
    strcat(prl,plus_n(tmp));
  }

}

void lampadina_troll(){
  int queue;
  crea_queue(100,&queue);
  char * str = (char * ) malloc(sizeof(char) * 100);
  str[0] = '0'; //0
  str[1] = '\n';
  str[2] = '4'; //1
  str[3] = '\n';
  str[4] = '0'; //2
  str[5] = '0';
  str[6] = '\n';
  str[7] =  '0'; //3
  str[8] =  '0';
  str[9] = '\n';
  str[10] =  '0'; //4
  str[11] =  '0'; //4
  str[12] = '\n';
  str[13] = '0'; //5
  str[14] = '1';
  str[15] = '0';
  str[16] = '0';
  str[17] = '1';
  str[18] = '\n';
  str[19] = '1'; //6
  str[20] = '\n';
  str[21] = '0'; //7
  str[22] = '\n';
  str[23] = '1'; //8
  str[24] = '\n';
  str[25] = '4';
  str[26] = '0';
  str[27] = '\n';
  str[28] = '1';
  str[29] = '\n';
  str[30] = 'c';
  str[31] = 'i';
  str[32] = 'a';
  str[33] = 'o';
  str[34] = '\n';
  str[35] = '\0';
  msgbuf messaggio;
  strcpy(messaggio.msg_text, str);
  messaggio.msg_type = 1;
  msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
}

int main() {

  if(fork() == 0){
    risposta();
  }
  else{
    sleep(1);
    lampadina_troll();
  }
    //bulb(1,1);
    /*int queue;
    msgbuf messaggio;
    crea_queue(1, &queue);
    strcpy(messaggio.msg_text,"0");
    messaggio.msg_type = 10;
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);*/
    return 0;
}
