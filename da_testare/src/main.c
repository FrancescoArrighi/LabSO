#include "main.h"


tree_device * create_tree_device(){
    tree_device * rt = (tree_device *) malloc(sizeof(tree_device));
    rt->root = NULL;
    return rt;
}

tree_device_node * tree_get_node(tree_device_node * tree, int id){
  tree_device_node * rt = NULL;
  //printf("tree insert get 1 - %d\n", id );
  if(tree != NULL){
    //printf("tree insert get 2\n" );
    if(tree->id == id){
      //printf("tree insert get 3\n" );
      rt = tree;
    }
    else{
      //printf("tree insert get 4\n" );
      int i;
      for(i = 0; i < tree->nfigli && rt == NULL && tree->figli[i] != NULL; i++){
        //printf("tree insert get 5\n" );
        rt = tree_get_node(tree->figli[i], id);
      }
    }
  }
  //printf("tree insert get 6\n" );
  return rt;
}

int tree_insert_device(tree_device * tree, int id_padre, int id, int pid, int n_figli, int type, char * name){
    int rt = TRUE;
    tree_device_node * node = (tree_device_node *) malloc(sizeof(tree_device_node));
    ////printf("tree insert insert 1\n" );
    node->id = id;
    node->type = type;
    node->pid = pid;
    node->nome = (char *) malloc(sizeof(char) * (strlen(name)+1));
    ////printf("tree insert insert 2\n" );
    strcpy(node->nome, name);
    ////printf("tree insert insert 3\n" );
    node->nfigli = n_figli;
    ////printf("tree insert insert 4\n" );
    node->figli = (tree_device_node ** ) malloc(sizeof(tree_device_node *) * n_figli);
    ////printf("tree insert insert 5\n" );
    int i;
    for(i = 0; i < n_figli; i++){
      node->figli[i] = NULL;
      ////printf("tree insert insert 6 - %d\n", i );
    }
    ////printf("tree insert insert 7\n" );
    if(id_padre <  0){ //inserisci in testa
      ////printf("tree insert insert 8\n" );
      tree->root = node;
    }
    else{
      ////printf("tree insert insert 9\n" );
      tree_device_node * padre = tree_get_node(tree->root, id_padre);
      ////printf("tree insert insert 10\n" );
      for(i = 0; i < padre->nfigli && padre->figli[i] != NULL; i++);
        if(i < padre->nfigli){
          padre->figli[i] = node;
        }
    }
    ////printf("tree insert insert 11\n" );
    return rt;
}

void tree_print_branch(tree_device_node * node, char * str){
  char * str_type;
  if(node->type == CONTROLLER){
    str_type = (char *) malloc(sizeof(char) * 11);
    strcpy(str_type, "Controller");
  }
  else if(node->type == HUB){
    str_type = (char *) malloc(sizeof(char) * 4);
    strcpy(str_type, "Hub");
  }
  else if(node->type == TIMER){
    str_type = (char *) malloc(sizeof(char) * 6);
    strcpy(str_type, "Timer");
  }
  else if(node->type == BULB){
    str_type = (char *) malloc(sizeof(char) * 5);
    strcpy(str_type, "Bulb");
  }
  else if(node->type == WINDOW){
    str_type = (char *) malloc(sizeof(char) * 7);
    strcpy(str_type, "Window");
  }
  else if(node->type == FRIDGE){
    str_type = (char *) malloc(sizeof(char) * 7);
    strcpy(str_type, "Fridge");
  }
  else{
    str_type = (char *) malloc(sizeof(char) * 3);
    strcpy(str_type, "ND");
  }

  printf("%s\n%s=> %s : %s : %d\n",str, str, node->nome, str_type, node->id);

  char * str_temp = malloc(sizeof(char) * (strlen(str)+2));
  strcpy(str_temp,str);
  str_temp[strlen(str)] = '|';
  str_temp[strlen(str)+1] = '\0';
  int i;
  for(i = 0; i < node->nfigli && node->figli[i] != NULL; i++){
    tree_print_branch(node->figli[i], str_temp);
  }
  printf("%s/\n",str);
}

void tree_print(tree_device * tree){
  tree_print_branch(tree->root, "");
}

