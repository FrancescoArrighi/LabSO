#include "deposito.h"



void deposito(int id, int id_controller){
  int_list * figli;
  figli = create_int_list();
  printf("deposito - %d - pid = %d\n", id,getpid());
  int queue;
  msgbuf messaggio;
  crea_queue(id, &queue);

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  while (TRUE) {
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
    printf("deposito - %d - pid = %d\n------------------\n%s\n------------------\n", id,getpid(), messaggio.msg_text);
    msg_type = messaggio.msg_type;
    protocoll_parser(messaggio.msg_text, &msg);
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;
    printf("deposito - 3 \n");
    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      if(id_dest == DEFAULT || id_dest == id){
        printf("defosito - fail: \n");
        //creo la risposta
        msgbuf risposta;
        risposta.msg_type = 2;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_DEPOSITO);
        concat_int(&risposta, DEFAULT);
        concat_string(&risposta, "Deposito");
        concat_int(&risposta, DEFAULT);
        concat_int(&risposta, figli->n);

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
          printf("$\n");
          if(leggi(queue, &risposta_figli, 2, 2)){
            printf("$\ndeposito messaggio:\n-----------\n%s\n-----------\n\n", risposta_figli.msg_text);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
              i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
            }
            if(atoi(msg_risp_f[MSG_INF_IDPADRE]) == DEFAULT){
              itoa(id, &(msg_risp_f[MSG_INF_IDPADRE]));
            }
            printf("deposito - infoc - 1\n" );
            strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
            printf("deposito - infoc - 2\n" );
            strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
            printf("deposito - infoc - 2\n" );
            ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
            printf("deposito - infoc - 3\n" );
            msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
            printf("deposito - infoc - 4\n" );
          }
          else{
            flag = FALSE;
          }
          printf("deposito - infoc - FINE\n" );
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
        //printf("/////////////////////////////////// 0 \n");
        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            //printf("/////////////////////////////////// ===================== %d - 1 \n", figli->n-i);
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              //printf("/////////////////////////////////// 2 \n");
              //printf("/////////////////////////////////// okkk |%s| codice: %d; HUB: %d; TIMER: %d;\n", msg[MSG_INF_CONTROLDV_NFIGLI], codice_messaggio(msg_risp_f), MSG_INF_HUB, MSG_INF_TIMER);

              if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
                //printf("/////////////////////////////////// okkk |%s| codice: %d; HUB: %d; TIMER: %d;\n", msg[MSG_INF_CONTROLDV_NFIGLI], codice_messaggio(msg_risp_f), MSG_INF_HUB, MSG_INF_TIMER);
                int j = 0;
                for(; j < 10; j++){
                  //printf("!! [%d]: |%s|\n", j, msg[j]);
                }
                i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
                //printf("/////////////////////////////////// okkk 2 \n");
              }
              if(atoi(msg_risp_f[MSG_INF_IDPADRE]) == DEFAULT){
                itoa(id, &(msg_risp_f[MSG_INF_IDPADRE]));
              }
              //printf("/////////////////////////////////// 3 \n");
              //printf("defosito - 1 \n" );
              strcpy(msg_risp_f[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
              strcpy(msg_risp_f[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
              //printf("defosito - 2 \n");
              ricomponi_messaggio(msg_risp_f, dim_msg, &risposta);
              //printf("defosito - 3 \n");
              risposta.msg_type = 2;
              msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
              //printf("/////////////////////////////////// 4 \n");
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
      printf("passo d1\n");
    }
    else if(codice_messaggio(msg) == MSG_AGGIUNGI){
      if(id_dest == DEFAULT || id_dest == id){
        printf("passo d2\n");
        int q_nf;
        crea_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf risposta;
        char ** msg_risp_f;

        insert_int(q_nf, 0, figli);
        printf("passo d3\n");
        recupero_in_cascata(q_nf);
        printf("passo d4\n");
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        printf("passo d5\n");
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        printf("passo d6\n");
      }
      else{
        printf("passo d /\n");
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
      crea_queue(new_id, &q_nf);

      if(new_type == HUB){
        if(fork() == 0){
          hub(new_id, FALSE, "hub");
          //deposito(new_id,CONTROLLER);
          exit(0);
        }
      }
      else if(new_type == TIMER){
        if(fork() == 0){
          //timer();
          exit(0);
        }
      }
      else if(new_type == BULB){
        if(fork() == 0){
          //timer();
          exit(0);
        }
      }
      else if(new_type == WINDOW){
        if(fork() == 0){
          //timer();
          exit(0);
        }
      }
      else if(new_type == FRIDGE){
        if(fork() == 0){
          //timer();
          exit(0);
        }
      }
      printf("deposito - 4\n" );
      insert_int(q_nf, 0, figli);
      printf("deposito - 5\n" );
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
    else if(codice_messaggio(msg) == MSG_DEPOSITO_DEL){
      int q, i, temp_int, flag = TRUE;
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
    printf("deposito - fine\n" );
  }
}
