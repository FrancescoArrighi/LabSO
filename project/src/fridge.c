#include "fridge.h"
//msg.type = 1 => ricevi
//msg.type = 2 => invio

void fridge(int id, int recupero, char *nome){ //recupero booleano
  t_frigo frigo;
  frigo.id = id;
  frigo.stato = FALSE;
  frigo.interruttore = FALSE;
  frigo.termostato = 3;// temperatura interna
  frigo.time = 0;
  frigo.nome = strdup(nome);
  frigo.delay = 5; //tempo di chiusura automatica
  frigo.percentuale = 0; // percentuale di riempimento
  int allarme = -1; // figlio per gestione alarme
  time_t t_start = 0; // tempo usato per calcolare eventuale tempo di apertura
  pid_t idf = -1; //pid del figlio che gestisce fifo

  int queue;
  msgbuf messaggio;

  create_queue(id, &queue);
  printf("Id frigo: %d\n", frigo.id);

  if(recupero){
    printf("inizio\n" );
    if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
      printf("errore lettura ripristino\n");
    }
    else{
      printf("recupero\n");
      int new_delay;
      char ** msg;
      protocoll_parser(messaggio.msg_text, &msg);
      // da aggiungere header
      frigo.stato = atoi(msg[MSG_FRIDGE_STATO]);
      frigo.interruttore = atoi(msg[MSG_FRIDGE_INTERRUTTORE]);
      frigo.termostato = atoi(msg[MSG_FRIDGE_TERM]);
      frigo.time = atoi(msg[MSG_FRIDGE_TIME]);
      frigo.delay = atoi(msg[MSG_FRIDGE_DELAY]);
      frigo.percentuale = atoi(msg[MSG_FRIDGE_PERC]);
      frigo.nome = msg[MSG_FRIDGE_NOME];
      t_start = atoi(msg[MSG_FRIDGE_TSTART]);
      frigo.id = atoi(msg[MSG_FRIDGE_ID]);
      if(frigo.stato){ //se frigo è aperto
      	new_delay = frigo.delay - frigo.time; //recupero delay rimanente
      	apri_frigo(&frigo, &t_start, &allarme, new_delay);
      }
      /*
      printf("Id frigo: %d\n", frigo.id);
      printf("stato: %d\n", frigo.stato);
      printf("Interruttore: %d\n", frigo.interruttore);
      printf("termostato: %d\n", frigo.termostato);
      printf("time: %d\n", frigo.time);
      printf("delay: %d\n", frigo.delay);
      printf("percentuale: %d\n", frigo.percentuale);
      printf("nome: %s\n", frigo.nome);
      printf("new delay: %d\n", new_delay);
      */

      //msgctl(queue, IPC_RMID, NULL); //rimuovo la coda precedentemente creata
    }
    printf("fine\n" );
  }

  if((idf = fork()) < 0){
    perror("Errore fork");
    exit(1);
  }
  else if(idf == 0){
    char buf_r[50];
    int fd, n_arg, flag = TRUE;
    char **cmd, *str;

    while(flag){
      fd = open(FIFO, O_RDONLY);
      read(fd, buf_r, 50);
      printf("%s\n", buf_r);
      n_arg = str_split(buf_r, &cmd);
      if((strcmp(cmd[0], frigo.nome) == 0) && (n_arg >= 3)){
        if(strcmp(cmd[1], "interruttore") == 0){
          crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_INTERRUTTORE);
          strcat(str, cmd[2]);
          strcat(str, "\n\0");
        }
        else if(strcmp(cmd[1], "delay") == 0){
          crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_DELAY);
          strcat(str, cmd[2]);
          strcat(str, "\n\0");
        }
        else if(strcmp(cmd[1], "perc") == 0){
          crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_PERC);
          strcat(str, cmd[2]);
          strcat(str, "\n\0");
        }
        else if(strcmp(cmd[1], "termostato") == 0){
          crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_TERMOSTATO);
          strcat(str, cmd[2]);
          strcat(str, "\n\0");
        }
        send_message(queue, &messaggio, str, 4);
      }
      else if(strcmp(cmd[0], "exit") == 0){
        printf("Fine lettura\n");
        flag = FALSE;
      }
      close(fd);
    }
  }
  else{
    //inizio loop
    while((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 0, 0)) != -1) {
      char ** msg;
      protocoll_parser(messaggio.msg_text, &msg);

      if(atoi(msg[MSG_OP]) == MSG_INF && controlla_fridge(msg, frigo.id)){
        if(t_start > 0){
          frigo.time = difftime(time(NULL), t_start);
        }
        send_info_fridge(msg, &frigo);
      }
      else if(atoi(msg[MSG_OP]) == FRIDGE_KILL && controlla_fridge(msg, frigo.id)){
        if(allarme > 0){
          kill(allarme, SIGTERM);
        }
        if(idf > 0){
          kill(idf, SIGTERM);
        }
        exit(EXIT_SUCCESS);
      }
      else if(atoi(msg[MSG_OP]) > 10000 && controlla_fridge(msg, frigo.id)){
      	int valore = atoi(msg[5]); //valore da modificare, es. stato, temperatura
      	int codice = codice_messaggio(msg);
      	printf("=>codice2: %d\n", codice);
      	switch(codice){
      	case FRIDGE_SET_STATO: // modifica stato
      	  set_stato(valore, &frigo, &t_start, &allarme);
      	  printf("Stato: %d\n", frigo.stato);
      	  break;
      	case FRIDGE_SET_INTERRUTTORE:
      	  set_interruttore(valore, &frigo, &t_start, &allarme);
      	  printf("Interruttore: %d\n", frigo.interruttore);
      	  break;
      	case FRIDGE_SET_TERMOSTATO:
      	  set_termostato(valore, &frigo);
      	  printf("Termostato: %d\n", frigo.termostato);
      	  break;
      	case FRIDGE_SET_DELAY:
      	  set_delay(valore, &frigo);
      	  printf("Delay: %d\n", frigo.delay);
      	  break;
      	case FRIDGE_SET_PERC:
      	  set_perc(valore, &frigo);
      	  printf("Percentuale: %d\n", frigo.percentuale);
      	  break;
      	case FRIDGE_RECUPERO: // modalità recupero
      	  duplicate(&frigo, t_start);
          char *msg_kill;
      	  crea_messaggio_base(&msg_kill, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_KILL);
          send_message(queue, &messaggio, msg_kill, 1);
      	  break;
      	default:
      	  printf("Errore codice istruzione!\n");
      	  break;
        }
      }
    }
  }
}


