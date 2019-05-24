#include "fridge.h"
//Tips per terminale: ipcs -q => code di messaggi attive
// ipcrm -q <msqid> => rimuove la coda
// rm /tmp/D_* => rimuove tutti i file che inizia in questo modo

//msg.type = 1 => ricevi
//msg.type = 2 => invio

/*
  funzioni get sono riservate a fifo
*/


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
  msgbuf risposta;
  msgbuf fifo_msgbuf;

  messaggio.msg_type = NUOVA_OPERAZIONE;
  risposta.msg_type = 2;
  fifo_msgbuf.msg_type = MSG_FIFO;

  crea_queue(id, &queue);
  printf("Id frigo: %d - pid: %d - ppid: %d\n", frigo.id, getpid(), getppid());

  if(recupero){
    if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
      perror("errore lettura ripristino");
    }
    else{
      printf("Frigo: recupero\n");
      int new_delay = 0;
      char ** msg;
      protocoll_parser(messaggio.msg_text, &msg);
      frigo.id = atoi(msg[MSG_FRIDGE_REC_ID]);
      frigo.stato = atoi(msg[MSG_FRIDGE_REC_STATO]);
      frigo.interruttore = atoi(msg[MSG_FRIDGE_REC_INTERRUTTORE]);
      frigo.termostato = atoi(msg[MSG_FRIDGE_REC_TERM]);
      frigo.time = atoi(msg[MSG_FRIDGE_REC_TIME]);
      frigo.delay = atoi(msg[MSG_FRIDGE_REC_DELAY]);
      frigo.percentuale = atoi(msg[MSG_FRIDGE_REC_PERC]);
      frigo.nome = malloc(sizeof(char) * (strlen(msg[MSG_FRIDGE_REC_NOME]) + 1));
      strcpy(frigo.nome, msg[MSG_FRIDGE_REC_NOME]);
      t_start = atoi(msg[MSG_FRIDGE_REC_TSTART]);
      if(frigo.stato){ //se frigo è aperto
      	new_delay = frigo.delay - frigo.time; //recupero delay rimanente
      	apri_frigo(&frigo, &t_start, &allarme, new_delay);
      }

      printf("Id frigo: %d\n", frigo.id);
      printf("stato: %d\n", frigo.stato);
      printf("Interruttore: %d\n", frigo.interruttore);
      printf("termostato: %d\n", frigo.termostato);
      printf("time: %d\n", frigo.time);
      printf("delay: %d\n", frigo.delay);
      printf("percentuale: %d\n", frigo.percentuale);
      printf("nome: %s\n", frigo.nome);
      printf("new delay: %d\n", new_delay);


      //msgctl(queue, IPC_RMID, NULL); //rimuovo la coda precedentemente creata
    }
    printf("Frigo: fine recupero\n" );
  }

  if((idf = fork()) < 0){
    perror("Errore fork");
    exit(1);
  }
  else if(idf == 0){ // figlio per la gestione del fifo con shell
    printf("Frigo - fifo: pid = %d, ppid = %d\n", getpid(), getppid());
    char *fifo_w = (char *) malloc(sizeof(char)*20);
    char *fifo_r = (char *) malloc(sizeof(char)*20);
    sprintf(fifo_w, "/tmp/D_%d_W", frigo.id);
    sprintf(fifo_r, "/tmp/D_%d_R", frigo.id);
    char buf_r[BUF_SIZE], buf_w[BUF_SIZE];
    int fd_write, fd_read, n_arg, flag = TRUE, richiesta = -1;
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
      richiesta = -1;
      fd_read = open(fifo_r, O_RDONLY);
      read(fd_read, buf_r, BUF_SIZE);
      n_arg = str_split(buf_r, &cmd);
      if((n_arg == 4) && strcmp(cmd[1], "set") == 0){
        printf("richiesta set\n");
        if(strcmp(cmd[2], "interruttore") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_SETINTERRUTTORE);
          concat_string(&messaggio, cmd[3]);
          msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "delay") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_SETDELAY);
          concat_string(&messaggio, cmd[3]);
          msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "percentuale") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_SETPERC);
          concat_string(&messaggio, cmd[3]);
          msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
          printf("invio messaggio\n");
        }
        else if(strcmp(cmd[2], "termostato") == 0){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_SETTERMOSTATO);
          concat_string(&messaggio, cmd[3]);
          msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
          printf("invio messaggio\n");
        }
        else{
          printf("Operazione non valida\n");
        }
      }
      else if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){
        if((strcmp(cmd[2], "info") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_INF);
          richiesta = MSG_INF;
        }
        else if((strcmp(cmd[2], "time") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETTIME);
          richiesta = MSG_FRIDGE_GETTIME;
        }
        else if((strcmp(cmd[2], "stato") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETSTATO);
          richiesta = MSG_FRIDGE_GETSTATO;
        }
        else if((strcmp(cmd[2], "delay") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETDELAY);
          richiesta = MSG_FRIDGE_GETDELAY;
        }
        else if((strcmp(cmd[2], "percentuale") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETPERC);
          richiesta = MSG_FRIDGE_GETPERC;
        }
        else if((strcmp(cmd[2], "termostato") == 0)){
          crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETTERMOSTATO);
          richiesta = MSG_FRIDGE_GETTERMOSTATO;
        }
        else{ //comando set ricevuto non è uno di quelli di frigo
          printf("Questo comando set non esiste\n");
          fd_write = open(fifo_w, O_WRONLY); //apro fifo di scrittura
          strcpy(buf_w, "L'operazione non supportata dal dispositivo\n");
          write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
        }

        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0); //invio la richiesta di info al frigo

        if((richiesta > 0) && ((msgrcv(queue, &messaggio,sizeof(messaggio.msg_text), MSG_FIFO, 0)) != -1)){ //quando ricevo la risposta
          int codice;
          printf("Apro fifo scrittura\n");
          fd_write = open(fifo_w, O_WRONLY); //apro fifo di scrittura
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 40);
          protocoll_parser(messaggio.msg_text, &info_response);
          codice = codice_messaggio(info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          switch (codice) {
            case MSG_INF:
              sprintf(str_temp, "Nome: %s\n", info_response[MSG_FRIDGE_INF_NOME]); //concateno i dati ricevuti
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Id: %s\n", info_response[MSG_ID_MITTENTE]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Stato: %s\n", info_response[MSG_FRIDGE_INF_STATO]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Interruttore: %s\n", info_response[MSG_FRIDGE_INF_INTERRUTTORE]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Temperatura interna: %s\n", info_response[MSG_FRIDGE_INF_TERM]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Time: %s\n", info_response[MSG_FRIDGE_INF_TIME]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Delay: %s\n", info_response[MSG_FRIDGE_INF_DELAY]);
              strcat(buf_w, str_temp);
              sprintf(str_temp, "Percentuale di riempimento: %s\n", info_response[MSG_FRIDGE_INF_PERC]);
              strcat(buf_w, str_temp);
              break;
            case MSG_FRIDGE_GETTIME:
              sprintf(str_temp, "Tempo di apertura: %s\n", info_response[MSG_FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case MSG_FRIDGE_GETPERC:
              sprintf(str_temp, "Percentuale di riempimento: %s\n", info_response[MSG_FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case MSG_FRIDGE_GETTERMOSTATO:
              sprintf(str_temp, "Temperatura interna: %s\n", info_response[MSG_FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case MSG_FRIDGE_GETDELAY:
              sprintf(str_temp, "Delay: %s\n", info_response[MSG_FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            case MSG_FRIDGE_GETSTATO:
              sprintf(str_temp, "Stato: %s\n", info_response[MSG_FRIDGE_VALORE]);
              strcat(buf_w, str_temp);
              break;
            default:
              break;
          }
          write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
          printf("Scrivo su fifo\n");
        }

      }
      else{
        printf("Comando non valido\n");
      }
      close(fd_read); //chiudo fifo
      close(fd_write);
    }
    unlink(fifo_r); //una volta uscita dal ciclo elimino file fifo
    unlink(fifo_w);
  }
  else{
    int queue_risposta;

    while(TRUE) {
      msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
      char ** msg;
      int codice;
      protocoll_parser(messaggio.msg_text, &msg);
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &queue_risposta);
      codice = codice_messaggio(msg);

      if(codice == MSG_INF && controlla_fridge(msg, frigo.id)){ //se è una richiesta info
        printf("Richiesta info\n");
        if(frigo.stato){
          frigo.time = difftime(time(NULL), t_start);
        }

        if(atoi(msg[MSG_ID_MITTENTE]) != frigo.id){
          printf("Rchiesta info dagli altri\n");
          send_info_fridge(msg, &frigo);
        }
        else{
          printf("Richiesta da se stesso\n");
          crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_INF);
          concat_int(&fifo_msgbuf, -1); // id_padre, in questo nessuno
          concat_dati(&fifo_msgbuf, &frigo);
          msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0);
        }

      }
      else if(codice == MSG_OVERRIDE && controlla_fridge(msg, frigo.id)){
        if(frigo.stato){
          frigo.time = difftime(time(NULL), t_start);
        }
        send_info_fridge(msg, &frigo);
      }
      else if(codice == MSG_RIMUOVIFIGLIO && controlla_fridge(msg, frigo.id)){
        printf("Frigo: ricevuto messaggio rimuovi figlio\n");
        int queue_deposito;
        crea_queue(DEPOSITO, &queue_deposito);
        if(frigo.id == atoi(msg[MSG_RIMUOVIFIGLIO_ID]) ){ //se sono io
          printf("Frigo: Sono io il figlio da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_ACKP);
          msgsnd(queue_risposta, &risposta, sizeof(risposta.msg_text), 0);
          if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP){ //se la specifica è SPEC_DEP
            printf("Frigo: invio aggiungi al deposito e salvo e termino\n");
            crea_messaggio_base(&messaggio, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_AGGIUNGI); //il deposito deve aggiungere un nuovo frigo
            concat_int(&messaggio, frigo.id); // con mio stesso id
            msgsnd(queue_deposito, &messaggio, sizeof(messaggio.msg_text), 0);
            salva_dati(&frigo, t_start, allarme, idf); //salvo i dati e mando alla coda del nuovo frigo
            exit(EXIT_SUCCESS); //termino processo
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_SALVA){ //se devo solo salvarmi
            printf("Frigo: salvo i dati\n");
            salva_dati(&frigo, t_start, allarme, idf);
            exit(EXIT_SUCCESS);
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEL){ //se devo solo terminare
            printf("Frigo: termino\n");
            if(allarme >= 0){
              kill(allarme, SIGTERM); //termino eventuale processo allarme
            }
            if(idf >= 0){
              kill(idf, SIGTERM); //termino eventuale figlio che gestisce fifo
            }
            exit(EXIT_SUCCESS); // termino processo
          }
        }
        else{ // se disp da eliminare non sono io, mando un ACKN
          printf("Frigo: non sono il dispositivo da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_ACKN);
          msgsnd(queue_risposta, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
      else if(codice == MSG_GET_TERMINAL_TYPE && controlla_fridge(msg, frigo.id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_MYTYPE);
        concat_int(&risposta, FRIDGE);
        msgsnd(queue_risposta, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_SALVA_SPEGNI && controlla_fridge(msg, frigo.id)){
        salva_dati(&frigo, t_start, allarme, idf);
        exit(EXIT_SUCCESS);
      }
      else if(codice == MSG_SPEGNI && controlla_fridge(msg, frigo.id)){
        printf("Spegnimento\n");
        if(allarme > 0){
          kill(allarme, SIGTERM);
        }
        if(idf > 0){
          kill(idf, SIGTERM);
        }
        exit(EXIT_SUCCESS);
      }
      else if(codice == MSG_FRIDGE_GETTIME && controlla_fridge(msg, frigo.id)){

        if(frigo.stato == TRUE){
          frigo.time = difftime(time(NULL), t_start);
        }
        crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETTIME);
        concat_int(&fifo_msgbuf, frigo.time);
        msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0); //invio la riposta al figlio (fifo)
      }
      else if(codice == MSG_FRIDGE_GETPERC && controlla_fridge(msg, frigo.id)){
        crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETPERC);
        concat_int(&fifo_msgbuf, frigo.percentuale);
        msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0);
      }
      else if(codice == MSG_FRIDGE_GETTERMOSTATO && controlla_fridge(msg, frigo.id)){
        crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETTERMOSTATO);
        concat_int(&fifo_msgbuf, frigo.termostato);
        msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0);
      }
      else if(codice == MSG_FRIDGE_GETSTATO && controlla_fridge(msg, frigo.id)){
        crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETSTATO);
        concat_int(&fifo_msgbuf, frigo.stato);
        msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0);
      }
      else if(codice == MSG_FRIDGE_GETDELAY && controlla_fridge(msg, frigo.id)){
        crea_messaggio_base(&fifo_msgbuf, FRIDGE, FRIDGE, frigo.id, frigo.id, MSG_FRIDGE_GETDELAY);
        concat_int(&fifo_msgbuf, frigo.delay);
        msgsnd(queue, &fifo_msgbuf, sizeof(fifo_msgbuf.msg_text), 0);
      }
      else if(codice == MSG_FRIDGE_SETSTATO && controlla_fridge(msg, frigo.id)){
        set_stato(atoi(msg[MSG_FRIDGE_VALORE]), &frigo, &t_start, &allarme);
        printf("Stato attuale: %d\n", frigo.stato);
      }
      else if(codice == MSG_FRIDGE_SETINTERRUTTORE && controlla_fridge(msg, frigo.id)){
        set_interruttore(atoi(msg[MSG_FRIDGE_VALORE]), &frigo, &t_start, &allarme);
        printf("Interruttore attuale: %d\n", frigo.interruttore);
      }
      else if(codice == MSG_FRIDGE_SETTERMOSTATO && controlla_fridge(msg, frigo.id)){
        set_termostato(atoi(msg[MSG_FRIDGE_VALORE]), &frigo);
        printf("Termostato attuale: %d\n", frigo.termostato);
      }
      else if(codice == MSG_FRIDGE_SETDELAY && controlla_fridge(msg, frigo.id)){
        set_delay(atoi(msg[MSG_FRIDGE_VALORE]), &frigo);
        printf("Delay attuale: %d\n", frigo.delay);
      }
      else if(codice == MSG_FRIDGE_SETPERC && controlla_fridge(msg, frigo.id)){
        set_perc(atoi(msg[MSG_FRIDGE_VALORE]), &frigo);
        printf("Percentuale attuale: %d\n", frigo.percentuale);
      }
      else{ //richieste non valide -> invio un ACKN
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          crea_messaggio_base(&messaggio, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_ACKN);
          msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo.id, MSG_ACKN);
          msgsnd(queue_risposta, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
    }
  }
}

