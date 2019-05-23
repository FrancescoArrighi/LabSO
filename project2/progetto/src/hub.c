#include "hub.h"
#include "useful_fun.h"

int override(int_list * queues, int type, int myid, int myqueue, msgbuf * msg_example){
  msgbuf messaggio;
  int rt = FALSE;
  messaggio.msg_type = NUOVA_OPERAZIONE;
  crea_messaggio_base(&messaggio, DEFAULT, HUB, DEFAULT, myid, MSG_OVERRIDE);
  invia_broadcast(&messaggio, queues);
  int first_message = TRUE;
  int i, next, codice_msg;
  char ** msg;
  for(i = queues->n; i > 0; i--){
    if(leggi(myqueue, &messaggio, 2, 2)){
      protocoll_parser(messaggio.msg_text, &msg);
      codice_msg = codice_messaggio(msg);
      if(codice_msg == MSG_ACKP){
        rt  = TRUE;
      }
      else{
        if(type == BULB && codice_msg == MSG_INF_BULB){
          if(first_message){
            strcpy(msg_example->msg_text, messaggio.msg_text);
            msg_example->msg_type = messaggio.msg_type;
            first_message = FALSE;
          }
          else{
            if(equal_bulb(*msg_example, messaggio) == FALSE){
              rt = TRUE;
            }
          }
        }
        else if(type == WINDOW && codice_msg == MSG_INF_WINDOW){
          if(first_message){
            strcpy(msg_example->msg_text, messaggio.msg_text);
            msg_example->msg_type = messaggio.msg_type;
            first_message = FALSE;
          }
          else{
            if(equal_window(*msg_example, messaggio)  == FALSE){
              rt = TRUE;
            }
          }
        }
        else if(type == FRIDGE && codice_msg == MSG_INF_FRIDGE){
          if(first_message){
            strcpy(msg_example->msg_text, messaggio.msg_text);
            msg_example->msg_type = messaggio.msg_type;
            first_message = FALSE;
          }
          else{
            if(equal_fridge(*msg_example, messaggio) == FALSE){
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

void hub(int id, int recupero, char * nome){
  int_list * figli = create_int_list();

  printf("hub - %d - pid = %d\n", id,getpid());

  int queue;
  msgbuf messaggio;

  crea_queue(id, &queue);

  if(recupero){
    printf("passo6\n");
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0);
    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    char * str;
    strcpy(nome, msg[MSG_RECUPERO_HUB_NOME]);
    int i;
    for( i = MSG_RECUPERO_HUB_INIZIOFIGLI; i < n; i++){
      insert_int(atoi(msg[i]), 0, figli);
      recupero_in_cascata(atoi(msg[i]));
    }
    printf("passo7\n");
  }

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  while ( TRUE) {
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0);
    perror("hub lettura ");
    //printf("1\n");

    int type_child;
    //printf("2\n");
    msg_type = messaggio.msg_type;
    //printf("parser\n");
    protocoll_parser(messaggio.msg_text, &msg);
    //printf("fine parser\natoi\n");
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    //printf("fine atoi\n");
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;

    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      //printf("hub - inf\n" );
      if(id_dest == DEFAULT || id_dest == id){
        //printf("hub - io\n" );
        //creo la risposta
        msgbuf risposta; //messaggio base + id_padre + nome + num figli + bool override
        risposta.msg_type = 2;
        //printf("hub - risposta\n" );
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_HUB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]);  //si suppone che solo il padre possa fare una richiesta di questo tipo
        concat_string(&risposta, nome);
        //printf("hub - risp: \n---------------------------\n%s\n---------------------------\n",  risposta.msg_text);
        concat_int(&risposta, type_child);
        //printf("hub - risp: \n---------------------------\n%s\n---------------------------\n",  risposta.msg_text);
        concat_int(&risposta, figli->n);
        //printf("hub - risp: \n---------------------------\n%s\n---------------------------\n",  risposta.msg_text);
        //printf("hub - preover\n" );
        //controllo override
        msgbuf msg_example;
        concat_int(&risposta, override(figli, type_child,id,queue,&msg_example));
        //printf("hub - dopoover\n" );

        //invio la risposta
        int msg_queue_mit;
        crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
        //printf("hub - risp: \n---------------------------\n%s\n---------------------------\n",  risposta.msg_text);
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        //printf("hub - inf\n" );
        //creo la richiesta di info per i figli
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_INF);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        //printf("hub - richiesta figli:\n---------------------------\n%s\n---------------------------\n", richiesta_figli.msg_text );
        invia_broadcast(&richiesta_figli, figli);
        //printf("hub - fine richiesta figli\n");
        msgbuf risposta_figli;
        char ** msg_risp_f;
        int codice_msg, dim_msg;
        int i, flag = TRUE;
        //printf("hub - inizio lettura dai figli\n");
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
          }
        }
        //printf("hub - fine\n");
      }
      else{
        //printf("hub - nope\n" );
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
        int rt = override(figli, type_child,id,queue,&msg_example);
        if(rt == FALSE){ // niente override
          char ** example;
          int dim_r = protocoll_parser(risposta.msg_text, &example);
          strcpy(example[MSG_ID_DESTINATARIO], msg[MSG_ID_MITTENTE]);
          strcpy(example[MSG_TYPE_DESTINATARIO], msg[MSG_TYPE_MITTENTE]);
          itoa(id, &example[MSG_ID_MITTENTE]);
          ricomponi_messaggio(example, dim_r, &risposta);
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
        risposta.msg_type = 2;
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        flag_rimuovi = TRUE;

        msgbuf msg_deposito;
        msg_deposito.msg_type = NUOVA_OPERAZIONE;
        crea_messaggio_base(&msg_deposito, DEPOSITO, HUB, DEPOSITO, id, MSG_AGGIUNGI);
        concat_int(&msg_deposito, id);
        int q_dep;
        crea_queue(DEPOSITO, &q_dep);
        msgsnd(q_dep, &msg_deposito, sizeof(msg_deposito.msg_text), 0);
        printf("passo1\n");
      }
      else{

        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);


        msgbuf richiesta_figli;

        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_RIMUOVIFIGLIO);
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
      }
    }
    else if(codice_messaggio(msg) == MSG_AGGIUNGI){
      if(id_dest == DEFAULT || id_dest == id){
        int q_nf;
        crea_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf richiesta_figlio, risposta_figlio, risposta;
        char ** msg_risp_f;

        crea_messaggio_base(&richiesta_figlio, DEFAULT, HUB, DEFAULT, id, MSG_GET_TERMINAL_TYPE);
        richiesta_figlio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(q_nf, &richiesta_figlio, sizeof(richiesta_figlio.msg_text), 0);

        int flag = TRUE;
        int type_new_c = -1;
        while(flag){
          if(leggi(queue, &risposta_figlio, 2, 2)){
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

          recupero_in_cascata(q_nf);

          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
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
    else if(codice_messaggio(msg) == MSG_GET_TERMINAL_TYPE ){
      if(id_dest == DEFAULT || id_dest == id){
        msgbuf risposta;
        if(type_child > 0){
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_MYTYPE);
          concat_int(&risposta, HUB);
        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        }
        concat_int(&messaggio, type_child);
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
    else{
      mesg_non_supp = TRUE;
    }
    if(codice_messaggio(msg) == MSG_SALVA_SPEGNI || codice_messaggio(msg) == MSG_SPEGNI || flag_rimuovi){
      printf("passo2\n");
      mesg_non_supp = FALSE;
      if(id_dest == DEFAULT || id_dest == id || flag_rimuovi){
        msgbuf msg_salva;

        //creo il messaggio di ripristino
        if(MSG_SALVA_SPEGNI || flag_rimuovi){
          printf("passo3\n");
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
          printf("passo4\n");
        }

        //invio il messaggio di chiusura ai figli
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_SALVA_SPEGNI);
        richiesta_figli.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&richiesta_figli, figli);
        printf("passo5\n");
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
  //printf("sono stronzo\n" );
}
