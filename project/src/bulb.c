//Header
#include "bulb.h"

/*Operazioni di una Bulb
MSG_BULB_SWITCH_S = 510001
MSG_BULB_SWITCH_I = 510002
MSG_BULB_GETTIME  = 510003
MSG_BULB_GETINFO  = 510004
*/

/* Funzione Bulb */
void bulb(int id, int recupero, char * nome){ //recupero booleano
  signal(SIGCHLD, SIG_IGN); //evita che vengono creati processi zombie quando processi figli eseguono exit
  int status = FALSE;
  int interruttore = FALSE;
  time_t t_start = 0;
  char * name = strdup(nome);

  pid_t idf = -1;

  int queue;
  msgbuf messaggio;
  msgbuf risposta;
  msgbuf rec_buf;
  msgbuf tmp_buf;

  rec_buf.msg_type = 10;

  crea_queue(id, &queue);
  printf("Bulb: %d - pid: %d - ppid: %d\n", id, getpid(), getppid());

  int flag = FALSE;

  char ** msg;

  if(recupero == TRUE){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[BULB_REC_STATO]);
      interruttore = atoi(msg[BULB_REC_INTERRUTTORE]);
      t_start = atoi(msg[BULB_REC_TSTART]);
      name = (char *) malloc (sizeof(char) * (strlen(msg[BULB_REC_NOME]) + 1));
      strcpy(name, msg[BULB_REC_NOME]);
    }
  }

  if((idf = fork()) < 0){
    perror("Errore fork");
    exit(1);
  }

  else if(idf == 0){ //Creo un processo figlio per leggere da input
    printf("Inizio comunicazione\n"); //Lettura da input
    int fd_write, fd_read, n_arg;
    int richiesta = 0;
    char buf_r[BUF_SIZE];
    char buf_w[BUF_SIZE];
    char * str;
    char **cmd;
    char * rfifo = percorso_file(id,READ);
    char * wfifo = percorso_file(id,WRITE);
    flag = TRUE;

    if((mkfifo(wfifo, 0666) == -1) && (errno != EEXIST)){ //se path esiste già continuo normalmente
      perror("Errore mkfifo");
      exit(1);
    }
    if((mkfifo(rfifo, 0666) == -1) && (errno != EEXIST)){
      perror("Errore mkfifo");
      exit(1);
    }

    while (flag) {
      printf("----Inizio lettura -----\n");
      richiesta = -1;
      fd_read = open(rfifo, O_RDONLY);
      printf("Pronta per leggere\n");
      read(fd_read, buf_r, BUF_SIZE); // Leggo dalla FIFO
      printf("Ho letto il comando\n");
      printf("CMD : %s\n", buf_r);
      n_arg = str_split(buf_r, &cmd); // Numero di argomenti passati

      if(strcmp(cmd[1], "chiuditi") == 0){ // se il comando inserito è close esco
        printf("Fine comunicazione\n");
        flag = FALSE;
        //kill(getpid(),SIGTERM); //Io ucciderei il processo qua
      }
      else if (n_arg == 2){ //Accetto comandi del interruttore
        if(strcmp(cmd[1], "interruttore") == 0){
          crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_BULB_SWITCH_I);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
        }
        else {
          printf("Operazione non valida\n");
        }

      }
      else if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){
        if(strcmp(cmd[2], "time") == 0){
          crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_BULB_GETTIME);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          richiesta = MSG_BULB_GETTIME;
        }
        else if(strcmp(cmd[2], "info") == 0){
          crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_INF);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          richiesta = MSG_INF;
        }

        fd_write = open(wfifo, O_WRONLY); //apro fifo di scrittura

        if(richiesta > 0) { //quando ricevo la risposta
          msgrcv(queue, &messaggio,sizeof(messaggio.msg_text), MSG_FIFO, 0);
          printf("Bulb fifo: Ricevuta risposta\n");
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 80);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          //concateno i dati ricevuti
          printf("Codice richiesta: %d\n", richiesta);
          if ((richiesta == MSG_INF) && codice_messaggio(info_response) == MSG_INF_BULB) {
            printf("Bulb fifo: Info\n");
            sprintf(str_temp, "\nNome: %s\n", info_response[BULB_INF_NOME]);
            strcpy(buf_w, str_temp);
            sprintf(str_temp, "Id: %s\n", info_response[MSG_ID_MITTENTE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Stato: %s\n", info_response[BULB_INF_STATO]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Interruttore: %s\n", info_response[BULB_INF_INTERRUTTORE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Tempo di utilizzo: %s\n", info_response[BULB_INF_TIME]);
            strcat(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w

          }
          else if (richiesta == MSG_BULB_GETTIME) {
            printf("Bulb fifo: Get Time\n");
            sprintf(str_temp, "\nTempo di utilizzo: %s\n", info_response[BULB_TIME]);
            strcpy(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
          }
          else {
            sprintf(str_temp, "\nRichiesta non identificata\n");
            strcpy(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
          }
          richiesta = -1;
          printf("Scrivo su fifo: %s\n", buf_w);
        }
        else {
          strcpy(buf_w, "Questa operazione non ci concerne");
          write(fd_write, buf_w, strlen(buf_w)+1);
        }
      }

      close(fd_read);
      printf("File in lettura è stato chiuso\n");
      close(fd_write);
      printf("File in scrittura è stato chiuso\n");
    }
    unlink(rfifo); //una volta uscita dal ciclo elimino file fifo
    unlink(wfifo);
    free(rfifo); // Libero la memoria allocata
    free(wfifo); // Libero la memoria allocata
  }

  else {
    int q_ris;

    //Inizio Loop
    while (TRUE) {
      printf("Bulb: Pronta per ricevere \n");
      msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
      protocoll_parser(messaggio.msg_text, &msg);
      int codice = codice_messaggio(msg);
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), & q_ris);
      printf("Bulb ha ricevuto\n");
      printf("Cod %d\n", atoi(msg[MSG_OP]));

      if(codice == MSG_INF && controllo_bulb(msg,id)) { //richiesta info
        printf("Bulb: Richiesta info\n");
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_BULB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_string(&risposta, name);
        concat_int(&risposta, status);
        concat_int(&risposta, interruttore);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          risposta.msg_type = MSG_FIFO;
          msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0); //mando un messaggio alla fifo
        }
        else{
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
      }

      else if(codice == MSG_OVERRIDE && controllo_bulb(msg, id)){
        printf("Bulb: Messaggio di override\n");
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_BULB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_string(&risposta, name);
        concat_int(&risposta, status);
        concat_int(&risposta, interruttore);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice == MSG_SALVA_SPEGNI && controllo_bulb(msg, id)){
        printf("Bulb: Sto salvando i dati\n");
        crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
        concat_int(&rec_buf, BULB);
        concat_int(&rec_buf, id);
        concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
        msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
        exit(EXIT_SUCCESS);
      }

      else if(codice_messaggio(msg) == MSG_SPEGNI && controllo_bulb(msg, id)){
        printf("Bulb: Spegnimento\n");
        if(idf > 0){
          kill(idf, SIGTERM);
        }
        exit(EXIT_SUCCESS);
      }

      else if(codice_messaggio(msg) == MSG_RIMUOVIFIGLIO && controllo_bulb(msg, id)){
        printf("Bulb: ricevuto messaggio rimuovi figlio\n");
        int queue_deposito;
        crea_queue(DEPOSITO, &queue_deposito);
        if(id == atoi(msg[MSG_RIMUOVIFIGLIO_ID])){ //se sono io
          printf("Bulb: Sono io il figlio da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
          if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP){ //se la specifica è SPEC_DEP
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
            concat_int(&rec_buf, BULB);
            concat_int(&rec_buf, id);
            concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            printf("Bulb: Dati salvati per il recupero, invio aggiungi al deposito e termino\n");
            crea_messaggio_base(&tmp_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_AGGIUNGI); //il deposito deve aggiungere un nuovo frigo
            concat_int(&tmp_buf, id); // con mio stesso id
            tmp_buf.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue_deposito, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
            exit(EXIT_SUCCESS); //termino processo
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_SALVA){ //se devo solo salvarmi
            printf("Bulb: salvo i dati\n");
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
            concat_int(&rec_buf, BULB);
            concat_int(&rec_buf, id);
            concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            exit(EXIT_SUCCESS);
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEL){ //se devo solo terminare
            printf("Bulb: termino\n");
            if(idf >= 0){
              kill(idf, SIGTERM); //termino eventuale figlio che gestisce fifo
            }
            exit(EXIT_SUCCESS); // termino processo
          }
        }
        else{ // se disp da eliminare non sono io, mando un ACKN
          printf("Bulb: non sono il dispositivo da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
      }

      else if(codice_messaggio(msg) == MSG_GET_TERMINAL_TYPE && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
        concat_int(&risposta, BULB);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice_messaggio(msg) == MSG_BULB_SWITCH_S && controllo_bulb(msg, id)){
        inverti_stato(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice_messaggio(msg) == MSG_BULB_SWITCH_I && controllo_bulb(msg, id)){
        inverti_interruttore(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice_messaggio(msg) == MSG_BULB_GETTIME && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        risposta.msg_type = MSG_FIFO;
        msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice_messaggio(msg) == MSG_AGGIUNGI && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
  }
}

// Funzione che prende i dati di una bulb e li concatena al messaggio salvato
// nel buffer
void concat_dati_bulb(msgbuf * m, int s, int i, time_t t, char * nb){
  char * st;
  itoa(s, &st);
  char * it;
  itoa(i, &it);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * strlen(m->msg_text) + strlen(st) + 1 + strlen(it) + 1 + strlen(tt) + 1 + strlen(nb) + 2);
  strcpy(r,m->msg_text);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,it);
  strcat(r,"\n");
  strcat(r,tt);
  strcat(r,"\n");
  strcat(r,nb);
  strcat(r,"\n");
  strcpy(m->msg_text, r);

  free(r);
}

// Funziona che controlla che la stringa protocollo sia destinanata ad una BULB
// con il nostro id o alternativamente abbia i valori di default
// restituisce un booleano
int controllo_bulb(char ** str, int id){
  int rt = FALSE;
  if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == DEFAULT){
    rt = TRUE;
  }
  return rt;
}

// Funzione che riceve i registri stato, interruttore e t_start per riferimento
// Inverte l'interruttore e conseguentemente anche lo stato -> Manuale
void inverti_interruttore(int * s, int * i, time_t *t) {
  if ((*i) == FALSE) {
    (*i) = TRUE;
    if(!(*s)){
      (*s) = TRUE;
      (*t) = time(NULL);
    }
    printf("Interruttore su ON\n");
  }
  else {
    (*i) = FALSE;
    if(*s){
      (*s) = FALSE;
    }
    printf("Interruttore su OFF\n");
  }
}

// Funzione che riceve i registri stato, interruttore e t_start per riferimento
// Inverte l'interruttore e conseguentemente anche lo stato -> Centralina
void inverti_stato(int * s, int * i, time_t *t) {
  if ((*s) == FALSE) {
    (*s) = TRUE;
    (*t) = time(NULL);
    printf("La vostra lampadina è stata accesa\n");
  }
  else {
    (*s) = FALSE;
    printf("La vostra lampadina è stata spenta\n");
  }
}

// Funzione che restituisce il tempo di accensione della Lampadina
// Se la lampadina è spenta restituisce 0
int tempo_bulb_on(int s, time_t t) {
  int res = 0;
  if(s == TRUE){
    res = (int) difftime(time(NULL),t);
  }

  return res;
}

int equal_bulb(msgbuf * msg1, msgbuf * msg2){
  printf("Bulb: confronto in corso\n");
  int rt = FALSE;
  char **buf1, **buf2;
  protocoll_parser(msg1->msg_text, &buf1);
  protocoll_parser(msg2->msg_text, &buf2);
  if((strcmp(buf1[BULB_INF_STATO], buf2[BULB_INF_STATO]) == 0) && (strcmp(buf1[BULB_INF_INTERRUTTORE], buf2[BULB_INF_INTERRUTTORE]) == 0) && (strcmp(buf1[BULB_INF_NOME], buf2[BULB_INF_NOME]) == 0)) {
    printf("Bulb: Messaggi uguali\n");
    rt = TRUE;
  }
  else {
    printf("Bulb: Messaggi diversi\n");
  }

  return rt;
}
//msgctl( queue, IPC_RMID, 0); - Serve a svuotare la queue

int stampa_info_bulb(msgbuf * m){
  int r = FALSE;
  char ** msg;
  protocoll_parser(m->msg_text, &msg);

  if ((codice_messaggio(msg) == MSG_INF_BULB) && ((atoi(msg[MSG_TYPE_MITTENTE]) == BULB) || (atoi(msg[MSG_TYPE_MITTENTE]) == DEFAULT))) {
    printf("\ninfo bulb:\n---------------------------------- \n");
    printf("%s[BULB] : %s\n", msg[BULB_INF_NOME], msg[MSG_ID_MITTENTE]);
    printf("| Stato : %s\n", msg[BULB_INF_STATO]);
    printf("| Interruttore : %s\n", msg[BULB_INF_INTERRUTTORE]);
    printf("| Tempo di utilizzo : %s\n", msg[BULB_INF_TIME]);
    printf("| \\ \n");
    printf("\n---------------------------------- \n\n");
    r = TRUE;
  }
  return r;
}
