#include "hub.h"

int override(int_list * figli, int myid, int win_stato, int bulb_stato, int fridge_stato, int fridge_delay, int fridge_termos){

  msgbuf messaggio, risposta;
  int rt = FALSE;
  int my_queue;
  crea_queue(myid, &my_queue);
  crea_messaggio_base(&messaggio, DEFAULT, HUB, DEFAULT, myid, MSG_OVERRIDE);
  messaggio.msg_type = NUOVA_OPERAZIONE;
  invia_broadcast(&messaggio, figli);
  int i, codice_msg, flag = TRUE, temp_int;
  char ** msg;
  for(i = figli->n; i > 0 && (flag == TRUE); i--){
    if(leggi(myqueue, &risposta, 2, 2)){
      protocoll_parser(risposta.msg_text, &msg);
      codice_msg = codice_messaggio(msg);
      if(codice_msg == MSG_ACKP){
        rt = TRUE;
      }
      else if(codice_msg == MSG_OVERRIDE_RISP){
        if(is_integer(msg[MSG_OVERRIDE_WINST])){
          temp_int = atoi(msg[MSG_OVERRIDE_WINST]);
          if(temp_int >= 0 && temp_int != win_stato){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

        if(is_integer(msg[MSG_OVERRIDE_FRIDGEST])){
          temp_int = atoi(msg[MSG_OVERRIDE_FRIDGEST]);
          if(temp_int >= 0 && temp_int != fridge_stato){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

        if(is_integer(msg[MSG_OVERRIDE_FRIDGEINT])){
          temp_int = atoi(msg[MSG_OVERRIDE_FRIDGEINT]);
          if(temp_int >= 0 && temp_int != fridge_interruttore){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

        if(is_integer(msg[MSG_OVERRIDE_FRIDGEDLY])){
          temp_int = atoi(msg[MSG_OVERRIDE_FRIDGEDLY]);
          if(temp_int >= 0 && temp_int != fridge_delay){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

      }
      else if(codice_msg == MSG_INF_BULB){
        if(is_integer(msg[BULB_INF_STATO])){
          temp_int = atoi(msg[BULB_INF_STATO]);
          if(temp_int >= 0 && temp_int != bulb_stato){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }
      }
      else if(codice_msg == MSG_INF_WINDOW){
        if(is_integer(msg[WINDOW_INF_STATO])){
          temp_int = atoi(msg[WINDOW_INF_STATO]);
          if(temp_int >= 0 && temp_int != win_stato){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }
      }
      else if(codice_msg == MSG_INF_FRIDGE){
        if(is_integer(msg[MSG_FRIDGE_INF_STATO])){
          temp_int = atoi(msg[MSG_FRIDGE_INF_STATO]);
          if(temp_int >= 0 && temp_int != fridge_stato){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

        if(is_integer(msg[MSG_FRIDGE_INF_DELAY])){
          temp_int = atoi(msg[MSG_FRIDGE_INF_DELAY]);
          if(temp_int >= 0 && temp_int != fridge_delay){
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }

        if(is_integer(msg[MSG_FRIDGE_INF_TERM])){
          temp_int = atoi(msg[MSG_FRIDGE_INF_TERM]);
          if(temp_int >= 0 && temp_int != fridge_termos{
            rt = TRUE;
          }
        }
        else{
          rt = TRUE;
        }
      }
    }
    else{
      flag = FALSE;
    }
  }
  return rt;
}

void stampa_info_hub(msgbuf *buf){
  char ** ris;
  char tmp[100];
  char stampa[BUF_SIZE];
  protocoll_parser(m->msg_text, &ris);

  if(codice_messaggio(ris) == MSG_INF_HUB) {
    sprintf(tmp, "%s[HUB] : %s\n", ris[MSG_INF_NOME], ris[MSG_ID_MITTENTE]);
    strcpy(stampa, tmp);
    sprintf(tmp, "| Numero figli : %s\n", ris[MSG_INF_CONTROLDV_NFIGLI]);
    strcat(stampa, tmp);
    strcat(stampa, "| \\");
    printf("\ninfo bulb:\n---------------------------------- \n%s\n----------------------------------\n",stampa);
  }
}

void hub(int id, int recupero, char * nome){
  int_list * figli = (int_list *) create_int_list();

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

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;

  int win_stato = FALSE;

  int bulb_stato = FALSE;

  int fridge_stato = FALSE;
  int fridge_delay = 5;
  int fridge_t_start = -1;
  int fridge_termos = 3;

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
        concat_int(&risposta, override(figli, id, win_stato, bulb_stato, fridge_stato, fridge_delay, fridge_termos));
        //invio la risposta
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
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
            msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
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
    else if(codice_messaggio(msg) == MSG_OVERRIDE){

      if(id_dest == DEFAULT || id_dest == id){
        msgbuf risposta;

        //controllo override
        msgbuf msg_example;
        int rt = override(figli, id, win_stato, bulb_stato, fridge_stato, fridge_delay, fridge_termos);
        if(rt == FALSE){ // niente override
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_OVERRIDE_RISP);
          concat_int(&risposta, win_stato);
          concat_int(&risposta, bulb_stato);
          concat_int(&risposta, fridge_stato);
          concat_int(&risposta, fridge_delay);
          concat_int(&risposta, fridge_termos);
        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        }
        risposta.msg_type = 2;
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{

        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_OVERRIDE);
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
          //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 248,risposta_figli.msg_text);
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
        /*
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
        }*/
        //if(type_new_c > 0 && (type_new_c == type_child || type_child <= 0)){
          //type_child = type_new_c;
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

        /*}
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        }*/
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
    else if(codice_messaggio(msg) == MSG_FRIDGE_SETTERMOSTATO){

      msgbuf risposta_figli, richiesta_figli, risposta;

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      char ** msg_risp_f;

      int i, flag1 = TRUE, flag2 = TRUE, dim_msg;

      if(id_dest == DEFAULT || id_dest == id){
        fridge_termos = atoi(msg[MSG_FRIDGE_VALORE]);
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_FRIDGE_SETTERMOSTATO);
      }
      else{
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_FRIDGE_SETTERMOSTATO);
      }

      concat_string(&richiesta_figli, msg[MSG_FRIDGE_VALORE]);
      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
          //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 454,risposta_figli.msg_text);
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            if(flag2 == TRUE){
              flag2 = FALSE;
              crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
              risposta.msg_type = 2;
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
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
    else if(codice_messaggio(msg) == MSG_SETSTATO){

      msgbuf risposta_figli, richiesta_figli, risposta;

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      char ** msg_risp_f;

      int i, flag1 = TRUE, flag2 = TRUE, dim_msg;

      if(id_dest == DEFAULT || id_dest == id){
        bulb_stato = atoi(msg[MSG_SETSTATO_VAL]);
        fridge_stato = atoi(msg[MSG_SETSTATO_VAL]);
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_SETSTATO);
      }
      else{
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_SETSTATO);
      }

      concat_string(&richiesta_figli, msg[MSG_SETSTATO_VAL]);
      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
          //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 454,risposta_figli.msg_text);
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            if(flag2 == TRUE){
              flag2 = FALSE;
              crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
              risposta.msg_type = 2;
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
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
    else if(codice_messaggio(msg) == MSG_WINDOW_OPEN){

      msgbuf risposta_figli, richiesta_figli, risposta;

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      char ** msg_risp_f;

      int i, flag1 = TRUE, flag2 = TRUE, dim_msg;

      if(id_dest == DEFAULT || id_dest == id){
        win_stato = TRUE;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_WINDOW_OPEN);
      }
      else{
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_WINDOW_OPEN);
      }

      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
          //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 454,risposta_figli.msg_text);
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            if(flag2 == TRUE){
              flag2 = FALSE;
              crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
              risposta.msg_type = 2;
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
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
    }
    else if(codice_messaggio(msg) == MSG_WINDOW_CLOSE){

      msgbuf risposta_figli, richiesta_figli, risposta;

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      char ** msg_risp_f;

      int i, flag1 = TRUE, flag2 = TRUE, dim_msg;

      if(id_dest == DEFAULT || id_dest == id){
        win_stato = FALSE;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_WINDOW_CLOSE);
      }
      else{
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), HUB, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_WINDOW_CLOSE);
      }

      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      for(i = figli->n; i > 0 && flag1; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
          //printf("hub - %d - pid = %d - riga : %d\n------------------\n%s\n------------------\n", id,getpid(), 454,risposta_figli.msg_text);
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) != MSG_ACKN){
            if(flag2 == TRUE){
              flag2 = FALSE;
              crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
              risposta.msg_type = 2;
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
