#include "window.h"

/* Operazioni di una WINDOW
WINDOW_OPEN    = 510001
WINDOW_CLOSE   = 510002
WINDOW_GETTIME = 510003
WINDOW_DEL     = 510004
*/

// Interruttori OPEN/CLOSE
// (on/off per aprire/chiudere: tornano subito in “off” dopo essere stati azionati)
int flag;

void sighandle_flag1(int sig){
  flag = 1;
}

void sighandle_flag2(int sig){
  flag = 2;
}

void window_info_r(int id, int s, time_t t){
    printf("Window - %d\n", id);
    //Stato e Tempo di apertura
    if(s == TRUE){
        printf("Stato: OPEN\n" );
        printf("Tempo di apertura: %2lf\n", difftime(time(NULL),t));
    }
    else{
      printf("Stato: CLOSE\n");
      printf("Finestra chiusa\n");
    }
    //Aggiungere campo interruttori ? sempre OFF
}
//Da sistemare
void window_m(char * m, int did, int operazione){
  char * msg;
  crea_messaggio_base(&msg, WINDOW, HUB, did, HUB, operazione);
  strcpy(m,msg);
}

char * recupero_window(char * m, int s, time_t t){
  char * st;
  itoa(s, &st);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * strlen(m) + strlen(st) + 1 + strlen(tt) + 2);
  strcpy(r,m);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,tt);
  strcat(r,"\n");

  return r;
}

int controllo_window(char ** str, int id){
  int rt = FALSE;
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == WINDOW || atoi(str[MSG_TYPE_DESTINATARIO]) == 0){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
  }
  return rt;
}

void window(int id, int recupero){
  int status = FALSE;
  time_t t_start = 0; //Tempo per il quale è rimasta aperta
  char nome[] = "WINDOW-ID";
  int idf1, idf2;

  int queue;
  msgbuf messaggio;

  if((idf1 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [OPEN]  => SIGUSR1\n  [CLOSE] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //apri
        flag = 0;
        window_m(messaggio.msg_text, id, WINDOW_OPEN);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
      else if(flag == 2){ //chiudi
        flag = 0;
        window_m(messaggio.msg_text, id, WINDOW_CLOSE);
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
      if(flag == 1){ //time
        flag = 0;
        window_m(messaggio.msg_text, id, WINDOW_GETTIME);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
      else if(flag == 2){ //kill
        flag = 0;
        window_m(messaggio.msg_text, id, WINDOW_DEL);
        send_message(queue, &messaggio, messaggio.msg_text, 2);
      }
    }
  }

  create_queue(id, &queue);
  char ** msg;

  if(recupero){
    printf("inizio\n" );
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      printf("%s\n", messaggio.msg_text);
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[MSG_OP + 1]);
      t_start = atoi(msg[MSG_OP + 2]);
    }
    printf("fine\n" );
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 2, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, &msg);

    if(atoi(msg[MSG_OP]) == MSG_INF && controllo_window(msg,id)) { //richiesta info su me stesso
      window_info_r(id, status, t_start);
    }
    else if (atoi(msg[MSG_OP]) >= 10000 && controllo_window(msg,id)) { // Richieste specifiche
      switch (codice_messaggio(msg)) {
        case WINDOW_OPEN:
          if(!status){
            t_start = time(NULL);
          }
          status = TRUE;
          printf("La finestra è stata aperta\n");
            break;
        case WINDOW_CLOSE:
          status = FALSE;
          printf("La finestra è stata chiusa\n");
            break;
        case WINDOW_GETTIME:
          if(status == TRUE){
            printf("Tempo di apertura: %2lf\n", difftime(time(NULL),t_start));
          }
          else{
            printf("Apri la finestra!\n");
          }
          break;
        case WINDOW_DEL:
          send_message(queue, &messaggio, recupero_window(messaggio.msg_text,status,t_start), 10);
          printf("Finestra pronta per essere eliminata\n");
          printf("%s\n", messaggio.msg_text);
          kill(idf1, SIGTERM);
          kill(idf2, SIGTERM);
          exit(0);
        default: printf("Errore nello switch\n" ); //Gestisci errore
          break;
      }

    }
  }
  printf("Errore lettura queue WINDOW\n");
}

//msgctl( queue, IPC_RMID, 0); - Serve a svuotare la queue
