#include "window.h"

/* Operazioni di una WINDOW
MSG_WINDOW_OPEN    = 510001
MSG_WINDOW_CLOSE   = 510002
MSG_WINDOW_GETTIME = 510003
MSG_WINDOW_GETINFO = 510004
*/

// Interruttori OPEN/CLOSE
// (on/off per aprire/chiudere: tornano subito in “off” dopo essere stati azionati)
// Questi interruttori sono sempre off quindi non esistono

//Funzione Window
void window(int id, int recupero){
  int status = FALSE;
  time_t t_start = 0; //Tempo per il quale è rimasta aperta
  char nome[] = "WINDOW";

  int queue;
  msgbuf messaggio;
  create_queue(id, &queue);

  pid_t idf1 = -1;

  int flag = FALSE;
  char ** msg;

  if((idf1 = fork()) == 0){ //Creo un processo figlio per leggere da input
    //Lettura da input
    int fd_write, fd_read, n_arg;
    int richiesta = 0;
    char buf_r[80];
    char buf_w[100];
    msgbuf m_temp;
    char * str;
    char ** cmd;
    char * rfifo = percorso_file(id,READ);
    char * wfifo = percorso_file(id,WRITE);
    mkfifo(rfifo, 0666); // percorso e permessi
    mkfifo(wfifo, 0666);
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
      richiesta = 0;
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
          crea_messaggio_base(&m_temp, WINDOW, WINDOW, id, id, MSG_WINDOW_OPEN);
          send_message(queue, &m_temp, m_temp.msg_text, NUOVA_OPERAZIONE); // Invio il messaggio con il codice giusto
        }
        else if(strcmp(cmd[1], "close") == 0){
          crea_messaggio_base(&m_temp, WINDOW, WINDOW, id, id, MSG_WINDOW_CLOSE);
          send_message(queue, &m_temp, m_temp.msg_text, NUOVA_OPERAZIONE); // Invio il messaggio con il codice giusto
        }
        else if(strcmp(cmd[1], "time") == 0){
          crea_messaggio_base(&m_temp, WINDOW, WINDOW, id, id, MSG_WINDOW_GETTIME);
          send_message(queue, &m_temp, m_temp.msg_text, NUOVA_OPERAZIONE); // Invio il messaggio con il codice giusto
          richiesta = MSG_WINDOW_GETTIME;
        }
        else if(strcmp(cmd[1], "info") == 0){
          crea_messaggio_base(&m_temp, WINDOW, WINDOW, id, id, MSG_INF);
          send_message(queue, &m_temp, m_temp.msg_text, NUOVA_OPERAZIONE); // Invio il messaggio con il codice giusto
          richiesta = MSG_INF;
        }
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
          if (richiesta == MSG_INF) {
            strcat(buf_w, "Nome: Window%s\n");
            sprintf(str_temp, "Id: %s\n", info_response[MSG_ID_MITTENTE]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Stato: %s\n", info_response[WINDOW_INF_STATO]);
            strcat(buf_w, str_temp);
            sprintf(str_temp, "Tempo di utilizzo: %s\n", info_response[WINDOW_INF_TIME]);
            strcat(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1); //srivo su wfifo buf_w
            //printf("%s\n", buf_w);
          }
          else if (richiesta == MSG_WINDOW_GETTIME) {
            sprintf(str_temp, "time: %s\n", info_response[WINDOW_TIME]);
            strcat(buf_w, str_temp);
            write(fd_write, buf_w, strlen(buf_w)+1);
            //printf("%s\n", buf_w);
          }
          else {
            printf("Non ci concerne questa Operazione\n");
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
      status = atoi(msg[WINDOW_INF_STATO]);
      t_start = atoi(msg[WINDOW_INF_TSTART]);
    }
    printf("fine\n" );
  }

  msgbuf risposta;
  int q_ris;

  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 2, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, &msg);
    create_queue(atoi(msg[MSG_ID_MITTENTE]), & q_ris);

    if(atoi(msg[MSG_OP]) == MSG_INF && controllo_window(msg,id) && (atoi(msg[MSG_ID_MITTENTE]) != id)) { //richiesta info su me stesso
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_WINDOW);
      concat_int(&risposta, status);
      concat_int(&risposta, tempo_window_on(status, t_start));
      if(atoi(msg[MSG_ID_MITTENTE]) == id){
        send_message(q_ris, &risposta, risposta.msg_text, NUOVA_OPERAZIONE); //mando un messaggio alla fifo
      }
      else{
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
    else if(atoi(msg[MSG_OP]) == MSG_OVERRIDE && controllo_window(msg, id)){
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF);
      concat_int(&risposta, status);
      concat_int(&risposta, tempo_window_on(status, t_start));
      send_message(q_ris, &risposta, risposta.msg_text, 2);
    }
    else if(atoi(msg[MSG_OP]) == MSG_SALVA_SPEGNI && controllo_window(msg, id)){
      concat_dati_window(&messaggio, status, t_start);
      send_message(queue, &messaggio, messaggio.msg_text, 10);
      //printf("Lampadina pronta per essere eliminata\n");
      exit(EXIT_SUCCESS);
    }
    else if(atoi(msg[MSG_OP]) == MSG_SPEGNI && controllo_window(msg, id)){
      //kill(idf1, SIGTERM); - uccidere il sottoprocesso
      exit(EXIT_SUCCESS);
    }
    else if(atoi(msg[MSG_OP]) == MSG_RIMUOVIFIGLIO && controllo_window(msg, id)){
      concat_dati_window(&messaggio, status, t_start);
      send_message(queue, &messaggio, messaggio.msg_text, 10);
      //printf("Lampadina pronta per essere eliminata\n");
      exit(EXIT_SUCCESS);
    }
    else if(atoi(msg[MSG_OP]) == MSG_GET_TERMINAL_TYPE && controllo_window(msg, id)){
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
      concat_int(&risposta, WINDOW);
      send_message(q_ris, &risposta, risposta.msg_text, 2);
    }
    else if(atoi(msg[MSG_OP]) == MSG_WINDOW_OPEN && controllo_window(msg, id)){
      apri_window(&status, &t_start);
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
      if(atoi(msg[MSG_ID_MITTENTE]) == id){
        send_message(q_ris, &risposta, risposta.msg_text, NUOVA_OPERAZIONE);
      }
      else{
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
    else if(atoi(msg[MSG_OP]) == MSG_WINDOW_CLOSE && controllo_window(msg, id)){
      chiudi_window(&status);
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
      if(atoi(msg[MSG_ID_MITTENTE]) == id){
        send_message(q_ris, &risposta, risposta.msg_text, NUOVA_OPERAZIONE);
      }
      else{
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
    else if(atoi(msg[MSG_OP]) == MSG_WINDOW_GETTIME && controllo_window(msg, id)){
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
      concat_int(&risposta, tempo_window_on(status, t_start));
      if(atoi(msg[MSG_ID_MITTENTE]) == id){
        send_message(q_ris, &risposta, risposta.msg_text, NUOVA_OPERAZIONE);
      }
      else{
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
    else if(atoi(msg[MSG_OP]) == MSG_AGGIUNGI && controllo_window(msg, id)){
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
      send_message(q_ris, &risposta, risposta.msg_text, 2);
    }
    else{
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), WINDOW, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
      if(atoi(msg[MSG_ID_MITTENTE]) == id){
        send_message(q_ris, &risposta, risposta.msg_text, NUOVA_OPERAZIONE);
      }
      else{
        send_message(q_ris, &risposta, risposta.msg_text, 2);
      }
    }
  }
  printf("Errore lettura queue WINDOW\n");
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
  if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
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
