#include "window.h"

// Interruttori OPEN/CLOSE
// (on/off per aprire/chiudere: tornano subito in “off” dopo essere stati azionati)
int flag;

void sighandle_flag1(int sig){
  flag = 1;
}

void sighandle_flag2(int sig){
  flag = 2;
}

void window(int id, int recupero){
  int status = FALSE;
  //int open = FALSE;
  //int close = FALSE;
  time_t t_start = 0; //Tempo per il quale è rimasta aperta
  char nome[] = "WINDOW-ID";
  int idf1, idf2;

  int queue;
  msgbuf messaggio;

  if((idf1 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    //printf("\n----------------\npid_signal: %d\n  [OPEN]  => SIGUSR1\n  [CLOSE] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //apri
        flag = 0;
        send_message(queue, &messaggio, "OPEN", 1);
        //printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //chiudi
        flag = 0;
        send_message(queue, &messaggio, "CLOSE", 1);
        //printf("=> %s\n", messaggio.msg_text);
      }
    }
  }
  else if((idf2 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    //printf("\n----------------\npid_signal: %d\n  [getTime] => SIGUSR1\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //time
        flag = 0;
        send_message(queue, &messaggio, "time", 1);
        //printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //kill
        flag = 0;
        send_message(queue, &messaggio, "RIP", 1);
      }
    }
  }

  create_queue(id, &queue);

  if(recupero){
    //printf("inizio\n" );
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        //printf("errore lettura ripristino\n");
    }
    else{
      status = messaggio.msg_text[0]-'0';
      char ** rt;
      str_split(messaggio.msg_text, &rt);
      t_start = atoi(rt[1]);
    }
    //printf("fine\n" );
  }

  //inizio loop
  //printf("\n\nstatus: %ld\n", status);
  //printf("time: %ld\n", t_start);
  //printf("fine\n" );
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    //printf("\n * Livello: %ld\n", messaggio.msg_type );
    if(strcmp(messaggio.msg_text, "OPEN") == 0){
      if(!status){
        t_start = time(NULL);
      }
      status = TRUE;
      //printf("OPEN\n" );
    }
    else if(strcmp(messaggio.msg_text, "CLOSE") == 0){
      status = FALSE;
      //printf("CLOSE\n" );
    }
    else if(strcmp(messaggio.msg_text, "time") == 0){
      if(status == TRUE){
        //printf("%2lf\n", difftime(time(NULL),t_start));
      }
      else{
        //printf("Scusa non riesco a leggere, apri la finestra\n");
      }
      //printf("time\n" );
    }
    else if(strcmp(messaggio.msg_text, "RIP") == 0){
      messaggio.msg_text[0] = '0' + status;
      messaggio.msg_text[1] = ' ';
      messaggio.msg_text[2] = '\0';
      char str[20];
      sprintf(str, "%d" , t_start);
      strcat(messaggio.msg_text, str);
      send_message(queue, &messaggio, messaggio.msg_text, 10);
      kill(idf1, SIGTERM);
      kill(idf2, SIGTERM);
      exit(0);
    }
    //printf("\n\nstatus: %ld\n", status);
    //printf("time: %ld\n", t_start);
    //printf("fine\n" );
  }
  //printf("Errore lettura queue WINDOW\n");
}
