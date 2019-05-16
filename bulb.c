//Header
#include "bulb.h"

/*Operazioni di una BULB
BULB_INVERTI_S = 410001
BULB_INVERTI_I = 410002
BULB_GETTIME   = 410003
BULB_DEL       = 410004
BULB_KILL      = 410005
*/

int flag;

void sighandle_flag1(int sig){
  flag = 1;
}

void sighandle_flag2(int sig){
  flag = 2;
}

void bulb_info_r(int id, int s, int i, time_t t){
    printf("Bulb - %d\n", id);
    //Interruttore
    if(i == TRUE){
        printf("Interruttore: ON\n" );
    }
    else{
      printf("Interruttore: OFF\n");
    }
    //Stato e Tempo di accensione
    if(s == TRUE){
        printf("Stato: ON\n" );
        printf("Tempo di utilizzo: %2lf\n", difftime(time(NULL),t));
    }
    else{
      printf("Stato: OFF\n");
      printf("Lampadina spenta\n");
    }
}

char * recupero_bulb(char * m, int s, int i, time_t t){
  char * st;
  itoa(s, &st);
  char * it;
  itoa(i, &it);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * strlen(m) + strlen(st) + 1 + strlen(it) + 1 + strlen(tt) + 2);
  strcpy(r,m);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,it);
  strcat(r,"\n");
  strcat(r,tt);
  strcat(r,"\n");

  return r;
}
//Non finita
void bulb_m(char * m, int did, int operazione){
  char * msg;
  crea_messaggio_base(&msg, BULB, HUB, did, HUB, operazione);
  strcpy(m,msg);
}

int controllo_bulb(char ** str, int id){
  int rt = FALSE;
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == BULB || atoi(str[MSG_TYPE_DESTINATARIO]) == 0){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
  }
  return rt;
}

/* Funzione Bulb */
void bulb(int id, int recupero){ //recupero booleano
  int status = FALSE;
  int interruttore = FALSE;
  time_t t_start = 0;
  char nome[] = "BULB-ID";
  int idf1, idf2;

  int queue;
  msgbuf messaggio;

  if((idf1 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [INVERTI status]  => SIGUSR1\n  [INVERTI INTERRUTTORE] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        bulb_m(messaggio.msg_text, id, BULB_INVERTI_S);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        bulb_m(messaggio.msg_text, id, BULB_INVERTI_I);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
    }
  }
  else if((idf2 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [getTime] => SIGUSR1\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //Get time
        flag = 0;
        bulb_m(messaggio.msg_text, id, BULB_GETTIME);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
      else if(flag == 2){ //ESC
        flag = 0;
        bulb_m(messaggio.msg_text, id, BULB_DEL);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
    }
  }

  create_queue(id, &queue);
  char ** msg;

  if(recupero){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[MSG_OP + 1]);
      interruttore = atoi(msg[MSG_OP + 2]);
      t_start = atoi(msg[MSG_OP + 3]);
    }
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 2, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, &msg);

    if(atoi(msg[MSG_OP]) == MSG_INF && controllo_bulb(msg,id)) { //richiesta info su me stesso
      bulb_info_r(id, status, interruttore, t_start);
    }
    else if (atoi(msg[MSG_OP]) >= 10000 && controllo_bulb(msg,id)) { // Richieste specifiche
      switch (codice_messaggio(msg)) {
        case BULB_INVERTI_S:
          if (status == FALSE) {
            status = TRUE;
            t_start = time(NULL);
            printf("La vostra lampadina è stata accesa\n");
          }
          else {
            status = FALSE;
            printf("La vostra lampadina è stata spenta\n");
          }
          break;
        case BULB_INVERTI_I:
          if (interruttore == FALSE) {
            interruttore = TRUE;
            if(!status){
              status = TRUE;
              t_start = time(NULL);
            }
            printf("Interruttore su ON\n");
          }
          else {
            interruttore = FALSE;
            if(status){
              status = FALSE;
            }
            printf("Interruttore su OFF\n");
          }
          break;
        case BULB_GETTIME:
          if(status == TRUE){
            printf("Tempo di utilizzo: %2lf\n", difftime(time(NULL),t_start));
          }
          else{
            printf("Accendi la luce!\n");
          }
          break;
        case BULB_DEL:
          send_message(queue, &messaggio, recupero_bulb(messaggio.msg_text,status, interruttore, t_start), 10);
          printf("Lampadina pronta per essere eliminata\n");
          break;
        case BULB_KILL:
          kill(idf1, SIGTERM);
          kill(idf2, SIGTERM);
          exit(0);
          break;
        default: printf("Errore nello switch\n" ); //Gestisci errore
          break;
      }

    }
  }
  printf("Errore lettura queue BULB\n");
}

//msgctl( queue, IPC_RMID, 0); - Serve a svuotare la queue
