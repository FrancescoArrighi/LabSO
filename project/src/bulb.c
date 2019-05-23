//Header
#include "bulb.h"

/*Operazioni di una BULB
MSG_BULB_SWITCH_S = 410001
MSG_BULB_SWITCH_I = 410002
MSG_BULB_GETTIME  = 410003
MSG_BULB_GETINFO  = 410004
*/

// Funzione che genera una stringa con tutti i dati della bulb da stampare
char * print_bulb_info(int id, int s, int i, time_t t){
    char * tmp1 = "\nBulb - id: ";
    char * tmp2 = "\nStato: ";
    char * tmp3 = "\nInterruttore: ";
    char * tmp4 = "\nTempo di utilizzo: ";
    char * tmpid;
    itoa(id,&tmpid);
    char * tmpt;
    float tt = difftime(time(NULL),t);
    itoa(tt,&tmpt);
    char tmps[4];
    char tmpi[4];

    if(s == TRUE){
        strcpy(tmps, "ON");
    }
    else{
      strcpy(tmps, "OFF");
    }
    if(i == TRUE){
        strcpy(tmpi, "ON");
    }
    else{
      strcpy(tmpi, "OFF");
    }

    int dim = strlen(tmp1) + strlen(tmpid) + strlen(tmp2) + strlen(tmps) + strlen(tmp3) + strlen(tmpi) + strlen(tmp4) + strlen(tmpt) + 1;

    char * res = (char *) malloc (sizeof(char) * dim);

    strcpy(res, tmp1);
    strcat(res, tmpid);
    strcat(res, tmp2);
    strcat(res, tmps);
    strcat(res, tmp3);
    strcat(res, tmpi);
    strcat(res, tmp4);
    strcat(res, tmpt);

    return res;
}