void get_all_info(int_list *queue, int my_queue){
  printf("start\n" );
  msgbuf richiesta, risposta;
  int dim, cod, i = queue->n;
  crea_messaggio_base(&richiesta, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_INF);
  richiesta.msg_type = NUOVA_OPERAZIONE;
  invia_broadcast(&richiesta, queue);
  char ** msg;
  tree_device * tree = create_tree_device();
  tree_insert_device(tree, -1, CONTROLLER, 0, queue->n, CONTROLLER, "Controller");
  for(; i > 0; i--){
    leggi(my_queue, &risposta, 2, 2);
    dim = protocoll_parser(risposta.msg_text, &msg);
    cod = codice_messaggio(msg);
    printf("read\n" );
    if(cod == MSG_INF_HUB || cod == MSG_INF_TIMER){
      i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
      tree_insert_device(tree, atoi(msg[MSG_INF_IDPADRE]), atoi(msg[MSG_ID_MITTENTE]), 0, atoi(msg[MSG_INF_CONTROLDV_NFIGLI]), atoi(msg[MSG_TYPE_MITTENTE]), msg[MSG_INF_NOME]);
    }
    else if(cod == MSG_INF_BULB || cod == MSG_INF_WINDOW || cod == MSG_INF_FRIDGE){
      tree_insert_device(tree, atoi(msg[MSG_INF_IDPADRE]), atoi(msg[MSG_ID_MITTENTE]), 0, 0, atoi(msg[MSG_TYPE_MITTENTE]), msg[MSG_INF_NOME]);
    }
  }
  tree_print(tree);
}


//ritorna TRUE se ci sono dispositivi con valori diversi FALSE altrimenti; msg_example contiene il primo messaggio info utilizzato per il controllo (se il risultato è TRUE significa che ogni dispositivo ha lo stesso stato dell'example)

void del(char ** cmd, int n, int_list * figli, int queue ,int deposito){

    msgbuf messaggio;
    int flag = FALSE;
    int id_dispositivo = -1;
    if(n == 2){
      flag = TRUE;
      id_dispositivo = atoi(cmd[1]);
      crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_dispositivo);
      concat_int(&messaggio, MSG_RIMUOVIFIGLIO_SPEC_DEL);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_dispositivo);
      concat_int(&messaggio, MSG_RIMUOVIFIGLIO_SPEC_DEL);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      invia_broadcast(&messaggio, figli);

      msgbuf risposta_figli;
      char ** msg_risp_f;
      int dim_msg, temp_int;
      int flag2 = TRUE;

      int i;

      for(i = figli->n + 1; i > 0 && flag2; i--){
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
          flag2 = FALSE;
        }
      }

    }
    if(flag == FALSE){
      printf("\nErrore campi: del \"<id dispositivo>\"\n <id dispositivo>: se non si conosce l'id del dispositivo usare il comando info\n");
    }
}

void lik(char ** cmd, int n, int_list * figli, int queue, int deposito){
  msgbuf messaggio;
  int flag = FALSE;
  //printf("\n1\n");
  if(n == 4){
    rimuovi_maiuscole(cmd[2]);
    if(strcmp("to", cmd[2]) == 0){
      flag = TRUE;

      int id_d1 = atoi(cmd[1]);
      int id_d2 = atoi(cmd[3]);
      crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_d1);
      concat_int(&messaggio, MSG_RIMUOVIFIGLIO_SPEC_DEP);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);
      //printf("\n3.6\n");

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_d1);
      concat_int(&messaggio, MSG_RIMUOVIFIGLIO_SPEC_DEP);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      invia_broadcast(&messaggio, figli);

      msgbuf risposta_figli;
      char ** msg_risp_f;
      int dim_msg, temp_int;
      int flag2 = TRUE;

      int i;
      //printf("\n4\n");
      for(i = figli->n + 1; i > 0 && flag2; i--){
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
          flag2 = FALSE;
        }
      }

      int flag3 = FALSE;
      //printf("\n6\n");
      if(id_d2 == CONTROLLER || id_d2 == DEFAULT){
        sleep(1);
        printf("CC 1\n" );

        crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_DEPOSITO_DEL);
        concat_int(&messaggio, id_d1);
        messaggio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

        msgbuf richiesta_figlio;

        int q_nf;

        crea_queue(id_d1, &q_nf);
        insert_int(q_nf, 0, figli);
        crea_messaggio_base(&richiesta_figlio, DEFAULT, CONTROLLER, DEFAULT, id_d1, MSG_SALVA_SPEGNI);
        richiesta_figlio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(q_nf, &richiesta_figlio, sizeof(richiesta_figlio.msg_text), 0);
        recupero_in_cascata(q_nf);
        flag3 = TRUE;

      }
      else{
        crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_d2, CONTROLLER, MSG_AGGIUNGI);
        concat_int(&messaggio, id_d1);
        messaggio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

        crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_d2, CONTROLLER, MSG_AGGIUNGI);
        concat_int(&messaggio, id_d1);
        messaggio.msg_type = NUOVA_OPERAZIONE;
        invia_broadcast(&messaggio, figli);

        flag2 = TRUE;

        for(i = figli->n + 1; i > 0 && flag2; i--){
          if(leggi(queue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_ACKP){
              flag3 = TRUE;
            }
          }
          else{
            flag2 = FALSE;
          }
        }
      }
      if(flag3 == TRUE){
        printf("\nOperazione eseguita con successo\n");
      }
    }
    //printf("\nfine\n");
  }

  if(flag == FALSE){
    //printf("\nErrore campi: link <id dispositivo> to <id dispositivo di controllo>\n  <id dispositivo> : \"hub\", \"timer\", \"bulb\", \"window\", \"fridge\"\n <id dispositivo di controllo> : \"controller\", \"hub\", \"timer\"\n");
  }
}

