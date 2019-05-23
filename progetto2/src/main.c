#include "main.h"


int equal_bulb(msgbuf msg_example, msgbuf messaggio){ //da fare
  return TRUE;
}

int equal_fridge(msgbuf msg_example, msgbuf messaggio){ // da fare
  return TRUE;
}

int equal_window(msgbuf msg_example, msgbuf messaggio){ // da fare
  return TRUE;
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
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_dispositivo);
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

      crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_DEPOSITO_DEL);
      concat_int(&messaggio, id_dispositivo);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

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
    rimuovi_maiuscole(cmd[1]);
    rimuovi_maiuscole(cmd[2]);
    rimuovi_maiuscole(cmd[3]);
    if(strcmp("to", cmd[2]) == 0){
      flag = TRUE;

      int id_d1 = atoi(cmd[1]);
      int id_d2 = atoi(cmd[3]);
      crea_messaggio_base(&messaggio, DEPOSITO, CONTROLLER, DEPOSITO, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_d1);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);
      //printf("\n3.6\n");

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_d1);
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
      //printf("\n6\n");
      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_d2, CONTROLLER, MSG_AGGIUNGI);
      concat_int(&messaggio, id_d1);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_d2, CONTROLLER, MSG_AGGIUNGI);
      concat_int(&messaggio, id_d1);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      invia_broadcast(&messaggio, figli);

      flag2 = TRUE;
      int flag3 = FALSE;

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

void list(char ** cmd, int n, int queue, int deposito){

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
  invia_broadcast(&newmsg, figli);
  int flag = TRUE;
  int i;
  printf("info:\n");
  for(i = figli->n + 1; i > 0 && flag; i--){
    if(leggi(queue, &risposta_figli, 2, 2)){
      int dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
      if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER || codice_messaggio(msg_risp_f) == MSG_INF_DEPOSITO){
        i += atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]);
        printf("stampa : aggiunto %d\n", atoi(msg_risp_f[MSG_INF_CONTROLDV_NFIGLI]) );
      }
      printf("info:\n---------------------------------- \n%s\n---------------------------------- \n", risposta_figli.msg_text);
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

    int flag = TRUE;
    printf("\n>");
    while (flag && getline(&str, &dim_str, stdin) >= 0) {
        n = str_split(str, &cmd);
        if(n > 0){
          if(strcmp(cmd[0], "list") == 0){

          }
          else if(strcmp(cmd[0], "add") == 0){
            add(cmd, n, queue_deposito, next_id);
            next_id++;
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
