//Header
#include "bulb.h"

/*Operazioni di una BULB
BULB_SWITCH_S  = 410001
BULB_SWITCH_I  = 410002
BULB_GETTIME   = 410003
BULB_GETINFO   = 410004
BULB_PRINTTIME = 411003
BULB_PRINTINFO = 411004
BULB_DEL       = 410005
BULB_KILL      = 410006
*/

// Funzione che crea una stringa "/tmp/D_id_" con l'id passato aggiungendo
//una W o R a seconda che il file sia Writer o Reader
char * percorso_file(int id, int tipo){
  char * tmp1 = "/tmp/D_";
  char * tmp2;
  itoa(id,&tmp2);
  char * r = (char *) malloc(sizeof(char) * (strlen(tmp1) + strlen(tmp2) + 3));
  strcpy(r, tmp1);
  strcat(r, tmp2);

  if (tipo == READ) {
    strcat(r, "_R");
  }
  else {
    strcat(r, "_W");
  }

  return r;
}

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
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == BULB || atoi(str[MSG_TYPE_DESTINATARIO]) == 0){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
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
float tempo_ON(int s, time_t t) {
  float res = 0;
  if(s == TRUE){
    res = difftime(time(NULL),t);
  }

  return res;
}

char * crea_msg_fifow(char * m, int id) {
  char * tmp;
  char ** md;
  protocoll_parser(m, &md);
  printf("Codice: %d\n", codice_messaggio(md));
  switch (codice_messaggio(md)) {
    case BULB_SWITCH_I:
      strcpy(tmp, "Interruttore invertito\n");
      break;
    case BULB_PRINTTIME:
      itoa(tempo_ON(atoi(md[MSG_OP + 1]), atoi(md[MSG_OP + 3])), &tmp);
      break;
    case BULB_PRINTINFO:
      strcpy(tmp, print_bulb_info(id, atoi(md[MSG_OP + 1]), atoi(md[MSG_OP + 2]), atoi(md[MSG_OP + 3])));
      break;
    default: printf("Errore nello switch della Write\n" ); //Gestisci errore
      break;
  }
  char * res = (char *) malloc (sizeof(char) * strlen(tmp) + 1);
  strcpy(res, tmp);
  printf("Ho creato il msg e non mi sono bloccato\n");
  return res;
}

void manda_msg_fifow(int f, char * p, char * testo) {
  char buf[100];
  strcpy(buf, testo);
  int flag = TRUE;

  printf("Sto quasi per scrivere\n");
  f = open(p, O_WRONLY);
  printf("Sto per scrivere\n");
  write(f, buf, strlen(buf)+1);
  printf("Ho inviato il msg e non mi sono bloccato: %s\n", buf);
  close(f);
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

  pid_t idf1 = -1;

  int flag = FALSE;
  char **cmd;
  char *str;

  char ** msg;

  if(recupero == TRUE){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[MSG_OP + 1]);
      interruttore = atoi(msg[MSG_OP + 2]);
      t_start = atoi(msg[MSG_OP + 3]);
    }
  }

  if((idf1 = fork()) == 0){ //Creo un processo figlio per leggere da input
    //Lettura da input
    int fd_write, fd_read, n_arg;
    int richiesta = 0;
    char buf_r[80];
    char buf_w[80];
    char * rfifo = percorso_file(id,READ);
    char * wfifo = percorso_file(id,WRITE);
    mkfifo(rfifo, 0666); // percorso e permessi
    //mkfifo(wfifo, 0666);
    flag = TRUE;

    while (flag) {
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
      else if((strcmp(cmd[0], nome) == 0) && (n_arg >= 2)){ //Accetto comandi del tipo "BULB qualcosa"
        if(strcmp(cmd[1], "interruttore") == 0){
          crea_messaggio_base(&str, BULB, BULB, id, id, BULB_SWITCH_I);
        }
        else if(strcmp(cmd[1], "time") == 0){
          crea_messaggio_base(&str, BULB, BULB, id, id, BULB_PRINTTIME);
          richiesta = BULB_PRINTTIME;
        }
        else if(strcmp(cmd[1], "info") == 0){
          crea_messaggio_base(&str, BULB, BULB, id, id, BULB_PRINTINFO);
          richiesta = BULB_PRINTINFO;
        }
        //printf("%s\n", str );
        send_message(queue, &messaggio, str, 1); // Invio il messaggio con il codice giusto
      }
      else {
        flag = FALSE;
        //Se ricevo qualcosa di diverso da ciò che è stato specificato
      }

      if (richiesta > 0) {
        fd_write = open(wfifo, O_WRONLY);

        if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 3, 0)) != -1){ //quando ricevo la risposta
          //printf("Ricevuta risposta info\n");
          //printf("\n\n%s\n", messaggio.msg_text);
          char **info_response;
          char *str_temp = (char *) malloc(sizeof(char) * 100);
          protocoll_parser(messaggio.msg_text, &info_response);
          memset(buf_w, 0, sizeof(buf_w)); //pulisco buf_w
          //concateno i dati ricevuti
          if (richiesta == BULB_PRINTINFO) {
            strcat(buf_w, "nome: Bulb%s\n");
            itoa(id, &str_temp);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "stato: %s\n", info_response[BULB_INF_STATO]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "interruttore: %s\n", info_response[BULB_INF_INTERRUTTORE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "time: %s\n", info_response[BULB_INF_TIME]);
            strcat(buf_w, str_temp);

            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
            //printf("%s\n", buf_w);
          }
          else if (richiesta == BULB_PRINTTIME) {
            sprintf(str_temp, "time: %s\n", info_response[BULB_INF_TIME]);
            strcat(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1);
            //printf("%s\n", buf_w);
          }
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
  //sleep(5);

  /*char * wfifo = percorso_file(id,WRITE);
  char buf[100];
  fd = open(wfifo, O_WRONLY);
  strcpy(buf, "Ciao");
  write(fd, buf, strlen(buf)+1);
  close(fd);
  printf("Ho inviato il msg e non mi sono bloccato: %s\n", buf);*/

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, &msg);
    //Manca la richiesta di tempo di utilizzo che arriva dal controller
    if(atoi(msg[MSG_OP]) == MSG_INF && controllo_bulb(msg,id) && (atoi(msg[MSG_ID_MITTENTE]) != id)) { //richiesta info su me stesso
      strcat(messaggio.msg_text, msg[MSG_INF_ID_PADRE]);
      strcat(messaggio.msg_text, "\n\0");
      concat_dati_bulb(&messaggio, status, interruttore, t_start);
      send_message(queue, &messaggio, messaggio.msg_text, 2); // Che priorita
    }
    else if (atoi(msg[MSG_OP]) >= 11000 && controllo_bulb(msg,id) && (atoi(msg[MSG_ID_MITTENTE]) == id)) { //Richiesta che attende una risposta
      switch (codice_messaggio(msg)) {
        case BULB_PRINTTIME:
          concat_dati_bulb(&messaggio, status, interruttore, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 3);
          break;
        case BULB_PRINTINFO:
          concat_dati_bulb(&messaggio, status, interruttore, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 3);
          break;
        default: printf("Richiesta specifica con risposta non definita\n" ); //Gestisci errore
          break;
      }
    }
    else if(atoi(msg[MSG_OP]) == BULB_KILL && controllo_bulb(msg, id)){
      //kill(idf1, SIGTERM); - uccidere il sottoprocesso
      exit(EXIT_SUCCESS);
    }
    else if (atoi(msg[MSG_OP]) >= 10000 && controllo_bulb(msg,id)) { // Richieste specifiche
      switch (codice_messaggio(msg)) {
        case BULB_SWITCH_S: //Comando inverti stato
          inverti_stato(&status, &interruttore, &t_start);
          break;
        case BULB_SWITCH_I: //Comando inverti interruttore
          inverti_interruttore(&status, &interruttore, &t_start);
          break;
        case BULB_DEL: //Comando che prepara all'eliminazione e manda il messaggio di recupero
          concat_dati_bulb(&messaggio, status, interruttore, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 10);
          printf("Lampadina pronta per essere eliminata\n");
          //printf("%s\n", messaggio.msg_text);
          break;
        case BULB_KILL: // Faccio solo un exit?
          //kill(idf1, SIGTERM);
          //kill(idf2, SIGTERM);
          exit(0);
          break;
        default: printf("Richiesta specifica senza risposta non definita\n" ); //Gestisci errore
          break;
      }
    }
  }
  printf("Errore lettura queue BULB\n");
}
