#include "fridge.h"
//Tips per terminale: ipcs -q => code di messaggi attive
// ipcrm -q <msqid> => rimuove la coda
// rm /tmp/D_* => rimuove tutti i file che inizia in questo modo

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
      frigo.stato = atoi(msg[FRIDGE_REC_STATO]);
      frigo.interruttore = atoi(msg[FRIDGE_REC_INTERRUTTORE]);
      frigo.termostato = atoi(msg[FRIDGE_REC_TERM]);
      frigo.time = atoi(msg[FRIDGE_REC_TIME]);
      frigo.delay = atoi(msg[FRIDGE_REC_DELAY]);
      frigo.percentuale = atoi(msg[FRIDGE_REC_PERC]);
      frigo.nome = msg[FRIDGE_REC_NOME];
      t_start = atoi(msg[FRIDGE_REC_TSTART]);
      frigo.id = atoi(msg[FRIDGE_REC_ID]);
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
    printf("Inizio comunicazione\n");
    char *fifo_w = (char *) malloc(sizeof(char)*20);
    char *fifo_r = (char *) malloc(sizeof(char)*20);
    sprintf(fifo_w, "/tmp/D_%d_W", frigo.id);
    sprintf(fifo_r, "/tmp/D_%d_R", frigo.id);
    char buf_r[BUF_SIZE], buf_w[BUF_SIZE];
    int fd_write, fd_read, n_arg, flag = TRUE, richiesta;
    char **cmd, *str;

    if((mkfifo(fifo_w, 0666) == -1) && (errno != EEXIST)){ //se path esiste già continuo normalmente
      perror("Errore mkfifo");
      exit(1);
    }
    if((mkfifo(fifo_r, 0666) == -1) && (errno != EEXIST)){
      perror("Errore mkfifo");
      exit(1);
    }

    while(flag){
      printf("----Inizio lettura -----\n");
      richiesta = 0;
      fd_read = open(fifo_r, O_RDONLY);
      read(fd_read, buf_r, BUF_SIZE);
      n_arg = str_split(buf_r, &cmd);
      if((n_arg == 4) && strcmp(cmd[1], "set") == 0){
        printf("richiesta set\n");
        printf("valore: %s\n", cmd[3]);
        if(strcmp(cmd[2], "interruttore") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_INTERRUTTORE);
          concat_string(&messaggio, cmd[3]);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "delay") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_DELAY);
          concat_string(&messaggio, cmd[3]);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "percentuale") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_PERC);
          concat_string(&messaggio, cmd[3]);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "termostato") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_SET_TERMOSTATO);
          concat_string(&messaggio, cmd[3]);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          printf("invio messaggio\n");
        }
        else{
          printf("Operazione non valida\n");

        }
      }
      else if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){
        if((strcmp(cmd[2], "info") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_INF);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = MSG_INF;
        }
        else if((strcmp(cmd[2], "time") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_TIME);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = FRIDGE_GET_TIME;
        }
        else if((strcmp(cmd[2], "stato") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_STATO);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = FRIDGE_GET_STATO;
        }
        else if((strcmp(cmd[2], "delay") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_DELAY);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = FRIDGE_GET_DELAY;
        }
        else if((strcmp(cmd[2], "percentuale") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_PERC);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = FRIDGE_GET_PERC;
        }
        else if((strcmp(cmd[2], "termostato") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_TERMOSTATO);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE); //invio la richiesta di info al frigo
          richiesta = FRIDGE_GET_TERMOSTATO;
        }

        if(richiesta > 0 && ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 5, 0)) != -1)){ //quando ricevo la risposta
          fd_write = open(fifo_w, O_WRONLY); //apro fifo di scrittura
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 40);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          switch (richiesta) {
            case MSG_INF:
              sprintf(str_temp, "Nome: %s\n", info_response[FRIDGE_INF_NOME]); //concateno i dati ricevuti
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Id: %s\n", info_response[MSG_ID_MITTENTE]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Stato: %s\n", info_response[FRIDGE_INF_STATO]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Interruttore: %s\n", info_response[FRIDGE_INF_INTERRUTTORE]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Temperatura interna: %s\n", info_response[FRIDGE_INF_TERM]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Time: %s\n", info_response[FRIDGE_INF_TIME]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Delay: %s\n", info_response[FRIDGE_INF_DELAY]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Percentuale di riempimento: %s\n", info_response[FRIDGE_INF_PERC]);
              strcat(buf_w, str_temp);
              break;
            case FRIDGE_GET_TIME:
              sprintf(str_temp, "Tempo di apertura: %s\n", info_response[FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case FRIDGE_GET_PERC:
              sprintf(str_temp, "Percentuale di riempimento: %s\n", info_response[FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case FRIDGE_GET_TERMOSTATO:
              sprintf(str_temp, "Temperatura interna: %s\n", info_response[FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case FRIDGE_GET_DELAY:
              sprintf(str_temp, "Delay: %s\n", info_response[FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case FRIDGE_GET_STATO:
              sprintf(str_temp, "Stato: %s\n", info_response[FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            default:
              break;
          }
          write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
        }

      }
      else if(strcmp(cmd[1], "close") == 0){ //se il secondo parametro è close
        printf("Chiusura comunicazione\n");
        flag = FALSE; //esco dal ciclo
      }
      else{
        printf("Comando non valido\n");
        flag = FALSE;
      }
      close(fd_read); //chiudo fifo
      close(fd_write);
    }
    unlink(fifo_r); //una volta uscita dal ciclo elimino file fifo
    unlink(fifo_w);
  }
  else{
    //inizio loop
    while((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0)) != -1) {
      char ** msg;
      protocoll_parser(messaggio.msg_text, &msg);

      if(atoi(msg[MSG_OP]) == MSG_INF && controlla_fridge(msg, frigo.id) && (atoi(msg[MSG_ID_MITTENTE]) != frigo.id)){ //se è una richiesta info
        if(frigo.stato){
          frigo.time = difftime(time(NULL), t_start);
        }
        send_info_fridge(msg, &frigo);
      }
      else if((atoi(msg[MSG_OP]) == MSG_INF) && (atoi(msg[MSG_ID_MITTENTE]) == frigo.id)){ //se è una richiesta info su se stesso
        printf("Richiesta info da figlio\n");
        if(frigo.stato){
          frigo.time = difftime(time(NULL), t_start);
        }
        crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_INF);
        concat_int(&messaggio, -1); // id_padre, in questo nessuno
        concat_dati(&messaggio, &frigo);
        printf("Invio il messaggio al figlio\n%s", messaggio.msg_text);
        send_message(queue, &messaggio, messaggio.msg_text, 5); //priorità da decidere
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
      else if(atoi(msg[MSG_OP]) < 10000 && controlla_fridge(msg, frigo.id)){
        printf("Ricevuto una richiesta con codice op minore di 10000\n");
        int codice = codice_messaggio(msg);
        printf("codice: %d\n", codice);
        printf("FRIDGE_GET_DELAY: %d\n", FRIDGE_GET_DELAY);
        switch(codice){
          case FRIDGE_GET_TIME:
            if(frigo.stato == TRUE){
              frigo.time = difftime(time(NULL), t_start);
            }
            crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_TIME);
            concat_int(&messaggio, frigo.time);
            break;
          case FRIDGE_GET_PERC:
            crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_PERC);
            concat_int(&messaggio, frigo.percentuale);
            break;
          case FRIDGE_GET_TERMOSTATO:
            crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_TERMOSTATO);
            concat_int(&messaggio, frigo.termostato);
            break;
          case FRIDGE_GET_STATO:
            crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_STATO);
            concat_int(&messaggio, frigo.stato);
            break;
          case FRIDGE_GET_DELAY:
            printf("La richiesta è di tipo get delay\n");
            crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_GET_DELAY);
            concat_int(&messaggio, frigo.delay);
            printf("Delay: %d\n", frigo.delay);
            printf("%s\n", messaggio.msg_text);
            break;
          default:
            break;
        }
        send_message(queue, &messaggio, messaggio.msg_text, 5);
        printf("Risposta\n%s\n", messaggio.msg_text);
        printf("Risposta inviata al figlio\n");
      }
      else if(atoi(msg[MSG_OP]) > 10000 && controlla_fridge(msg, frigo.id)){
      	int valore = atoi(msg[FRIDGE_VALORE]); //valore da modificare, es. stato, temperatura
      	int codice = codice_messaggio(msg);
      	printf("=>codice2: %d\n", codice);
      	switch(codice){
      	case FRIDGE_SET_STATO: // modifica stato
      	  set_stato(valore, &frigo, &t_start, &allarme);
      	  printf("Stato attuale: %d\n", frigo.stato);
      	  break;
      	case FRIDGE_SET_INTERRUTTORE:
      	  set_interruttore(valore, &frigo, &t_start, &allarme);
      	  printf("Interruttore attuale: %d\n", frigo.interruttore);
      	  break;
      	case FRIDGE_SET_TERMOSTATO:
      	  set_termostato(valore, &frigo);
      	  printf("Termostato attuale: %d\n", frigo.termostato);
      	  break;
      	case FRIDGE_SET_DELAY:
      	  set_delay(valore, &frigo);
      	  printf("Delay attuale: %d\n", frigo.delay);
      	  break;
      	case FRIDGE_SET_PERC:
      	  set_perc(valore, &frigo);
      	  printf("Percentuale attuale: %d\n", frigo.percentuale);
      	  break;
      	case FRIDGE_RECUPERO: // modalità recupero
          // da sistemare
          duplicate(&frigo, t_start);
      	  crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, FRIDGE_KILL);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
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
  int cod_op;

  cod_op = codice_messaggio(msg);
  crea_messaggio_base(&messaggio, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo->id, cod_op);
  concat_string(&messaggio, msg[MSG_INF_IDPADRE]);
  concat_dati(&messaggio, frigo); //concateno i dati del frigo
  create_queue(atoi(msg[MSG_ID_MITTENTE]), &queue); //stabilisco una coda col padre
  send_message(queue, &messaggio, messaggio.msg_text, 2);
  printf("End send\n");
}


void apri_frigo(t_frigo *frigo, time_t *t_start, int *allarme, int delay_recupero){
  frigo->stato = TRUE;
  if(!delay_recupero){ //se non sono in recupero
    time(t_start); //salvo tempo inizio apertura
  }

  if((*allarme = fork()) == 0){ //inizializzo un timer
    if(delay_recupero == FALSE){ //se non sono in recupero
      printf("Chiusura automatica fra %d secondi\n", frigo->delay);
      sleep(frigo->delay); //chiusura dopo [delay] secondi
    }
    else{ // altrimenti
      sleep(delay_recupero); // chiusura dopo [delay_recupero] secondi
    }

    int queue;
    msgbuf messaggio;
    create_queue(frigo->id, &queue);
    crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo->id, frigo->id, FRIDGE_SET_STATO); //invio un messaggio a se stesso...
    concat_int(&messaggio, FALSE); //...con stato FALSE
    send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
    exit(0); //termino allarme
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
  if(frigo->stato == FALSE){
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

  crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo->id, frigo->id, FRIDGE_RECUPERO); //creo header
  if(frigo->stato){
    frigo->time = difftime(time(NULL), t_start);
  }
  concat_dati(&messaggio, frigo);
  concat_int(&messaggio, t_start);
  concat_int(&messaggio, frigo->id);
  create_queue(frigo->id, &queue); //invio il messaggio al nuovo frigo duplicato che avrà lo stesso id di questo
  send_message(queue, &messaggio, messaggio.msg_text, 10);
  printf("-------Fine---------\n");
}


void concat_dati(msgbuf *messaggio, t_frigo *frigo){ //concateno dati da inviar
  concat_int(messaggio, frigo->stato);
  concat_int(messaggio, frigo->interruttore);
  concat_int(messaggio, frigo->termostato);
  concat_int(messaggio, frigo->time);
  concat_int(messaggio, frigo->delay);
  concat_int(messaggio, frigo->percentuale);
  concat_string(messaggio, frigo->nome);
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
