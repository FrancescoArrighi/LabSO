//Header
#include "bulb.h"

/*Operazioni di una Bulb
MSG_BULB_SWITCH_S = 510001
MSG_BULB_SWITCH_I = 510002
MSG_GETTIME       = 10003
*/

//Funzione che inizializza una bulb ricevendo un id un un bool per eventuale recupero ed un nome
void bulb(int id, int recupero, char * nome){
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

  if(recupero == TRUE){ //Gestisco il recupero di una bulb
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
    char buf_r[BUF_SIZE];
    char buf_w[BUF_SIZE];
    char * str;
    char **cmd;
    char * rfifo = percorso_file(id,READ);
    char * wfifo = percorso_file(id,WRITE);
    flag = TRUE;

    if((mkfifo(wfifo, 0666) == -1) && (errno != EEXIST)){ //Controllo la corretta creazione della fifo di scrittura
      perror("Errore mkfifo");
      exit(1);
    }
    if((mkfifo(rfifo, 0666) == -1) && (errno != EEXIST)){ //Controllo la corretta creazione della fifo di lettura
      perror("Errore mkfifo");
      exit(1);
    }

    while (flag) {
      printf("----Inizio lettura -----\n");
      fd_read = open(rfifo, O_RDONLY);
      printf("Pronta per leggere\n");
      read(fd_read, buf_r, BUF_SIZE); // Leggo dalla FIFO
      printf("Ho letto il comando\n");
      printf("CMD : %s\n", buf_r);
      n_arg = str_split(buf_r, &cmd); // Numero di argomenti passati
      int c_umano = codice_messaggio(cmd);

      if (c_umano > 0){ //Agisco solo se il codice non è un ack negativo
        if(c_umano == MSG_BULB_SWITCH_I){
          crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_BULB_SWITCH_I);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
        }

        else if((c_umano == MSG_GETTIME) || (c_umano == MSG_INF)) { //Gestisco la parte che necessita di una risposta
          if(c_umano == MSG_GETTIME){
            crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_GETTIME);
            tmp_buf.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          }
          else if(c_umano == MSG_INF){
            crea_messaggio_base(&tmp_buf, BULB, BULB, id, id, MSG_INF);
            tmp_buf.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          }

          fd_write = open(wfifo, O_WRONLY); //apro fifo di scrittura

          msgrcv(queue, &messaggio,sizeof(messaggio.msg_text), MSG_FIFO, 0);
          printf("Bulb fifo: Ricevuta risposta\n");
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 80);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          //concateno i dati ricevuti
          if ((c_umano == MSG_INF) && (codice_messaggio(info_response) == MSG_INF_BULB)) {
            printf("Bulb fifo: Info\n");
            sprintf(str_temp, "\nNome[BULB]: %s\n", info_response[BULB_INF_NOME]);
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
          else if (c_umano == MSG_GETTIME) {
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
          printf("Scrivo su fifo: %s\n", buf_w);
        }
      }
      else {
        printf("Operazione non valida\n");
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
      msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
      protocoll_parser(messaggio.msg_text, &msg);
      int codice = codice_messaggio(msg);
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), & q_ris);
      //printf("Bulb ha ricevuto\n");
      //printf("Cod %d\n", atoi(msg[MSG_OP]));

      if(codice == MSG_INF && controllo_bulb(msg,id)) { //richiesta info
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

      else if(codice == MSG_OVERRIDE && controllo_bulb(msg, id)){ // Controllo di override
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_BULB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_string(&risposta, name);
        concat_int(&risposta, status);
        concat_int(&risposta, interruttore);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice == MSG_SALVA_SPEGNI && controllo_bulb(msg, id)){ //Salvo i dati per un recupero
        crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
        concat_int(&rec_buf, BULB);
        concat_int(&rec_buf, id);
        concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
        msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
        exit(EXIT_SUCCESS);
      }

      else if(codice == MSG_SPEGNI && controllo_bulb(msg, id)){ //Spengo tutto e uccido anche il sottoprocesso che legge da umano
        if(idf > 0){
          kill(idf, SIGTERM);
        }
        exit(EXIT_SUCCESS);
      }

      else if(codice == MSG_RIMUOVIFIGLIO && controllo_bulb(msg, id)){ //Gestisco la ricezione di un messaggio di rimuovi figlio
        int queue_deposito;
        crea_queue(DEPOSITO, &queue_deposito);
        if(id == atoi(msg[MSG_RIMUOVIFIGLIO_ID])){ //se sono io
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
          if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP){ //se la specifica è SPEC_DEP - salvo dati
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
            concat_int(&rec_buf, BULB);
            concat_int(&rec_buf, id);
            concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            crea_messaggio_base(&tmp_buf, DEPOSITO, BULB, DEPOSITO, id, MSG_AGGIUNGI); //il deposito deve aggiungere una nuova bulb
            concat_int(&tmp_buf, id); // con mio stesso id
            tmp_buf.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue_deposito, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
            exit(EXIT_SUCCESS); //termino processo
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_SALVA){ //se devo solo salvarmi
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_BULB);
            concat_int(&rec_buf, BULB);
            concat_int(&rec_buf, id);
            concat_dati_bulb(&rec_buf, status, interruttore, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            exit(EXIT_SUCCESS);
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEL){ //se devo solo terminare
            if(idf >= 0){
              kill(idf, SIGTERM); //termino eventuale figlio che gestisce fifo
            }
            exit(EXIT_SUCCESS); // termino processo
          }
        }
        else{ // se disp da eliminare non sono io, mando un ACKN
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
      }

      else if(codice == MSG_GET_TERMINAL_TYPE && controllo_bulb(msg, id)){ //Invio il mio tipo
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
        concat_int(&risposta, BULB);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice == MSG_BULB_SWITCH_S && controllo_bulb(msg, id)){ //Inverto il mio stato
        inverti_stato(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice == MSG_BULB_SWITCH_I && controllo_bulb(msg, id)){ //Inverto il mio interruttore
        inverti_interruttore(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }

      else if(codice == MSG_GETTIME && controllo_bulb(msg, id)){ //Rispondo ad una richiesta di Tempo d'utilizzo(solo umano)
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        risposta.msg_type = MSG_FIFO;
        msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_AGGIUNGI && controllo_bulb(msg, id)){//Ad un messaggio di tipo aggiungi rispondo con ack negativo
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{ //Per qualsiasi altro messaggio rispondo con ack negativo
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          risposta.msg_type = MSG_FIFO;
          msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0); //mando un messaggio alla fifo
        }
        else{
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
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
    if((*s) == FALSE){
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

//Funzione che confronta due messaggi di con le info dei Bulb
//Ritorna false se sono diverse lo stato o l'interruttore
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

//Funzioni per la stampa delle info bulb
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

int leggi_info_bulb(msgbuf * m) {
  int r = FALSE;
  char ** ris;
  char tmp[100];
  char stampa[BUF_SIZE];
  protocoll_parser(m->msg_text, &ris);

  if ((codice_messaggio(ris) == MSG_INF_BULB) && ((atoi(ris[MSG_TYPE_MITTENTE]) == BULB) || (atoi(ris[MSG_TYPE_MITTENTE]) == DEFAULT))) {
    sprintf(tmp, "%s[BULB] : %s\n", ris[BULB_INF_NOME], ris[MSG_ID_MITTENTE]);
    strcpy(stampa, tmp);
    sprintf(tmp, "| Stato : %s\n", ris[BULB_INF_STATO]);
    strcat(stampa, tmp);
    sprintf(tmp, "| Interruttore : %s\n", ris[BULB_INF_INTERRUTTORE]);
    strcat(stampa, tmp);
    sprintf(tmp, "| Tempo di utilizzo : %s\n", ris[BULB_INF_TIME]);
    strcat(stampa, tmp);
    strcat(stampa, "| \\");
    r = TRUE;
    printf("\ninfo bulb:\n---------------------------------- \n%s\n----------------------------------\n",stampa);
  }
  return r;
}