/* Funzione Bulb */
void bulb(int id, int recupero){ //recupero booleano
  int status = FALSE;
  int interruttore = FALSE;
  time_t t_start = 0;
  char nome[] = "BULB";

  int queue;
  msgbuf messaggio;
  create_queue(id, &queue);

  pid_t idf = -1;

  int flag = FALSE;

  char ** msg;

  if(recupero == TRUE){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[BULB_INF_STATO]);
      interruttore = atoi(msg[BULB_INF_INTERRUTTORE]);
      t_start = atoi(msg[BULB_INF_TIME]);
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
      read(fd_read, buf_r, 80); // Leggo dalla FIFO
      printf("Ho letto il comando\n");
      printf("CMD : %s\n", buf_r);
      n_arg = str_split(buf_r, &cmd); // Numero di argomenti passati

      if(strcmp(cmd[0], "exit") == 0){ // se il comando inserito è exit esco
        printf("Fine lettura\n");
        flag = FALSE;
        //kill(getpid(),SIGTERM); //Io ucciderei il processo qua
      }
      else if (n_arg == 2){ //Accetto comandi del tipo "BULB qualcosa"
        if(strcmp(cmd[1], "interruttore") == 0){
          crea_messaggio_base(&messaggio, BULB, BULB, id, id, MSG_BULB_SWITCH_I);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
        }
        else {
          printf("Operazione non valida\n");
        }

      }
      else if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){
        if(strcmp(cmd[2], "time") == 0){
          crea_messaggio_base(&messaggio, BULB, BULB, id, id, MSG_BULB_GETTIME);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          richiesta = MSG_BULB_GETTIME;
        }
        else if(strcmp(cmd[2], "info") == 0){
          crea_messaggio_base(&messaggio, BULB, BULB, id, id, MSG_INF);
          send_message(queue, &messaggio, messaggio.msg_text, NUOVA_OPERAZIONE);
          richiesta = MSG_INF;
        }

        printf("Codice richiesta: %d\n", richiesta);
        if((richiesta > 0) && ((msgrcv(queue, &messaggio,sizeof(messaggio.msg_text), 5, 0)) != -1)) { //quando ricevo la risposta
          printf("Ricevuta risposta\n");
          fd_write = open(wfifo, O_WRONLY); //apro fifo di scrittura
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 80);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          //concateno i dati ricevuti
          if (richiesta == MSG_INF) {
            strcpy(buf_w, "\nNome: Bulb\n");
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
            sprintf(str_temp, "Tempo di utilizzo: %s\n", info_response[BULB_TIME]);
            strcpy(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1);
            }
          else {
            printf("Non ci concerne questa Operazione\n");
          }
          richiesta = -1;
          printf("Scrivo su fifo: %s\n", buf_w);
        }
      }

      close(fd_read);
      printf("File in lettura è stato chiuso\n");
      close(fd_write);
      printf("File in scrittura è stato chiuso\n");
    }
    unlink(rfifo); //una volta uscita dal ciclo elimino file fifo
    unlink(wfifo);
    //free(rfifo); // Libero la memoria allocata
  }

  else {
    msgbuf risposta;
    int q_ris;

    //Inizio Loop
    while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0)) != -1) {
      protocoll_parser(messaggio.msg_text, &msg);
      create_queue(atoi(msg[MSG_ID_MITTENTE]), & q_ris);
      printf("Sto per ricevere\n");
      printf("Cod %d\n", atoi(msg[MSG_OP]));

      if(codice_messaggio(msg) == MSG_INF && controllo_bulb(msg,id)) { //richiesta info su me stesso
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_BULB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_int(&risposta, status);
        concat_int(&risposta, interruttore);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          send_message(q_ris, &risposta, risposta.msg_text, 5); //mando un messaggio alla fifo
        }
        else{
          send_message(q_ris, &risposta, risposta.msg_text, 2);
        }
      }

      else if(codice_messaggio(msg) == MSG_OVERRIDE && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_BULB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]); //concat id padre
        concat_int(&risposta, status);
        concat_int(&risposta, interruttore);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }

      else if(codice_messaggio(msg) == MSG_SALVA_SPEGNI && controllo_bulb(msg, id)){
        concat_dati_bulb(&messaggio, status, interruttore, t_start);
        send_message(queue, &messaggio, messaggio.msg_text, 10);
        //printf("Lampadina pronta per essere eliminata\n");
        exit(EXIT_SUCCESS);
      }

      else if(codice_messaggio(msg) == MSG_SPEGNI && controllo_bulb(msg, id)){
        //kill(idf1, SIGTERM); - uccidere il sottoprocesso
        exit(EXIT_SUCCESS);
      }

      else if(codice_messaggio(msg) == MSG_RIMUOVIFIGLIO && controllo_bulb(msg, id)){
        concat_dati_bulb(&messaggio, status, interruttore, t_start);
        send_message(queue, &messaggio, messaggio.msg_text, 10);
        //printf("Lampadina pronta per essere eliminata\n");
        exit(EXIT_SUCCESS);
      }

      else if(codice_messaggio(msg) == MSG_GET_TERMINAL_TYPE && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
        concat_int(&risposta, BULB);
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }

      else if(codice_messaggio(msg) == MSG_BULB_SWITCH_S && controllo_bulb(msg, id)){
        inverti_stato(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }

      else if(codice_messaggio(msg) == MSG_BULB_SWITCH_I && controllo_bulb(msg, id)){
        inverti_interruttore(&status, &interruttore, &t_start);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }

      else if(codice_messaggio(msg) == MSG_BULB_GETTIME && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        concat_int(&risposta, tempo_bulb_on(status, t_start));
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          send_message(q_ris, &risposta, risposta.msg_text, 5); //mando un messaggio alla fifo
        }
        else{
          send_message(q_ris, &risposta, risposta.msg_text, 2);
        }
      }
      else if(codice_messaggio(msg) == MSG_AGGIUNGI && controllo_bulb(msg, id)){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
      else{
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), BULB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
  }
  printf("Errore lettura queue BULB\n");
}

// Funzione che prende i dati di una bulb e li concatena al messaggio salvato
// nel buffer
void concat_dati_bulb(msgbuf * m, int s, int i, time_t t){
  char * st;
  itoa(s, &st);
  char * it;
  itoa(i, &it);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * strlen(m->msg_text) + strlen(st) + 1 + strlen(it) + 1 + strlen(tt) + 2);
  strcpy(r,m->msg_text);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,it);
  strcat(r,"\n");
  strcat(r,tt);
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
