#include "window.h"

/* Operazioni di una Window
MSG_WINDOW_OPEN    = 610001
MSG_WINDOW_CLOSE   = 610002
MSG_WINDOW_GETTIME = 610003
MSG_WINDOW_GETINFO = 610004
*/

// Interruttori OPEN/CLOSE
// (on/off per aprire/chiudere: tornano subito in “off” dopo essere stati azionati)
// Questi interruttori sono sempre off quindi non esistono

//Funzione Window
void window(int id, int recupero, char * nome){
  signal(SIGCHLD, SIG_IGN); //evita che vengono creati processi zombie quando processi figli eseguono exit
  int status = FALSE;
  time_t t_start = 0; //Tempo per il quale è rimasta aperta
  char * name = strdup(nome);

  pid_t idf = -1;

  int queue;
  msgbuf messaggio;
  msgbuf risposta;
  msgbuf rec_buf;
  msgbuf tmp_buf;

  rec_buf.msg_type = 10;

  crea_queue(id, &queue);
  printf("Window: %d - pid: %d - ppid: %d\n", id, getpid(), getppid());

  int flag = FALSE;

  char ** msg;

  if(recupero == TRUE){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[WINDOW_REC_STATO]);
      t_start = atoi(msg[WINDOW_REC_TSTART]);
      name = (char *) malloc (sizeof(char) * (strlen(msg[WINDOW_REC_NOME]) + 1));
      strcpy(name, msg[WINDOW_REC_NOME]);
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
    char ** cmd;
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
      else if (n_arg == 2){
        if(strcmp(cmd[1], "open") == 0){
          crea_messaggio_base(&tmp_buf, WINDOW, WINDOW, id, id, MSG_WINDOW_OPEN);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
        }
        else if(strcmp(cmd[1], "close") == 0){
          crea_messaggio_base(&tmp_buf, WINDOW, WINDOW, id, id, MSG_WINDOW_CLOSE);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
        }
        else {
          printf("Operazione non valida\n");
        }
      }
      else if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){
        if(strcmp(cmd[2], "time") == 0){
          crea_messaggio_base(&tmp_buf, WINDOW, WINDOW, id, id, MSG_WINDOW_GETTIME);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          richiesta = MSG_WINDOW_GETTIME;
        }
        else if(strcmp(cmd[2], "info") == 0){
          crea_messaggio_base(&tmp_buf, WINDOW, WINDOW, id, id, MSG_INF);
          tmp_buf.msg_type = NUOVA_OPERAZIONE;
          msgsnd(queue, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
          richiesta = MSG_INF;
        }

        fd_write = open(wfifo, O_WRONLY); //apro fifo di scrittura

        printf("Codice richiesta: %d\n", richiesta);
        if(richiesta > 0) { //quando ricevo la risposta
          msgrcv(queue, &messaggio, sizeof(messaggio.msg_text), MSG_FIFO, 0);
          printf("Window fifo: Ricevuta risposta\n");
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 80);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          //concateno i dati ricevuti
          printf("Codice richiesta: %d\n", richiesta);
          if ((richiesta == MSG_INF) && codice_messaggio(info_response) == MSG_INF_WINDOW) {
            printf("Window fifo: Info\n");
            sprintf(str_temp, "\nNome: %s\n", info_response[WINDOW_INF_NOME]);
            strcpy(buf_w, str_temp);
            sprintf(str_temp, "Id: %s\n", info_response[MSG_ID_MITTENTE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Stato: %s\n", info_response[WINDOW_INF_STATO]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Tempo di utilizzo: %s\n", info_response[WINDOW_INF_TIME]);
            strcat(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1);
          }
          else if (richiesta == MSG_WINDOW_GETTIME) {
            printf("Bulb fifo: Info\n");
            sprintf(str_temp, "Tempo di utilizzo: %s\n", info_response[WINDOW_TIME]);
            strcpy(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1);
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

    //inizio loop
    while (TRUE) {
      printf("Window: Pronta per ricevere \n");
      msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
      protocoll_parser(messaggio.msg_text, &msg);
      int codice = codice_messaggio(msg);
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &q_ris);
      printf("Window ha ricevuto\n");
      printf("Cod %d\n", atoi(msg[MSG_OP]));

      if(codice == MSG_INF && controllo_window(msg,id)) { //richiesta info
        printf("Window: Richiesta info\n");
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_WINDOW);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_string(&risposta, name);
        concat_int(&risposta, status);
        concat_int(&risposta, tempo_window_on(status, t_start));
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          risposta.msg_type = MSG_FIFO;
          msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0); //mando un messaggio alla fifo
        }
        else{
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
      else if(codice == MSG_OVERRIDE && controllo_window(msg, id)){
        printf("Window: Messaggio di override\n");
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_WINDOW);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_string(&risposta, name);
        concat_int(&risposta, status);
        concat_int(&risposta, tempo_window_on(status, t_start));
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_SALVA_SPEGNI && controllo_window(msg, id)){
        printf("Window: Sto salvando i dati\n");
        crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_WINDOW);
        concat_int(&rec_buf, WINDOW);
        concat_int(&rec_buf, id);
        concat_dati_window(&rec_buf, status, t_start, name);
        msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
        exit(EXIT_SUCCESS);
      }
      else if(codice == MSG_SPEGNI && controllo_window(msg, id)){
        printf("Window: Spegnimento\n");
        if(idf > 0){
          kill(idf, SIGTERM);
        }
        exit(EXIT_SUCCESS);
      }
      else if(codice == MSG_RIMUOVIFIGLIO && controllo_window(msg, id)){
        printf("Window: ricevuto messaggio rimuovi figlio\n");
        int queue_deposito;
        crea_queue(DEPOSITO, &queue_deposito);
        if(id == atoi(msg[MSG_RIMUOVIFIGLIO_ID])){ //se sono io
          printf("Window: Sono io il figlio da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
          if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP){ //se la specifica è SPEC_DEP
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_WINDOW);
            concat_int(&rec_buf, WINDOW);
            concat_int(&rec_buf, id);
            concat_dati_window(&rec_buf, status, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            printf("Window: Dati salvati per il recupero, invio aggiungi al deposito e termino\n");
            crea_messaggio_base(&tmp_buf, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_AGGIUNGI); //il deposito deve aggiungere un nuovo frigo
            concat_int(&tmp_buf, id); // con mio stesso id
            tmp_buf.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue_deposito, &tmp_buf, sizeof(tmp_buf.msg_text), 0);
            exit(EXIT_SUCCESS); //termino processo
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_SALVA){ //se devo solo salvarmi
            printf("Window: salvo i dati\n");
            crea_messaggio_base(&rec_buf, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_RECUPERO_WINDOW);
            concat_int(&rec_buf, WINDOW);
            concat_int(&rec_buf, id);
            concat_dati_window(&rec_buf, status, t_start, name);
            msgsnd(queue, &rec_buf, sizeof(rec_buf.msg_text), 0);
            exit(EXIT_SUCCESS);
          }
          else if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEL){ //se devo solo terminare
            printf("Window: termino\n");
            if(idf >= 0){
              kill(idf, SIGTERM); //termino eventuale figlio che gestisce fifo
            }
            exit(EXIT_SUCCESS); // termino processo
          }
        }
        else{ // se disp da eliminare non sono io, mando un ACKN
          printf("Window: non sono il dispositivo da eliminare\n");
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
      else if(codice == MSG_GET_TERMINAL_TYPE && controllo_window(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
        concat_int(&risposta, WINDOW);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_WINDOW_OPEN && controllo_window(msg, id)){
        apri_window(&status, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_WINDOW_CLOSE && controllo_window(msg, id)){
        chiudi_window(&status);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_WINDOW_GETTIME && controllo_window(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        concat_int(&risposta, tempo_window_on(status, t_start));
        risposta.msg_type = MSG_FIFO;
        msgsnd(queue, &risposta, sizeof(risposta.msg_text), 0);
      }
      else if(codice == MSG_AGGIUNGI && controllo_window(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(q_ris, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
  }
}

void concat_dati_window(msgbuf * m, int s, time_t t, char * nw){
  char * st;
  itoa(s, &st);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * (strlen(m->msg_text) + strlen(st) + 1 + strlen(tt) + 1 + strlen(nw) + 2));
  strcpy(r,m->msg_text);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,tt);
  strcat(r,"\n");
  strcat(r,nw);
  strcat(r,"\n");
  strcpy(m->msg_text,r);

  free(r);
}

int controllo_window(char ** str, int id){
  int rt = FALSE;
  if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == DEFAULT){
      rt = TRUE;
  }
  return rt;
}

void apri_window(int * s, time_t * t) {
  if(!(*s)){
    (*t) = time(NULL);
  }
  (*s) = TRUE;
  printf("La finestra è stata aperta\n");
}

void chiudi_window(int * s) {
  (*s) = FALSE;
  printf("La finestra è stata chiusa\n");
}
// Funzione che restituisce il tempo di accensione della Lampadina
// Se la lampadina è spenta restituisce 0
int tempo_window_on(int s, time_t t) {
  int res = 0;
  if(s == TRUE){
    res = (int) difftime(time(NULL),t);
  }

  return res;
}

int equal_window(msgbuf * msg1, msgbuf * msg2){
  printf("Window: confronto in corso\n");
  int rt = FALSE;
  char **buf1, **buf2;
  protocoll_parser(msg1->msg_text, &buf1);
  protocoll_parser(msg2->msg_text, &buf2);
  if(strcmp(buf1[WINDOW_INF_STATO], buf2[WINDOW_INF_STATO]) == 0 && (strcmp(buf1[WINDOW_INF_NOME], buf2[WINDOW_INF_NOME]) == 0)) {
    printf("Window: Messaggi uguali\n");
    rt = TRUE;
  }
  else {
    printf("Window: Messaggi diversi\n");
  }

  return rt;
}

int stampa_info_window(msgbuf * m){
  int r = FALSE;
  char ** msg;
  protocoll_parser(m->msg_text, &msg);

  if ((codice_messaggio(msg) == MSG_INF_WINDOW) && ((atoi(msg[MSG_TYPE_MITTENTE]) == WINDOW) || (atoi(msg[MSG_TYPE_MITTENTE]) == DEFAULT))) {
    printf("\ninfo window:\n---------------------------------- \n");
    printf("%s[WINDOW] : %s\n", msg[WINDOW_INF_NOME], msg[MSG_ID_MITTENTE]);
    printf("| Stato : %s\n", msg[WINDOW_INF_STATO]);
    printf("| Tempo di utilizzo : %s\n", msg[WINDOW_INF_TIME]);
    printf("| \\ \n");
    printf("\n---------------------------------- \n\n");
    r = TRUE;
  }
  return r;
}