int equal_fridge(msgbuf *msg1, msgbuf *msg2){
  printf("Frigo: confronto in corso\n");
  int rt = FALSE;
  char **buf1, **buf2;
  protocoll_parser(msg1->msg_text, &buf1);
  protocoll_parser(msg2->msg_text, &buf2);
  if((strcmp(buf1[MSG_FRIDGE_INF_STATO], buf2[MSG_FRIDGE_INF_STATO]) == 0) && (strcmp(buf1[MSG_FRIDGE_INF_INTERRUTTORE], buf2[MSG_FRIDGE_INF_INTERRUTTORE]) == 0)
  && (strcmp(buf1[MSG_FRIDGE_INF_TERM], buf2[MSG_FRIDGE_INF_TERM]) == 0) && (strcmp(buf1[MSG_FRIDGE_INF_NOME], buf2[MSG_FRIDGE_INF_NOME]) == 0)
  && (strcmp(buf1[MSG_FRIDGE_INF_DELAY], buf2[MSG_FRIDGE_INF_DELAY]) == 0) && (strcmp(buf1[MSG_FRIDGE_INF_PERC], buf2[MSG_FRIDGE_INF_PERC]) == 0)){
    printf("Frigo: Messaggi uguali\n");
    rt = TRUE;
  }
  else{
    printf("Frigo: Messaggi diversi\n");
  }

  return rt;
}

