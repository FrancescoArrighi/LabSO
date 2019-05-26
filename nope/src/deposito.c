#include "deposito.h"

void deposito(int id, int id_controller){
  int_list * figli;
  figli = create_int_list();
  int queue;
  msgbuf messaggio;
  crea_queue(id, &queue);

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  int dim_messaggio;
  printf("deposito - %d - pid = %d - %d\n", id,getpid(), queue);
  while (TRUE) {
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
    svuota_msg_queue(queue, 2);
    msg_type = messaggio.msg_type;
    dim_messaggio = protocoll_parser(messaggio.msg_text, &msg);
    //printf("mit: %d - op: %d - dest: %s\n", atoi(msg[MSG_ID_MITTENTE]), atoi(msg[MSG_OP]), msg[MSG_ID_DESTINATARIO]);
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;
    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      if(id_dest == DEFAULT || id_dest == id){
        //creo la risposta
        msgbuf risposta;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_DEPOSITO);
        concat_int(&risposta, DEFAULT);
        concat_string(&risposta, "Deposito");
        concat_int(&risposta, DEFAULT);
        concat_int(&risposta, figli->n);
        risposta.msg_type = 2;

        //invio la risposta
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);

        //creo la richiesta di info per i figli
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, DEFAULT, DEPOSITO, DEFAULT, id, MSG_INF);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);

        msgbuf risposta_figli;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int flag = TRUE;

        int i;

        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
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
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), DEPOSITO, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_INF);
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
            printf("timeout deposito risposte figli info - %d - pid = %d\n", id,getpid() );
          }
        }
        if(flag2 == TRUE){
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
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

      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
      risposta.msg_type = 2;
      msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);


      msgbuf richiesta_figli;

      crea_messaggio_base(&richiesta_figli, DEFAULT, DEPOSITO, DEFAULT, id, MSG_RIMUOVIFIGLIO);
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
    else if(codice_messaggio(msg) == MSG_AGGIUNGI){
      if(id_dest == DEFAULT || id_dest == id){
        //printf("aggiungo %d, per ordine di %d\n", atoi(msg[MSG_AGGIUNGI_IDF]), atoi(msg[MSG_ID_MITTENTE]));
        int q_nf;
        crea_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf risposta;
        char ** msg_risp_f;

        insert_int(q_nf, 0, figli);
        recupero_in_cascata(q_nf);
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
      }
      else{
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), DEPOSITO, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_AGGIUNGI);
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
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
          risposta.msg_type = 2;
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }
      }
    }
    else if(codice_messaggio(msg) == MSG_ADD_DEVICE){
      int q_nf;
      int new_type = atoi(msg[MSG_ADD_DEVICE_TYPE]);
      int new_id = atoi(msg[MSG_ADD_DEVICE_ID]);
      char * nome = malloc(strlen(msg[MSG_ADD_DEVICE_NOME])+1);
      strcpy(nome, msg[MSG_ADD_DEVICE_NOME]);
      crea_queue(new_id, &q_nf);

      insert_int(q_nf, 0, figli);

      if(new_type == HUB){
        if(fork() == 0){
          hub(new_id, FALSE, nome);
          //deposito(new_id,CONTROLLER);
          exit(0);
        }
      }
      else if(new_type == TIMER){
        if(fork() == 0){
          msgbuf t_b;
          ricomponi_messaggio(&(msg[MSG_ADD_DEVICE_TIMER_NEXTMSG]), dim_messaggio - MSG_ADD_DEVICE_TIMER_NEXTMSG,&t_b);
          dv_timer(new_id, FALSE, nome, t_b.msg_text, atoi(msg[MSG_ADD_DEVICE_TIMER_DELAY]));
          exit(0);
        }
      }
      else if(new_type == BULB){
        if(fork() == 0){
          bulb(new_id, FALSE, nome);
          exit(0);
        }
      }
      else if(new_type == WINDOW){
        if(fork() == 0){
          window(new_id, FALSE, nome);
          exit(0);
        }
      }
      else if(new_type == FRIDGE){
        if(fork() == 0){
          fridge(new_id, FALSE, nome);
          exit(0);
        }
      }
    }
    else if(codice_messaggio(msg) == MSG_GET_TERMINAL_TYPE ){
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
    else if(codice_messaggio(msg) == MSG_FRIDGE_SETTERMOSTATO){

      msgbuf risposta_figli, richiesta_figli, risposta;

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      char ** msg_risp_f;

      int i, flag1 = TRUE, flag2 = TRUE, dim_msg;

      if(id_dest == DEFAULT || id_dest == id){
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
    else if(codice_messaggio(msg) == MSG_DEPOSITO_DEL){
      int q, i, temp_int, flag = TRUE;
      //printf("eliminato: %s\n", msg[MSG_DEPOSITO_DEL_ID]);
      crea_queue(atoi(msg[MSG_DEPOSITO_DEL_ID]), &q);
      for(i = 0; i < figli->n && flag; i++){
        if(get_int(i, &temp_int, figli)){
          if(temp_int == q){
            rm_int(i, figli);
          }
        }
        else{
          flag = FALSE;
        }
      }
    }
    else{
      mesg_non_supp = TRUE;
    }
    if(codice_messaggio(msg) == MSG_SALVA_SPEGNI || codice_messaggio(msg) == MSG_SPEGNI){
      msgbuf richiesta_figli;
      if(codice_messaggio(msg) == MSG_SALVA_SPEGNI){
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), DEPOSITO, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_SALVA_SPEGNI);
      }
      else{
        crea_messaggio_base(&richiesta_figli, atoi(msg[MSG_TYPE_DESTINATARIO]), DEPOSITO, atoi(msg[MSG_ID_DESTINATARIO]), id, MSG_SPEGNI);
      }
      richiesta_figli.msg_type = NUOVA_OPERAZIONE;
      invia_broadcast(&richiesta_figli, figli);
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
      itoa(DEPOSITO, &msg_ric_f[MSG_TYPE_MITTENTE]);
      itoa(DEPOSITO, &msg_ric_f[MSG_ID_MITTENTE]);
      ricomponi_messaggio(msg_ric_f, temp_n, &richiesta_figli);

      richiesta_figli.msg_type = NUOVA_OPERAZIONE;

      invia_broadcast(&richiesta_figli, figli);

      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);

      int i;

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
  }
}
