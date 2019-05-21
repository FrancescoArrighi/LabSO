#include "window.h"

/* Operazioni di una WINDOW
WINDOW_OPEN    = 510001
WINDOW_CLOSE   = 510002
WINDOW_GETTIME = 510003
WINDOW_GETINFO   = 510004
WINDOW_PRINTTIME = 511003
WINDOW_PRINTINFO = 511004
WINDOW_DEL     = 510005
WINDOW_KILL      = 510006
*/

// Interruttori OPEN/CLOSE
// (on/off per aprire/chiudere: tornano subito in “off” dopo essere stati azionati)
// Questi interruttori sono sempre off quindi non esistono
/*char * percorso_file(int id, int tipo){
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
}*/

void window_info_r(int id, int s, time_t t){
    printf("Window - %d\n", id);
    //Stato e Tempo di apertura
    if(s == TRUE){
        printf("Stato: OPEN\n" );
        printf("Tempo di apertura: %2lf\n", difftime(time(NULL),t));
    }
    else{
      printf("Stato: CLOSE\n");
      printf("Finestra chiusa\n");
    }
    //Aggiungere campo interruttori ? sempre OFF
}
//Da sistemare
void window_m(char * m, int did, int operazione){
  char * msg;
  crea_messaggio_base(&msg, WINDOW, HUB, did, HUB, operazione);
  strcpy(m,msg);
}

void concat_dati_window(msgbuf * m, int s, time_t t){
  char * st;
  itoa(s, &st);
  char * tt;
  itoa(t, &tt);
  char * r = (char *) malloc(sizeof(char) * (strlen(m->msg_text) + strlen(st) + 1 + strlen(tt) + 2));
  strcpy(r,m->msg_text);
  strcat(r,st);
  strcat(r,"\n");
  strcat(r,tt);
  strcat(r,"\n");
  strcpy(m->msg_text,r);

  free(r);
}

int controllo_window(char ** str, int id){
  int rt = FALSE;
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == WINDOW || atoi(str[MSG_TYPE_DESTINATARIO]) == 0){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
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
//Funzione Window
void window(int id, int recupero){
  int status = FALSE;
  time_t t_start = 0; //Tempo per il quale è rimasta aperta
  char nome[] = "WINDOW-ID";

  int queue;
  msgbuf messaggio;
  create_queue(id, &queue);
  pid_t idf1 = -1;
  int flag = FALSE;
  char ** cmd;
  char * str;
  char ** msg;

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
      else if((strcmp(cmd[0], nome) == 0) && (n_arg >= 2)){ //Accetto comandi del tipo "WINDOW qualcosa"
        if(strcmp(cmd[1], "open") == 0){
          crea_messaggio_base(&str, WINDOW, WINDOW, id, id, WINDOW_OPEN);
        }
        else if(strcmp(cmd[1], "close") == 0){
          crea_messaggio_base(&str, WINDOW, WINDOW, id, id, WINDOW_CLOSE);
        }
        else if(strcmp(cmd[1], "time") == 0){
          crea_messaggio_base(&str, WINDOW, WINDOW, id, id, WINDOW_PRINTTIME);
          richiesta = WINDOW_PRINTTIME;
        }
        else if(strcmp(cmd[1], "info") == 0){
          crea_messaggio_base(&str, WINDOW, WINDOW, id, id, WINDOW_PRINTINFO);
          richiesta = WINDOW_PRINTINFO;
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
          if (richiesta == WINDOW_PRINTINFO) {
            strcat(buf_w, "nome: Bulb%s\n");
            itoa(id, &str_temp);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "stato: %s\n", info_response[WINDOW_INF_STATO]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "interruttore: %s\n", info_response[WINDOW_INF_INTERRUTTORE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "time: %s\n", info_response[WINDOW_INF_TIME]);
            strcat(buf_w, str_temp);

            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su fifo buf_w
            //printf("%s\n", buf_w);
          }
          else if (richiesta == WINDOW_PRINTTIME) {
            sprintf(str_temp, "time: %s\n", info_response[WINDOW_INF_TIME]);
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


  if(recupero){
    printf("inizio\n" );
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      printf("%s\n", messaggio.msg_text);
      protocoll_parser(messaggio.msg_text, &msg);
      status = atoi(msg[MSG_OP + 1]);
      t_start = atoi(msg[MSG_OP + 2]);
    }
    printf("fine\n" );
  }

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 2, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, &msg);

    //Manca la richiesta di tempo di utilizzo che arriva dal controller
    if(atoi(msg[MSG_OP]) == MSG_INF && controllo_window(msg,id) && (atoi(msg[MSG_ID_MITTENTE]) != id)) { //richiesta info su me stesso
      strcat(messaggio.msg_text, msg[MSG_INF_ID_PADRE]);
      strcat(messaggio.msg_text, "\n\0");
      concat_dati_window(&messaggio, status, t_start);
      send_message(queue, &messaggio, messaggio.msg_text, 2); // Che priorita
    }
    else if (atoi(msg[MSG_OP]) >= 11000 && controllo_window(msg,id) && (atoi(msg[MSG_ID_MITTENTE]) == id)) { //Richiesta che attende una risposta
      switch (codice_messaggio(msg)) {
        case WINDOW_PRINTTIME:
          concat_dati_window(&messaggio, status, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 3);
          break;
        case WINDOW_PRINTINFO:
          concat_dati_window(&messaggio, status, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 3);
          break;
        default: printf("Richiesta specifica con risposta non definita\n" ); //Gestisci errore
          break;
      }
    }
    else if(atoi(msg[MSG_OP]) == WINDOW_KILL && controllo_window(msg, id)){
      //kill(idf1, SIGTERM); - uccidere il sottoprocesso
      exit(EXIT_SUCCESS);
    }
    else if (atoi(msg[MSG_OP]) >= 10000 && controllo_window(msg,id)) { // Richieste specifiche
      switch (codice_messaggio(msg)) {
        case WINDOW_OPEN:
          apri_window(&status, &t_start);
            break;
        case WINDOW_CLOSE:
          chiudi_window(&status);
            break;
        case WINDOW_DEL:
          concat_dati_window(&messaggio, status, t_start);
          send_message(queue, &messaggio, messaggio.msg_text, 10);
          printf("Finestra pronta per essere eliminata\n");
          //printf("%s\n", messaggio.msg_text);
          kill(idf1, SIGTERM);
          exit(0);
            break;
        default: printf("Errore nello switch\n" ); //Gestisci errore
            break;
      }

    }
  }
  printf("Errore lettura queue WINDOW\n");
}