void send_info_fridge(char **msg, t_frigo *frigo){ //invia info
  printf("Send info\n");
  int queue;
  msgbuf messaggio;
  char *str;
  int cod_op;

  cod_op = codice_messaggio(msg) + 1000; //+1000 serve per specificare che si tratta di un messaggio di risposta
  crea_messaggio_base(&str, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo->id, cod_op);
  strcat(str, msg[MSG_INF_IDPADRE]); //concat id padre
  strcat(str, "\n\0");
  concat_dati(str, frigo); //concateno i dati del frigo

  printf("new msg: \n%s\n", str);
  create_queue(atoi(msg[MSG_ID_MITTENTE]), &queue); //stabilisco una coda col padre
  send_message(queue, &messaggio, str, 2);
  printf("End send\n");
}


void apri_frigo(t_frigo *frigo, time_t *t_start, int *allarme, int delay_recupero){
  frigo->stato = TRUE;
  if(!delay_recupero){ //se non sono in recupero
    time(t_start); //salvo tempo inizio apertura
  }

  if((*allarme = fork()) == 0){ //inizializzo un timer
    if(!delay_recupero){ //se non sono in recupero
      printf("Chiusura automatica fra %d secondi\n", frigo->delay);
      sleep(frigo->delay); //chiusura dopo [delay] secondi
    }
    else{ // altrimenti
      sleep(delay_recupero); // chiusura dopo [delay_recupero] secondi
    }

    int queue;
    msgbuf messaggio;
    char * str = (char * ) malloc(sizeof(char) * 100);
    char * id_frigo = (char * ) malloc(sizeof(char) * 10);
    sprintf(id_frigo, "%d", frigo->id);
    create_queue(frigo->id, &queue);
    //modifica
    crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo->id, frigo->id, FRIDGE_SET_STATO); //invio un messaggio a se stesso...
    strcat(str, "0\n\0"); //...con stato FALSE
    send_message(queue, &messaggio, str, 1);
    exit(0);
  }
}

void chiudi_frigo(t_frigo *frigo, time_t *t_start, int *allarme){
  frigo->stato = FALSE;
  *t_start = 0; //resetto tempo di inizio apertura
  frigo->time = 0; //resetto eventuale tempo di apertura

  if(*allarme >= 0){ //elimino eventuale timer
    kill(*allarme, SIGTERM);
    *allarme = -1;
  }
}