void send_info_fridge(char **msg, t_frigo *frigo){ //invia info
  printf("Send info\n");
  int queue_risposta;
  msgbuf risposta;
  risposta.msg_type = 2;
  crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo->id, MSG_INF_FRIDGE);
  concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
  concat_dati(&risposta, frigo); //concateno i dati del frigo
  crea_queue(atoi(msg[MSG_ID_MITTENTE]), &queue_risposta); //stabilisco una coda col padre
  msgsnd(queue_risposta, &risposta, sizeof(risposta.msg_text), 0);
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
    messaggio.msg_type = NUOVA_OPERAZIONE;
    crea_queue(frigo->id, &queue);
    crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, frigo->id, frigo->id, MSG_FRIDGE_SETSTATO); //invio un messaggio a se stesso...
    concat_int(&messaggio, FALSE); //...con stato FALSE
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
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
  if((valore >= 0) && (valore <= 7)){
    frigo->termostato = valore;
    printf("Temperatura combiata con successo\n");
  }
  else{
    printf("Si consiglia un range [0°-7°]\n");
  }
}

void set_delay(int valore, t_frigo *frigo){
  printf("Cambio delay\n");
  if(valore > 0){
    frigo->delay = valore;
    printf("Delay cambiato con successo\n");
  }
  else{
    printf("Errore: delay negativo\n");
  }
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

void salva_dati(t_frigo *frigo, time_t t_start, int allarme, int idf){
  printf("---------Duplicate--------\n");
  int queue;
  msgbuf recupero_msgbuf;
  recupero_msgbuf.msg_type = 10;

  crea_messaggio_base(&recupero_msgbuf, FRIDGE, FRIDGE, frigo->id, frigo->id, MSG_RECUPERO_FRIDGE); //creo header
  if(frigo->stato){
    frigo->time = difftime(time(NULL), t_start);
  }
  concat_int(&recupero_msgbuf, FRIDGE);
  concat_int(&recupero_msgbuf, frigo->id);
  concat_dati(&recupero_msgbuf, frigo);
  concat_int(&recupero_msgbuf, t_start);
  crea_queue(frigo->id, &queue); //invio il messaggio al nuovo frigo duplicato che avrà lo stesso id di questo
  msgsnd(queue, &recupero_msgbuf, sizeof(recupero_msgbuf.msg_text), 0);
  printf("-------Fine---------\n");

  if(allarme > 0){ //elimino eventuali figli
    kill(allarme, SIGTERM);
  }
  if(idf > 0){
    kill(idf, SIGTERM);
  }
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
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == FRIDGE || (atoi(str[MSG_TYPE_DESTINATARIO]) == DEFAULT)){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == DEFAULT){
      rt = TRUE;
    }
  }
  return rt;
}

void stampa_info_fridge(msgbuf *buf){
  char ** msg;
  int codice;
  protocoll_parser(buf->msg_text, &msg);
  codice = codice_messaggio(msg);
  if((codice == MSG_INF_FRIDGE) && (atoi(msg[MSG_TYPE_MITTENTE]) == FRIDGE)){
    printf("\ninfo fridge:\n---------------------------------- \n");
    printf("%s(FRIDGE) : %s\n", msg[MSG_FRIDGE_INF_NOME], msg[MSG_ID_MITTENTE]);
    printf("| Stato : %s\n", msg[MSG_FRIDGE_INF_STATO]);
    printf("| Interruttore : %s\n", msg[MSG_FRIDGE_INF_STATO]);
    printf("| Termostato : %s\n", msg[MSG_FRIDGE_INF_TERM]);
    printf("| Time : %s\n", msg[MSG_FRIDGE_INF_TIME]);
    printf("| Delay : %s\n", msg[MSG_FRIDGE_INF_DELAY]);
    printf("| Percentuale di riempimento : %s\n", msg[MSG_FRIDGE_INF_PERC]);
    printf("| \\ \n");
    printf("\n---------------------------------- \n\n");
  }
}