void list(char ** cmd, int n, int_list * figli, int queue, int deposito){
    if(n == 1){
      msgbuf newmsg, risposta_figli;
      char ** msg_risp_f;
      crea_messaggio_base(&newmsg, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_INF);
      newmsg.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &newmsg, sizeof(newmsg.msg_text), 0);

      int flag = TRUE;
      int i, dim_msg, cod;
      char * str_type;
      for(i = 1; i > 0 && flag; i--){
        if(leggi(queue, &risposta_figli, 2, 2)){
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          cod = codice_messaggio(msg_risp_f);
          if(cod == MSG_INF_HUB || cod == MSG_INF_TIMER || cod == MSG_INF_DEPOSITO){
            i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
          }

          if(cod == MSG_INF_HUB){
            str_type = (char *) malloc(sizeof(char) * 4);
            strcpy(str_type, "Hub");
          }
          else if(cod == MSG_INF_TIMER){
            str_type = (char *) malloc(sizeof(char) * 6);
            strcpy(str_type, "Timer");
          }
          else if(cod == MSG_INF_BULB){
            stampa_info_bulb(&risposta_figli);
            str_type = (char *) malloc(sizeof(char) * 5);
            strcpy(str_type, "Bulb");
          }
          else if(cod == MSG_INF_WINDOW){
            stampa_info_window(&risposta_figli);
            str_type = (char *) malloc(sizeof(char) * 7);
            strcpy(str_type, "Window");
          }
          else if(cod == MSG_INF_FRIDGE){
            str_type = (char *) malloc(sizeof(char) * 7);
            strcpy(str_type, "Fridge");
          }
          else{
            str_type = (char *) malloc(sizeof(char) * 3);
            strcpy(str_type, "ND");
          }

          if(cod != MSG_INF_DEPOSITO){
            printf("=> %s : %s : %s\n", msg_risp_f[MSG_INF_NOME], str_type ,msg_risp_f[MSG_ID_MITTENTE]);
          }
        }
        else{
          flag = FALSE;
          printf(">>timeout\n");
        }
    }
    get_all_info(figli, queue);
  }
  else{
    printf("\nErrore campi: il comando list non accetta parametri");
  }
}


void add(char ** cmd, int n, int q_dep, int new_id){

  msgbuf messaggio;
  int flag = FALSE;
  if(n == 2){
    flag = TRUE;
    rimuovi_maiuscole(cmd[1]);
    crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_ADD_DEVICE);
    if(strcmp(cmd[1], "hub") == 0){
      concat_int(&messaggio, HUB);
    }
    else if(strcmp(cmd[1], "timer") == 0){
      concat_int(&messaggio, TIMER);
    }
    else if(strcmp(cmd[1], "bulb") == 0){
      concat_int(&messaggio, BULB);
    }
    else if(strcmp(cmd[1], "window") == 0){
      concat_int(&messaggio, WINDOW);
    }
    else if(strcmp(cmd[1], "fridge") == 0){
      concat_int(&messaggio, FRIDGE);
    }
    else{
      flag = FALSE;
    }
    concat_int(&messaggio, new_id);
    if(flag){
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(q_dep, &messaggio, sizeof(messaggio.msg_text), 0);
    }
  }
  if(flag == FALSE){
    printf("\nErrore campi: add <type dispositivo>\n <type dispositivo>: \"hub\", \"timer\", \"bulb\", \"window\", \"fridge\"\n");
  }
}