void set_stato(int valore, t_frigo *frigo, time_t *t_start, int *allarme){ // valore: nuovo stato [1-apri], [0-chiudi]
  printf("----Setstato\n");
  if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    apri_frigo(frigo, t_start, allarme, 0);
    printf("fridge aperto con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ // chiudo un frigo aperto
    chiudi_frigo(frigo, t_start, allarme);
    printf("fridge chiuso con successo\n");
  }
}

void set_interruttore(int valore, t_frigo *frigo, time_t *t_start, int *allarme){
   printf("----Set interruttore\n");
   if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    frigo->interruttore = TRUE;
    apri_frigo(frigo, t_start, allarme, 0);
    printf("fridge aperto manualmente con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ //chiudo un frigo aperto
    frigo->interruttore = FALSE;
    chiudi_frigo(frigo, t_start, allarme);
    printf("fridge chiuso munualmente con successo\n");
  }
}

void set_termostato(int valore, t_frigo *frigo){
  printf("Cambio Temperatura interna\n");

  frigo->termostato = valore;

  printf("Temperatura combiata con successo\n");
}

void set_delay(int valore, t_frigo *frigo){
  printf("Cambio delay\n");
  if(valore > 0){
    frigo->delay = valore;
  }
  else{
    printf("Errore: delay negativo\n");
  }

  printf("Delay cambiato con successo\n");
}

void set_perc(int valore, t_frigo *frigo){
  int new_perc;
  new_perc = frigo->percentuale + valore;
  if(!frigo->stato){
    printf("Apri prima il frigo!\n");
  }else if((new_perc >= 0) && (new_perc <= 100)){
    printf("Percentuale di riempimento cambiata con successo\n");
    frigo->percentuale = new_perc;
  }
  else if(new_perc > 100){
    printf("Errore: frigo troppo pieno\n");
  }
  else if(new_perc < 0){
    printf("Errore: percentuale di riempimento sotto zero\n");
  }
}

void duplicate(t_frigo *frigo, time_t t_start){
  printf("---------Duplicate--------\n");
  int queue;
  msgbuf messaggio;
  char * str = (char * ) malloc(sizeof(char) * 100);
  char *tmp = (char*) malloc(sizeof(char)*20);

  crea_messaggio_base(&str, 6, 6, frigo->id, frigo->id, 10006); //creo header
  sprintf(tmp, "%ld", t_start);
  if(t_start > 0){
    frigo->time = difftime(time(NULL), t_start);
  }
  concat_dati(str, frigo);
  strcat(str, tmp); //concateno t_start
  strcat(str, "\n");
  sprintf(tmp, "%d", frigo->id);
  strcat(str, tmp); //concateno id
  strcat(str, "\n\0");
  create_queue(frigo->id, &queue); //invio il messaggio al nuovo frigo duplicato con id tmp valore
  send_message(queue, &messaggio, str, 10);
  printf("-------Fine---------\n");
}

char * plus_only_n (char * a) {
  char * b = (char *) malloc (sizeof(char) *(strlen(a)+1));
  strcpy(b,a);
  b[strlen(a)] = '\n';

  return b;
}

void concat_dati(char* prl, t_frigo *frigo){ //concateno dati da inviar
  int i;
  char* buf = (char*) malloc(sizeof(char)*20);
  for(i=0; i<7; i++){
    switch(i){
    case 0:
      sprintf(buf, "%d", frigo->stato);
      break;
    case 1:
      sprintf(buf, "%d", frigo->interruttore);
      break;
    case 2:
      sprintf(buf, "%d", frigo->termostato);
      break;
    case 3:
      sprintf(buf, "%d", frigo->time);
      break;
    case 4:
      sprintf(buf, "%d", frigo->delay);
      break;
    case 5:
      sprintf(buf, "%d", frigo->percentuale);
      break;
    case 6:
      strcpy(buf, frigo->nome);
      break;
    default:
      printf("Argomento non valido");
      break;
    }
    strcat(prl, plus_only_n(buf));
  }
  strcat(prl, "\0");
}

int controlla_fridge(char ** str, int id){ // controllo se sono io il destinatario
  int rt = FALSE;
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == FRIDGE || (atoi(str[MSG_TYPE_DESTINATARIO]) == DEFAULT && atoi(str[MSG_OP]) < 10000)){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
  }
  return rt;
}
