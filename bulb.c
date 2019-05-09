//Header
#include "bulb.h"

int flag;

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
  int idf1, idf2;

  int queue;
  msgbuf messaggio;

  if((idf1 = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    //printf("\n----------------\npid_signal: %d\n  [ON]  => SIGUSR1\n  [OFF] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        send_message(queue, &messaggio, "ON", 1);
        //printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        send_message(queue, &messaggio, "OFF", 1);
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
      if(flag == 1){ //accendi
        flag = 0;
        send_message(queue, &messaggio, "time", 1);
        //printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //accendi
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
      stato = messaggio.msg_text[0]-'0';
      interruttore = messaggio.msg_text[1]-'0';
      char ** rt;
      str_split(messaggio.msg_text, &rt);
      t_start = atoi(rt[1]);
    }
    //printf("fine\n" );
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    //printf("\n * Livello: %ld\n", messaggio.msg_type );
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
        //printf("%2lf\n", difftime(time(NULL),t_start));
      }
      else{
        //printf("Scusa non riesco a leggere, accendi la luce\n");
      }
      //printf("time\n" );
    }
    else if(strcmp(messaggio.msg_text, "RIP") == 0){
      messaggio.msg_text[0] = '0' + stato;
      messaggio.msg_text[1] = '0' + interruttore;
      messaggio.msg_text[2] = ' ';
      messaggio.msg_text[3] = '\0';
      char str[20];
      sprintf(str, "%d" , t_start);
      strcat(messaggio.msg_text, str);
      send_message(queue, &messaggio, messaggio.msg_text, 10);
      kill(idf1, SIGTERM);
      kill(idf2, SIGTERM);
      exit(0);
    }
    //printf("\n\ninterruttore: %d\n", interruttore);
    //printf("stato: %d\n", stato);
    //printf("time: %ld\n", t_start);
  }
  //printf("Errore lettura queue BULB\n");
}