void swtch(char ** cmd, int n, int queue, int deposito){

}

void info(char ** cmd, int n, int queue, int deposito, int_list * figli){
  msgbuf newmsg, risposta_figli;
  char ** msg_risp_f;
  char ** msg;
  crea_messaggio_base(&newmsg, DEFAULT, CONTROLLER, atoi(cmd[1]), CONTROLLER, MSG_INF);
  newmsg.msg_type = NUOVA_OPERAZIONE;
  msgsnd(deposito, &newmsg, sizeof(newmsg.msg_text), 0);
  if(atoi(cmd[1]) == 1) {
    crea_messaggio_base(&newmsg, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_INF);
  }
  invia_broadcast(&newmsg, figli);
  int flag = TRUE;
  int i, dim_msg, cod;
  printf("controller info:\n");
  for(i = figli->n + 1; i > 0 && flag; i--){
    if(leggi(queue, &risposta_figli, 2, 2)){
      dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
      cod = codice_messaggio(msg_risp_f);
      if(cod == MSG_INF_HUB || cod == MSG_INF_TIMER || cod == MSG_INF_DEPOSITO){
        i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
        printf("stampa : aggiunto %d\n", atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]) );
      }
      printf("----\n%s\n----\n", risposta_figli.msg_text );
      if(cod == MSG_INF_HUB){

      }
      else if(cod == MSG_INF_TIMER){

      }
      else if(cod == MSG_INF_BULB){
        stampa_info_bulb(&risposta_figli);
      }
      else if(cod == MSG_INF_WINDOW){
        stampa_info_window(&risposta_figli);
      }
      else if(cod == MSG_INF_FRIDGE){

      }
    }
    else{
      flag = FALSE;
      printf(">>timeout\n");
    }
  }
}


void controller(int myid, int id_deposito){
    printf("controller - %d - pid = %d\n", myid, getpid());
    ssize_t dim_str = 110;
    char * str = (char *) malloc(sizeof(char) * dim_str);
    char ** cmd;
    int_list * figli = (int_list *) create_int_list();
    int n;
    int next_id = 385;
    int my_queue, queue_deposito;
    crea_queue(myid, &my_queue);
    crea_queue(id_deposito, &queue_deposito);
    printf("controller - %d - pid = %d - %d\n", myid,getpid(), my_queue);

    int flag = TRUE;
    printf("\n>");
    while (flag && getline(&str, &dim_str, stdin) >= 0) {
        svuota_msg_queue(my_queue, 2);
        n = str_split(str, &cmd);
        if(n > 0){
          if(strcmp(cmd[0], "list") == 0){
            list(cmd, n, figli, my_queue, queue_deposito);;
          }
          else if(strcmp(cmd[0], "add") == 0){
            int flag = TRUE;
            int q;
            key_t key;

            while(flag){
              next_id++;
              if((key = ftok("/tmp/domotica.txt", next_id)) != -1){ // crea la chiave
                if( (q = msgget(key, IPC_CREAT | IPC_EXCL)) != -1){ // crea il file se non esiste
                    flag = FALSE;
                }
              }
            }
            add(cmd, n, queue_deposito, next_id);
          }
          else if(strcmp(cmd[0], "del") == 0){
            del(cmd, n, figli, my_queue, queue_deposito);
          }
          else if(strcmp(cmd[0], "link") == 0){
            lik(cmd, n, figli, my_queue, queue_deposito);
          }
          else if(strcmp(cmd[0], "switch") == 0){
            swtch(cmd, n, my_queue, queue_deposito);
          }
          else if(strcmp(cmd[0], "info") == 0){
            info(cmd, n, my_queue, queue_deposito, figli);
          }
          else if(strcmp(cmd[0], "quit") == 0){
              flag = FALSE;
          }
          else{
              printf("comando non valido \n");
          }
        }
        sleep(1);
        printf("\n>");
    }
}

int main() {
  signal(SIGCHLD, SIG_IGN);
  int id_controller = CONTROLLER;
  int id_deposito = DEPOSITO;
  printf("%d\n",  MSG_INF);
  unlink("/tmp/domotica.txt");
  perror("unlink: ");
  creat("/tmp/domotica.txt", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  perror("apertura: ");
  if (fork () > 0) { //codice centralina
    controller(id_controller, id_deposito);
  }
  else{ // codice magazzino
    deposito(id_deposito, id_controller);
  }
  return 0;
}
