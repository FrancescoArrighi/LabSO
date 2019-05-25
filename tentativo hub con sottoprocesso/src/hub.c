#include "hub.h"

int override(int_list * queues, int type, int myid, int myqueue, msgbuf * msg_example){
  msgbuf messaggio, risposta;
  int rt = FALSE;
  crea_messaggio_base(&messaggio, DEFAULT, HUB, DEFAULT, myid, MSG_OVERRIDE);
  messaggio.msg_type = NUOVA_OPERAZIONE;
  invia_broadcast(&messaggio, queues);
  int first_message = TRUE;
  int i, next, codice_msg;
  char ** msg;
  for(i = queues->n; i > 0; i--){
    if(leggi(myqueue, &risposta, 2, 2)){
      //printf("hub - %d - pid = %d - riga: %d\n------------------\n%s\n------------------\n", myid,getpid(), 14,messaggio.msg_text);
      protocoll_parser(risposta.msg_text, &msg);
      codice_msg = codice_messaggio(msg);
      if(codice_msg == MSG_ACKP){
        rt  = TRUE;
      }
      else{
        if(type == BULB && codice_msg == MSG_INF_BULB){
          if(first_message){
            strcpy(msg_example->msg_text, risposta.msg_text);
            msg_example->msg_type = risposta.msg_type;
            first_message = FALSE;
          }
          else{
            if(equal_bulb(*msg_example, risposta) == FALSE){
              rt = TRUE;
            }
          }
        }
        else if(type == WINDOW && codice_msg == MSG_INF_WINDOW){
          if(first_message){
            strcpy(msg_example->msg_text, risposta.msg_text);
            msg_example->msg_type = risposta.msg_type;
            first_message = FALSE;
          }
          else{
            if(first_message){
              strcpy(msg_example->msg_text, risposta.msg_text);
              msg_example->msg_type = risposta.msg_type;
              first_message = FALSE;
            }
            else{
              if(equal_window(*msg_example, risposta)  == FALSE){
                rt = TRUE;
              }
            }
          }
        }
        else if(type == FRIDGE && codice_msg == MSG_INF_FRIDGE){
          if(first_message){
            strcpy(msg_example->msg_text, risposta.msg_text);
            msg_example->msg_type = risposta.msg_type;
            first_message = FALSE;
          }
          else{
            if(equal_fridge(*msg_example, risposta) == FALSE){
              rt = TRUE;
            }
          }
        }
      }
    }
    else{
      i = 0;
    }
  }
  return rt;
}

void rimuovi_comunicazione_utente(int sig){
  close(fd_read); //chiudo fifo
  unlink(fifo_r); //una volta uscita dal ciclo elimino file fifo
}

int fd_write, fd_read;

int is_integer(char * str){
  int rt = FALSE;
  if(str != NULL){
    if(strlen(str) > 0){
      int i;
      rt = TRUE;
      for(i = 0; i <= strlen(str); i++){
        if(str[i] < '0' || str[i] > '9'){
          rt = FALSE;
        }
      }
    }
  }
  return rt;
}

void comunicazione_utente(int id_p){
    signal(SIGTERM, rimuovi_comunicazione_utente);

    int queue = crea_queue();
    msgbuf richiesta;

    char *fifo_r = (char *) malloc(sizeof(char)*20);
    sprintf(fifo_r, "/tmp/D_%d_R", id_p);
    char buf_r[BUF_SIZE];
    int n_arg;
    char **cmd, *str;

    if((mkfifo(fifo_w, 0666) == -1) && (errno != EEXIST)){ //se path esiste già continuo normalmente
      perror("Errore mkfifo");
      exit(1);
    }
    if((mkfifo(fifo_r, 0666) == -1) && (errno != EEXIST)){
      perror("Errore mkfifo");
      exit(1);
    }

    fd_read = open(fifo_r, O_RDONLY);

    while(TRUE){
      read(fd_read, buf_r, BUF_SIZE);
      n_arg = str_split(buf_r, &cmd);
      if(n_arg >= 6){
        if(is_integer([cmd[0]) && is_integer([cmd[1]) && is_integer([cmd[2]) && is_integer([cmd[3]) && is_integer([cmd[4]) || is_integer([cmd[5])){
          if(atoi([cmd[3]) == id && atoi([cmd[4]) == id){
            ricomponi_messaggio(&richiesta, n_arg - 1 ,cmd[1]);
            richiesta.msg_type = NUOVA_OPERAZIONE;
            msgsnd(queue, &richiesta, sizeof(richiesta.msg_text), 0);
          }
        }
      }
    }

  }
}

void fridge_open(int id, int *f_stato, time_t *t_start, int *allarme, int delay,int delay_recupero){
  (*f_stato) = TRUE;
  if(delay_recupero >= 0){ //se non sono in recupero
    (*t_start) = time(NULL); //salvo tempo inizio apertura
  }

  if((*allarme = fork()) == 0){ //inizializzo un timer
    if(delay_recupero < 0){ //se non sono in recupero
      sleep(delay); //chiusura dopo [delay] secondi
    }
    else{ // altrimenti
      sleep(delay_recupero); // chiusura dopo [delay_recupero] secondi
    }

    int queue;
    msgbuf messaggio;
    messaggio.msg_type = NUOVA_OPERAZIONE;
    crea_queue(id, &queue);
    crea_messaggio_base(&messaggio, FRIDGE, FRIDGE, id, id, MSG_FRIDGE_SETSTATO); //invio un messaggio a se stesso...
    concat_int(&messaggio, FALSE); //...con stato FALSE
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
    exit(0); //termino allarme
  }
}

void fridge_close(int id, int *f_stato, time_t *t_start, int *allarme){
  (*f_stato) = FALSE;
  (*t_start) = -1; //resetto tempo di inizio apertura

  if(*allarme >= 0){ //elimino eventuale timer
    kill(*allarme, SIGTERM);
    *allarme = -1;
  }
}




void hub(int id, int recupero, char * nome){
  int_list * figli = (int_list *) create_int_list();

  int win_stato = FALSE;

  int bulb_statp = FALSE;
  int bulb_inter = FALSE;

  int fridge_stato = FALSE;
  int fridge_interruttore = FALSE;
  int fridge_delay = 5;
  int fridge_t_start = -1;
  int fridge_termos = 3;

  int queue;
  msgbuf messaggio;

  crea_queue(id, &queue);
  printf("hub - %d - pid = %d - %d\n", id,getpid(), queue);

  if(recupero){
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0);

    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    char * str;
    nome = malloc(sizeof(char) * (strlen(msg[MSG_RECUPERO_HUB_NOME]) + 1));
    strcpy(nome, msg[MSG_RECUPERO_HUB_NOME]);
    int i;
    for( i = MSG_RECUPERO_HUB_INIZIOFIGLI; i < n; i++){
      insert_int(atoi(msg[i]), 0, figli);
      recupero_in_cascata(atoi(msg[i]));
    }
  }

  comunicazione_utente();
  char *fifo_w = (char *) malloc(sizeof(char)*20);
  sprintf(fifo_w, "/tmp/D_%d_W", id);
  fd_write = open(fifo_w, O_WDONLY);

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  while ( TRUE) {
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
    svuota_msg_queue(queue, 2);
    //printf("hub - %d - pid = %d - riga: %d\n------------------\n%s\n------------------\n", id,getpid(), 100,messaggio.msg_text);

    int type_child = DEFAULT;
    msg_type = messaggio.msg_type;
    protocoll_parser(messaggio.msg_text, &msg);
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;

    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      if(id_dest == DEFAULT || id_dest == id){
        //creo la risposta
        msgbuf risposta; //messaggio base + id_padre + nome + num figli + bool override
        risposta.msg_type = 2;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_HUB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]);  //si suppone che solo il padre possa fare una richiesta di questo tipo
        concat_string(&risposta, nome);
        concat_int(&risposta, type_child);
        concat_int(&risposta, figli->n);

        //controllo override
        msgbuf msg_example;
        if(id == 386){
          //printf("inizio overflow\n");
        }
        //concat_int(&risposta, override(figli, type_child,id,queue,&msg_example));
        concat_int(&risposta, 0);
        if(id == 386){
          //printf("fine\n");
        }
        //invio la risposta
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          write(fd_write, risposta.msg_text, BUF_SIZE);
        }
        else{
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }
        if(id == 386){
          //printf("myinfo/*\n%s\n/*\n", risposta.msg_text);
        }
        //creo la richiesta di info per i figli
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_INF);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);
        msgbuf risposta_figli;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int i, flag = TRUE;
        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("hub - %d - pid = %d - riga: %d\n------------------\n%s\n------------------\n", id,getpid(), 139,risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
              i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
            }
            if(atoi(msg_risp_f[MSG_INF_IDPADRE]) == DEFAULT){
              itoa(id, &(msg_risp_f[MSG_INF_IDPADRE]));
            }
            strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
            strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
            ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
            if(atoi(msg[MSG_ID_MITTENTE]) == id){
              write(fd_write, risposta.msg_text, BUF_SIZE);
            }
            else{
              msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
            }
          }
          else{
            flag = FALSE;
            printf("timeout deposito risposte figli info - %d - pid = %d\n", id,getpid() );
          }
        }
      }
      else{
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_INF);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);

        msgbuf risposta_figli, risposta;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int flag1 = TRUE;
        int flag2 = TRUE;

        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);

        int i;

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 176,risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
                i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
              }
              if(atoi(msg_risp_f[MSG_INF_IDPADRE]) == DEFAULT){
                itoa(id, &(msg_risp_f[MSG_INF_IDPADRE]));
              }
              strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
              strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
              ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
              risposta.msg_type = 2;
              if(atoi(msg[MSG_ID_MITTENTE]) == id){
                write(fd_write, risposta.msg_text, BUF_SIZE);
              }
              else{
                msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
              }
            }
          }
          else{
            flag1 = FALSE;
          }
        }
        if(flag2 == TRUE){
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          if(atoi(msg[MSG_ID_MITTENTE]) == id){
            write(fd_write, risposta.msg_text, BUF_SIZE);
          }
          else{
            msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
          }
        }
      }
    }
    else if(codice_messaggio(msg) == MSG_RIMUOVIFIGLIO){

      msgbuf risposta;
      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      //controllo se devo essere rimosso
      if(atoi(msg[MSG_RIMUOVIFIGLIO_ID]) == id){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        flag_rimuovi = TRUE;

        if(atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP){
          msgbuf msg_deposito;
          msg_deposito.msg_type = NUOVA_OPERAZIONE;
          crea_messaggio_base(&msg_deposito, DEPOSITO, HUB, DEPOSITO, id, MSG_AGGIUNGI);
          concat_int(&msg_deposito, id);
          int q_dep;
          crea_queue(DEPOSITO, &q_dep);
          msgsnd(q_dep, &msg_deposito, sizeof(msg_deposito.msg_text), 0);
        }
      }
      else{

        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);


        msgbuf richiesta_figli;

        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_RIMUOVIFIGLIO);
        concat_string(&richiesta_figli, msg[MSG_RIMUOVIFIGLIO_ID]);
        concat_string(&richiesta_figli, msg[MSG_RIMUOVIFIGLIO_SPEC]);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);

        msgbuf risposta_figli;
        char ** msg_risp_f;
        int codice_msg, dim_msg, temp_int;
        int flag = TRUE;

        int i;

        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 313,risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_ACKP){
              int q, id_figlio = atoi(msg_risp_f[MSG_ID_MITTENTE]);
              crea_queue(id_figlio, &q);
              for(i = 0; i < figli->n && get_int(i, &temp_int, figli); i++){
                if(temp_int == q){
                  rm_int(i, figli);
                }
              }
            }
          }
          else{
            flag = FALSE;
          }
        }
      }
      risposta.msg_type = 2;
      msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
    }
    else if(codice_messaggio(msg) == MSG_AGGIUNGI){
      if(id_dest == DEFAULT || id_dest == id){
        int q_nf;
        crea_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf richiesta_figlio, risposta_figlio, risposta, richiesta_dep, risposta_deb;
        char ** msg_risp_f;

        crea_messaggio_base(&richiesta_figlio, DEFAULT, HUB, DEFAULT, id, MSG_GET_TERMINAL_TYPE);
        richiesta_figlio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(q_nf, &richiesta_figlio, sizeof(richiesta_figlio.msg_text), 0);

        int flag = TRUE;
        int type_new_c = -1;
        while(flag){
          if(leggi(queue, &risposta_figlio, 2, 2)){
            //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 347,risposta_figlio.msg_text);
            protocoll_parser(risposta_figlio.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_MYTYPE){
              type_new_c = atoi(msg[MSG_MYTYPE_TYPE]);
              flag = FALSE;
            }
          }
          else{
            flag = FALSE;
          }
        }
        if(type_new_c > 0 && (type_new_c == type_child || type_child <= 0)){
          type_child = type_new_c;
          insert_int(q_nf, 0, figli);

          int q_dep;
          crea_queue(DEPOSITO, &q_dep);
          crea_messaggio_base(&richiesta_dep, DEPOSITO, HUB, DEPOSITO, id, MSG_DEPOSITO_DEL);
          concat_string(&richiesta_dep, msg[MSG_AGGIUNGI_IDF]);
          richiesta_dep.msg_type = NUOVA_OPERAZIONE;
          msgsnd(q_dep, &richiesta_dep, sizeof(richiesta_dep.msg_text), 0);

          crea_messaggio_base(&richiesta_figlio, DEFAULT, HUB, DEFAULT, id, MSG_SALVA_SPEGNI);
          richiesta_figlio.msg_type = NUOVA_OPERAZIONE;
          msgsnd(q_nf, &richiesta_figlio, sizeof(richiesta_figlio.msg_text), 0);

          recupero_in_cascata(q_nf);

          crea_messaggio_base(&risposta, DEPOSITO, HUB, DEPOSITO, id, MSG_ACKP);

        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        }
        risposta.msg_type = 2;
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);

      }
      else{

        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_AGGIUNGI);
        concat_string(&richiesta_figli, msg[MSG_AGGIUNGI_IDF]);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);

        msgbuf risposta_figli, risposta;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int flag1 = TRUE;
        int flag2 = TRUE;

        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);

        int i;

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 395,risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
              strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
              ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
              risposta.msg_type = 2;
              msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
            }
          }
          else{
            flag1 = FALSE;
          }
        }
        if(flag2 == TRUE){
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
      //printf("stato iniziale %d\n", id );
    }
    else if(codice_messaggio(msg) == MSG_GET_TERMINAL_TYPE ){
      if(id_dest == DEFAULT || id_dest == id){
        msgbuf risposta;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
        if(type_child > 0){
          concat_int(&risposta, type_child);
        }
        else{
          concat_int(&risposta, DEFAULT);
        }
        risposta.msg_type = 2;

        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{

        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_GET_TERMINAL_TYPE);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);

        msgbuf risposta_figli, risposta;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int flag1 = TRUE;
        int flag2 = TRUE;

        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);

        int i;

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 454,risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
              strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
              ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
              risposta.msg_type = 2;
              msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
            }
          }
          else{
            flag1 = FALSE;
          }
        }
        if(flag2 == TRUE){
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
    }
    else if(codice_messaggio(msg) == MSG_FRIDGE_SETTERMOSTATO ){
      msgbuf richiesta_figli, risposta_figli, riposta;
      concat_string(&richiesta_figli, msg[MSG_FRIDGE_VALORE]);

      int flag3 = FALSE;

      if(atoi(msg[MSG_ID_DESTINATARIO]) == id){
        fridge_termos = atoi(msg[MSG_FRIDGE_VALORE]);
        flag3 = TRUE;
        if(atoi(msg[MSG_ID_MITTENTE]) == id){
          crea_messaggio_base(&risposta, HUB, HUB, id, id, ACKP);
          risposta.msg_type = 2;
          write(fd_write, risposta.msg_text, BUF_SIZE);
        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, ACKP);
          risposta.msg_type = 2;
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_FRIDGE_SETTERMOSTATO);
      }
      else{
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_FRIDGE_SETTERMOSTATO);
      }

      invia_broadcast(risposta_figli);

      int i, flag1 = TRUE, flag2 = TRUE;
      char ** msg_risp_f;

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){

          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            flag2 = FALSE;
            strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
            strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
            ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
            risposta.msg_type = 2;
            msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
          }
        }
        else{
          flag1 = FALSE;
        }
      }
      if(flag2 == TRUE){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
    else{
      mesg_non_supp = TRUE;
    }
    if(codice_messaggio(msg) == MSG_SALVA_SPEGNI || codice_messaggio(msg) == MSG_SPEGNI || flag_rimuovi){
      //printf("spengo per %s\n",msg[MSG_ID_MITTENTE] );
      mesg_non_supp = FALSE;
      if(id_dest == DEFAULT || id_dest == id || flag_rimuovi){
        msgbuf msg_salva;
        msgbuf richiesta_figli;

        //creo il messaggio di ripristino
        if( codice_messaggio(msg) == MSG_SALVA_SPEGNI || ( codice_messaggio(msg) == MSG_RIMUOVIFIGLIO && (atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_SALVA || atoi(msg[MSG_RIMUOVIFIGLIO_SPEC]) == MSG_RIMUOVIFIGLIO_SPEC_DEP ) ) ) {
          //printf("====> %d\n",codice_messaggio(msg));
          crea_messaggio_base(&msg_salva, HUB, HUB, id, id, MSG_RECUPERO_HUB);
          concat_int(&msg_salva, HUB);
          concat_int(&msg_salva, id);
          concat_string(&msg_salva, nome);
          msg_salva.msg_type = 10;
          int i, next;
          for(i = 0; i < figli->n && get_int(i, &next, figli); i++){
            concat_int(&msg_salva, next);
          }
          msgsnd(queue, &msg_salva, sizeof(msg_salva.msg_text), 0);
          crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_SALVA_SPEGNI);
        }
        else{
          crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_SPEGNI);
        }

        //invio il messaggio di chiusura ai figli
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);
        exit(0);
      }
      else{

        msgbuf richiesta_figli;
        if(codice_messaggio(msg) == MSG_SALVA_SPEGNI){
          crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_SALVA_SPEGNI);
        }
        else{
          crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_SPEGNI);
        }
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);
      }
    }
    if(mesg_non_supp == TRUE){
      msgbuf risposta_figli, richiesta_figli, risposta;
      char ** msg_risp_f;
      char ** msg_ric_f;
      int codice_msg, dim_msg;
      int flag1 = TRUE;
      int flag2 = TRUE;

      strcpy(richiesta_figli.msg_text, messaggio.msg_text);
      int temp_n = protocoll_parser(richiesta_figli.msg_text, &msg_ric_f);
      strcpy(msg_ric_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_DESTINATARIO]);
      strcpy(msg_ric_f[MSG_ID_DESTINATARIO], msg[MSG_ID_DESTINATARIO]);
      itoa(HUB, &msg_ric_f[MSG_TYPE_MITTENTE]);
      itoa(HUB, &msg_ric_f[MSG_ID_MITTENTE]);
      ricomponi_messaggio(msg_ric_f, temp_n, &richiesta_figli);

      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);

      int i;

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
        //printf("hub - %d - pid = %d - riga: %d\n------------------\n%s\n------------------\n", id,getpid(), 545,risposta_figli.msg_text);
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            flag2 = FALSE;
            strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
            strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
            ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
            risposta.msg_type = 2;
            msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
          }
        }
        else{
          flag1 = FALSE;
        }
      }
      if(flag2 == TRUE){
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
  }
}
