#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>


//Size funzioni
#define MSG_SIZE 100
//valori di verità
#define TRUE 1
#define FALSE 0

//dispositivi
#define DEFAULT 0
#define CONTROLLER 1
#define DEPOSITO 2
#define HUB 3
#define TIMER 4
#define BULB 5
#define WINDOW 6
#define FRIDGE 7

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

#define NUOVA_OPERAZIONE 4

//messaggio base
#define MSG_TYPE_DESTINATARIO 0
#define MSG_TYPE_MITTENTE 1
#define MSG_ID_DESTINATARIO 2
#define MSG_ID_MITTENTE 3
#define MSG_OP 4

#define MSG_ACKP
#define MSG_ACKN

//messaggio info
#define MSG_INF 1 //codice messaggio
#define MSG_INF_IDPADRE 5
#define MSG_INF_NOME 6
#define MSG_INF_CONTROLDV_NFIGLI 7

#define MSG_OVERRIDE

#define MSG_AGGIUNGI
#define MSG_AGGIUNGI_IDF

#define MSG_GET_TERMINAL_TYPE
#define MSG_MYTYPE

#define MSG_INF_DEPOSITO
#define MSG_INF_HUB
#define MSG_INF_TIMER
#define MSG_INF_BULB
#define MSG_INF_WINDOW
#define MSG_INF_FRIDGE

#define MSG_DEPOSITO_DEL
#define MSG_DEPOSITO_DEL_ID

//Struct
typedef struct pair_int{
    int first;
    int second;
} pair_int;

typedef struct tree_device_node{
    int id;
    char * nome;
    int type;
    int pid;
    int nfigli;
    struct tree_device_node ** figli;
} tree_device_node;


typedef struct tree_device{
    struct tree_device_node * root;
} tree_device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct int_list{
    int_node * head;
    int n;
} int_list;

typedef struct msgbuf{
    long int msg_type;
    char msg_text[MSG_SIZE];
} msgbuf;

tree_device * create_tree_device(){
    tree_device * rt = (tree_device *) malloc(sizeof(tree_device));
    rt->root = NULL;
    return rt;
}

tree_device_node * tree_get_node(tree_device_node * tree, int id){
  tree_device_node * rt = NULL;
  if(tree != NULL){
    if(tree->id == id){
      rt = tree;
    }
    else{
      int i;
      for(i = 0; i < tree->nfigli && rt == NULL && tree->figli[i] != NULL; i++){
        rt = tree_get_node(tree->figli[i], id);
      }
    }
  }
  return rt;
}

int tree_insert_device(tree_device * tree, int id_padre, int id, int pid, int n_figli, int type, char * name){
    int rt = TRUE;
    tree_device_node * node = (tree_device_node *) malloc(sizeof(tree_device_node));
    node->id = id;
    node->type = type;
    node->pid = pid;
    node->nome = (char *) malloc(sizeof(char) * (strlen(name)+1));
    strcpy(node->nome, name);
    node->nfigli = n_figli;
    node->figli = (tree_device_node ** ) malloc(sizeof(tree_device_node *) * n_figli);
    int i;
    for(i = 0; i < n_figli; i++){
      node->figli[i] = NULL;
    }
    if(id_padre <  0){ //inserisci in testa
      tree->root = node;
    }
    else{
      tree_device_node * padre = tree_get_node(tree->root, id);
      for(i = 0; i < padre->nfigli && padre->figli[i] == NULL; i++);
      if(i < padre->nfigli){
        padre->figli[i] = node;
      }
    }
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

  printf("%s\n%s-> %s : %s : %d\n",str, str, node->nome, str_type, node->pid);

  char * str_temp = malloc(sizeof(char) * (strlen(str)+2));
  strcpy(str_temp,str);
  str_temp[strlen(str)] = "|";
  str_temp[strlen(str)+1] = "\0";
  int i;
  for(i = 0; i < node->nfigli && node->figli[i] != NULL; i++){
    tree_print_branch(node->figli[i], str_temp);
  }
  printf("%s/\n");
}

void tree_print(tree_device * tree){
  tree_print_branch(tree->root, "");
}

//ritorna TRUE o FAlSE in base se è riuscito o meno a leggere l'integer
int get_int(int n, int * val, int_list * list){
    int rt = TRUE;
    if(list->n <= n || n < 0){
        rt = FALSE;
    }
    else{
        int_node * temp = list->head;
        int i;
        for(i = 0; i < n; i++){
            temp = temp->next;
        }
        *val = temp->val;
    }
    return rt;
}

int_list * create_int_list(){
    int_list * rt = (int_list *) malloc(sizeof(int_list));
    rt->head = NULL;
    rt->n = 0;
    return rt;
}

int insert_int(int val, int n, int_list * list){
    int rt;
    if(list->n < n || n < 0){
        rt = FALSE;
    }
    else{
        int_node * new_node = (int_node *) malloc(sizeof(int_node));
        new_node->val = val;
        if(n == 0){
            new_node->next = list->head;
            list->head = new_node;
        }
        else{
            int_node * temp = list->head;
            int i;
            for(i = 0; i < n-1; i++){
                temp = temp->next;
            }
            new_node->next = temp->next;
            temp->next = new_node;
        }
        list->n++;
        rt = TRUE;
    }
    return rt;
}

int rm_int(int n, int_list * list){
    int rt;
    if(list->n <= n || n < 0){
        rt = FALSE;
    }
    else{
        int_node * temp = list->head;
        if(n == 0){
            list->head = temp->next;
            free(temp);
        }else{
            int i;
            for(i = 0; i < n-1; i++){
                temp = temp->next;
            }
            int_node * temp2 = temp->next;
            temp->next = temp2->next;
            free(temp2);
        }
        rt = TRUE;
        list->n--;
    }
    return rt;
}

int str_split(char * str, char *** rt){
    int i = 0, j = 0, t = 0, c;
    int flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == ' ' || str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != ' ' && str[i-1] != '\n'){
            j++;
        }
        if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    (*rt) = (char **) malloc(sizeof(char *) * j);
    j = 0;
    flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == ' ' || str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != ' ' && str[i-1] != '\n'){
            (*rt)[j] = (char *) malloc(sizeof(char *) * (i-t+1));
            for (c = 0; t+c < i; c++) {
                (*rt)[j][c] = str[t+c];
            }
            (*rt)[j][c] = '\0';
            j++;
        }
        if(str[i] == ' '){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}

int protocoll_parser(char * str, char *** rt){
    int i = 0, j = 0, t = 0, c;
    int flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != '\n'){
            j++;
        }
        if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    (*rt) = (char **) malloc(sizeof(char *) * j);
    j = 0;
    flag = TRUE;
    for(i = 0; flag; i++){
        if((str[i] == '\n' || str[i] == '\0') && i > 0 && str[i-1] != '\n'){
            (*rt)[j] = (char *) malloc(sizeof(char *) * (i-t+1));
            for (c = 0; t+c < i; c++) {
                (*rt)[j][c] = str[t+c];
            }
            (*rt)[j][c] = '\0';
            j++;
        }
        if(str[i] == '\n'){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}

void crea_queue(int id, int * queue){
  key_t key;
  if((key = ftok("/tmp/", id)) == -1){ // crea la chiave
      printf("errore 1\n");
      exit(1);
  }
  if(((*queue) = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste
      printf("errore 2\n");
      exit(1);
  }
}

void bulb_info_r(char ** str){
    if(str[6][0] == '0'){
        printf("Stato: OFF\n" );
    }
    else{
      printf("Stato: ON\n");
    }
    if(str[7][0] == '0'){
        printf("Interruttore: OFF\n" );
    }
    else{
      printf("Interruttore: ON\n");
    }
    printf("Timer: %lld\n", atoi(str[9]));
    printf("Nome: %s\n", str[11]);

}

int itoa(int n, char **str){
  int i, temp, segno = 0;
  if(n == 0){
    i = 1;
    (*str) = (char *) malloc(sizeof(char) * (i+1));
    (*str)[0] = '0';
    (*str)[1] = '\0';
  }
  else{
    if(n < 0){
      segno = 1;
      n *= -1;
    }

    temp = n;
    for(i = segno; temp > 0; i++){
      temp =  temp/10;
    }
    (*str) = (char *) malloc(sizeof(char) * (i+1));

    temp = n;
    (*str)[i] = '\0';

    i--;
    for(; temp > 0; i--){
      (*str)[i] = temp%10 + '0';
      temp = temp/10;
    }

    if(segno){
      (*str)[i] = '-';
    }
  }
  return i+1;
}

int controlla_validita(char ** str, int id){
 int rt = FALSE;
 if(atoi(str[0]) == CONTROLLER || (atoi(str[0]) == DEFAULT && str[5][0] == '0')){
   printf("k1 %lld\n", atoi(str[3]));
   int i;
   for ( i = 0; str[3][i] != '\0'; i++) {
     printf("> %d - %c\n", str[3][i], str[3][i]);
   };
   if(atoi(str[3]) == id || atoi(str[3]) == 0){
     rt = TRUE;
     printf("k2\n" );
   }
 }
 return rt;
}
/*
void risposta(){
  int queue;
  msgbuf messaggio;
  crea_queue(100,&queue);
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) == -1) {
     printf("Errore\n" );
  }
  else{
    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    int i;
    printf("------\n");
    for(i = 0; i < n; i++){
      printf("> %d\n", strlen(msg[i]));
    }
    printf("------\n");
    if(controlla_validita(msg,100)){
      if(msg[5][0] == '0'){
        char * str = malloc(sizeof(char) * 7);
        str[0] = msg[0][0];
        str[1] = '\0';
        strcat(str,msg[5]);
        str[6] = '\0';
        int codice = atoi(str);
        printf("=> %d\n", codice);
        switch (codice) {
          case 1001:
            bulb_info_r(msg);
            break;

          default: printf("Errore 3\n" );
            break;
        }
      }
      else{
        printf("Errore 2\n" );
      }
    }
    else{
      printf("Errore 1\n" );
    }
  }
}*/

int codice_messaggio(char ** msg){
  int rt = atoi(msg[MSG_OP]);
  if((rt) >= 10000){
    rt += atoi(msg[MSG_TYPE_DESTINATARIO]) * 100000;
  }
  return rt;
}

void concat_int(msgbuf * messaggio, int n){
  char * str
  itoa(n, &str);
  strcat(messaggio.msg_text, str);
  strcat(messaggio.msg_text, "\n");
}

void concat_string(msgbuf * messaggio, char * str){
  strcat(messaggio.msg_text, str);
  strcat(messaggio.msg_text, "\n");
}

void crea_messaggio_base(msgbuf * messaggio, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op){

  messaggio->msg_text[0] = '\0';
  concat_int(messaggio, type_dest);
  concat_int(messaggio, type_mit);
  concat_int(messaggio, id_dest);
  concat_int(messaggio, id_mit);
  concat_int(messaggio, codice_op % 100000); // rimuovo il valore "tipo dispositivo" nel caso presente NB: il tipo mittente è passato come parametro separato
}

void get_all_info(int_list *queue){
  msgbuf messaggio;
  int q, i;
  messaggio.msg_type = 1;
  char * msg;
  crea_messaggio_base(&msg, 0, CONTROLLER, 0, CONTROLLER, MSG_INF);
  strcpy(messaggio.msg_text, msg);
  for(i = 0; get_int(i, q, queue); i++){
    msgsnd(q, &messaggio, sizeof(messaggio.msg_text), 0);
  }
  char ** msg;
  tree_device * tree = create_tree_device();
  tree_insert_device(tree, -1, 1, 1, "Controller");
  for(; i > 0; i--){
    msgrcv(queue, &messaggio, sizeof(messaggio.msg_text), 1, 0);
    int dim = protocoll_parser(messaggio.msg_text, &msg);
    if(codice_messaggio(msg) == MSG_INF){
      tree_insert_device(tree, atoi(msg[MSG_INF_IDPADRE]), atoi(msg[MSG_ID_MITTENTE]), atoi(msg[MSG_TYPE_MITTENTE]), msg[MSG_INF_NOME]);
      if(atoi(msg[MSG_TYPE_MITTENTE]) == HUB || atoi(msg[MSG_TYPE_MITTENTE]) == TIMER){
        i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
      }
    }
  }
  tree_print(tree);
}

void ricomponi_messaggio(char ** cmd, int n, msgbuf * messaggio){ // ricomponi_messaggio(cmd, &str);

  messaggio->msg_text[0] = '\0';
  for(i = 0; i < n; i++){
    concat_string(messaggio, cmd[i]);
  }
}

void invia_broadcast(msgbuf * messaggio, int_list * queue){
  int_node * next = queue->head;
  int i;
  for(i = 0; i < queue->n && next != NULL; i++){
    msgsnd(next->val, messaggio, sizeof(messaggio->msg_text), 0);
  }
}

void info(char ** cmd, int n, int_list * figli_controller, int my_queue){
  int flag = FALSE;
  if(n == 2 ){
    int id;
    if(strcmp(cmd[1], "all")){
      get_all_info(figli_controller);
    }
    else if((id = atoi(cmd[1])) > 0){
      char * msg;
      crea_messaggio_base(&msg, 0, CONTROLLER, id, CONTROLLER, 1);
      invia_broadcast(msg, figli_controller);

      msgbuf messaggio;
      char ** msg;
      msgrcv(queue, &messaggio, sizeof(messaggio.msg_text), 1, 0);
      dim = protocoll_parser(messaggio.msg_text, &msg);
      if(msg[1] == HUB){
        //...
      }
      else if(msg[1] == TIMER){
        //... da finire
      }
    }
    else{
      flag = TRUE;
    }
  }
  else{
    flag = TRUE;
  }
  if(flag){
    printf("Errore sintattico digitare 'help' per l'elenco dei comandi disponibili\n");
  }
}

void recupero_in_cascata(int myqueue){
  char ** msg;
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
     printf("errore lettura ripristino\n");
  }
  else{
    protocoll_parser(messaggio.msg_text, &msg);
    int type = atoi(msg[MSG_RECUPERO_TYPE]);
    int myid = atoi(msg[MSG_RECUPERO_ID]);
    msgsnd(queue, &messaggio ,sizeof(messaggio.msg_text), 0);
    if(type == HUB){
      if(fork() == 0){
        hub(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == TIMER){
      if(fork() == 0){
        timer(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == BULB){
      if(fork() == 0){
        bulb(myid, TRUE);
        exit(0);
      }
    }
    else if(type == WINDOW){
      if(fork() == 0){
        window(myid, TRUE);
        exit(0);
      }
    }
    else if(type == FRIDGE){
      if(fork(myid, TRUE) == 0){
        fridge(myid, TRUE, "");
        exit(0);
      }
    }
  }
}


int leggi(int queue, msgbuf * messaggio, int p, float t){
  int rt = TRUE;
  int flag = TRUE;
  int i;
  for(i = 0; i < (20 * t) && (errno == ENOMSG || flag); i++){
    usleep(50000); //microsecondi
    errno = 0;
    msgrcv(queue, messaggio ,sizeof(messaggio->msg_text), p, IPC_NOWAIT);
    flag = FALSE;
  }
  if(i >= (20 * t)){
    rt = FALSE;
  }
  return rt;
}

//ritorna TRUE se ci sono dispositivi con valori diversi FALSE altrimenti; msg_example contiene il primo messaggio info utilizzato per il controllo (se il risultato è TRUE significa che ogni dispositivo ha lo stesso stato dell'example)
int override(int_list * queues, int type, int myid, int myqueue, msgbuf * msg_example){
  msgbuf messaggio;
  int rt = FALSE;
  messaggio.msg_type = NUOVA_OPERAZIONE;
  crea_messaggio_base(&messaggio, DEFAULT, HUB, DEFAULT, myid, MSG_OVERRIDE);
  broadcast(&messaggio, queues);
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

  int queue;
  msgbuf messaggio;

  int type_child;

  if(fork() == 0){// codice figlio da fare
    exit(0);
  }

  crea_queue(id, &queue);

  if(recupero){
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      char ** msg;
      int n = protocoll_parser(messaggio.msg_text, msg);
      char * str;
      strcpy(nome, msg[MSG_RECUPERO_HUB_NOME]);
      for(int i = MSG_RECUPERO_HUB_INIZIOFIGLI; i < n; i++){
        insert_int(atoi(msg[i]), 0, figli);
        recupero_in_cascata(atoi(msg[i]));
      }
    }
  }

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0)) != -1) {

    msg_type = messaggio.msg_type;
    protocoll_parser(messaggio.msg_text, msg);
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;

    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      if(id_dest == DEFAULT || id_dest == id){
        //creo la risposta
        msgbuf risposta; //messaggio base + id_padre + nome + num figli + bool override
        risposta.msg_type = 2;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_RISP_HUB);
        concat_string(&risposta, msg[MSG_ID_MITTENTE]);  //si suppone che solo il padre possa fare una richiesta di questo tipo
        concat_string(&risposta, nome);
        concat_int(&risposta, type_child);
        concat_int(&risposta, figli->n);

        //controllo override

        msgbuf msg_example;
        int rt = override(figli, type_child);
        concat_int(&risposta, override(figli, type_child,id,queue,&msg_example));


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
        int flag = TRUE;
        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
              i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
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
        }/*
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        for(; i > 0; i--){
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }*/
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
                i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
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
          itoa(id, example[MSG_ID_MITTENTE]);
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
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
        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_ACKP){
              id_figlio = atoi(msg_risp_f[MSG_ID_MITTENTE]);
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
        create_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf richiesta_figli, risposta_figli;
        char ** msg_risp_f;

        crea_messaggio_base(&richiesta_figlio, DEFAULT, HUB, DEFAULT, id, MSG_GET_TERMINAL_TYPE);
        richiesta_figlio.msg_type = NUOVA_OPERAZIONE;
        msgsnd(q_nf, &richiesta_figlio, sizeof(richiesta_figlio.msg_text), 0);

        int flag = TRUE;
        int type_new_c = -1;
        while(flag){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
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
        }
        else{
          crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        }
        concat_int(messaggio, type_child);
        risposta.msg_type = 2;
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
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
    if((codice_messaggio(msg) == MSG_SALVA_SPEGNI || codice_messaggio(msg) == MSG_SPEGNI || flag_rimuovi){
      mesg_non_supp = FALSE;
      if(id_dest == DEFAULT || id_dest == id || flag_rimuovi){
        msgbuf msg_salva;

        //creo il messaggio di ripristino
        if(MSG_SALVA_SPEGNI || flag_rimuovi){
          msg_salva.msg_type = 10;
          crea_messaggio_base(&msg_salva, HUB, HUB, id, id, MSG_RECUPERO_HUB);
          concat_string(msg_salva, nome);
          int i, next;
          for(i = 0; i < figli->n && get_int(i, &next, figli); i++){
            concat_int(msg_salva, next);
          }
          msgsnd(queue, &msg_salva, sizeof(msg_salva.msg_text), 0);
        }

        //invio il messaggio di chiusura ai figli
        msgbuf richiesta_figli;
        crea_messaggio_base(&richiesta_figli, DEFAULT, HUB, DEFAULT, id, MSG_SALVA_SPEGNI);
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
      msgbuf risposta;
      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
      risposta.msg_type = 2;
      msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
    }
  }
}

void del(char ** cmd, int n, int_list figli_controller, int myqueue ,int deposito){

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
      for(i = figli->n + 1; i > 0 && flag2; i--){
        if(leggi(myqueue, &risposta_figli, 2, 2)){
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) == MSG_ACKP){
            int id_figlio = atoi(msg_risp_f[MSG_ID_MITTENTE]);
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

void link(char ** cmd, int n, int_list figli_controller, int queue, int deposito){
  msgbuf messaggio;
  int flag = FALSE;
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

      crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, DEFAULT, CONTROLLER, MSG_RIMUOVIFIGLIO);
      concat_int(&messaggio, id_d1);
      messaggio.msg_type = NUOVA_OPERAZIONE;
      invia_broadcast(&messaggio, figli);

      msgbuf risposta_figli;
      char ** msg_risp_f;
      int dim_msg, temp_int;
      int flag2 = TRUE;
      for(i = figli->n + 1; i > 0 && flag2; i--){
        if(leggi(myqueue, &risposta_figli, 2, 2)){
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) == MSG_ACKP){
            int id_figlio = atoi(msg_risp_f[MSG_ID_MITTENTE]);
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
  }

  crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_2, CONTROLLER, MSG_AGGIUNGI);
  concat_int(&messaggio, id_1);
  messaggio.msg_type = NUOVA_OPERAZIONE;
  msgsnd(deposito, &messaggio, sizeof(messaggio.msg_text), 0);

  crea_messaggio_base(&messaggio, DEFAULT, CONTROLLER, id_2, CONTROLLER, MSG_AGGIUNGI);
  concat_int(&messaggio, id_d1);
  messaggio.msg_type = NUOVA_OPERAZIONE;
  invia_broadcast(&messaggio, figli);

  flag2 = TRUE;
  int flag3 = FALSE;
  for(i = figli->n + 1; i > 0 && flag2; i--){
    if(leggi(myqueue, &risposta_figli, 2, 2)){
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
    printf("\nOperazione eseguita con successo\n", );
  }

  if(flag == FALSE){
    printf("\nErrore campi: link <id dispositivo> to <id dispositivo di controllo>\n  <id dispositivo> : \"hub\", \"timer\", \"bulb\", \"window\", \"fridge\"\n <id dispositivo di controllo> : \"controller\", \"hub\", \"timer\"\n");
  }
}

void list(char ** cmd, int n, int queue, int deposito){

}

void rimuovi_maiuscole(char * str){
  if(str != NULL){
    int i, k = 'a' -'A';
    for(i = 0; i <= strlen(str); i++){
      if(str[i] >= 'A' && str[i] <= 'Z'){
        str[i] += k;
      }
    }
  }
}

void add(char ** cmd, int n, int q_dep){

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

void controller(int myid, int id_deposito){
    char * str = (char *) malloc(sizeof(char) * 110);
    char ** cmd;
    int_list figli;
    int n;
    int my_queue, queue_deposito;
    crea_queue(myid, &my_queue);
    crea_queue(id_deposito, &queue_deposito);

    int flag = TRUE;
    while (flag) {
        printf("\n>");
        scanf("%100s", str);
        n = str_split(str, cmd);
        if(strcmp(cmd[0], "list") == 0){

        }
        else if(strcmp(cmd[0], "add") == 0){
          add(cmd, n, queue_deposito);
        }
        else if(strcmp(cmd[0], "del") == 0){
          del(cmd, n, figli);
        }
        else if(strcmp(cmd[0], "link") == 0){
          add(link,n, figli);
        }
        else if(strcmp(cmd[0], "switch") == 0){
          swtch(cmd,n); //non si può usare switch
        }
        else if(strcmp(cmd[0], "info") == 0){
          info(cmd, n, my_queue);
        }
        else if(strcmp(cmd, "quit") == 0){
            flag = FALSE;
        }
        else{
            printf("comando non valido \n");
        }
    }
}

void deposito(int id, int id_controller){
  int_list * figli = create_int_list();

  int queue;
  msgbuf messaggio;

  crea_queue(id, &queue);

  //inizio loop
  char ** msg;
  int msg_type;
  int flag_rimuovi;
  int id_dest;
  int mesg_non_supp;
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), NUOVA_OPERAZIONE, 0)) != -1) {

    msg_type = messaggio.msg_type;
    protocoll_parser(messaggio.msg_text, msg);
    id_dest = atoi(msg[MSG_ID_DESTINATARIO]);
    mesg_non_supp = FALSE;
    flag_rimuovi = FALSE;

    if(codice_messaggio(msg) == MSG_INF){ //richiesta info su tutti
      if(id_dest == DEFAULT || id_dest == id){
        //creo la risposta
        msgbuf risposta; //messaggio base + id_padre + nome + num figli + bool override
        risposta.msg_type = 2;
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_INF_RISP_HUB);
        concat_int(&risposta, figli->n);

        //controllo override

        msgbuf msg_example;
        int rt = override(figli, type_child);
        concat_int(&risposta, override(figli, type_child,id,queue,&msg_example));


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
        for(i = figli->n; i > 0 && flag; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
              i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
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
        }/*
        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
        risposta.msg_type = 2;
        for(; i > 0; i--){
          msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
        }*/
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
            dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
            if(codice_messaggio(msg_risp_f) != MSG_ACKN){
              flag2 = FALSE;
              if(codice_messaggio(msg_risp_f) == MSG_INF_HUB || codice_messaggio(msg_risp_f) == MSG_INF_TIMER){
                i += atoi(msg[MSG_INF_CONTROLDV_NFIGLI]);
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
      for(i = figli->n; i > 0 && flag; i--){
        if(leggi(myqueue, &risposta_figli, 2, 2)){
          dim_msg = protocoll_parser(risposta_figli.msg_text, &msg_risp_f);
          if(codice_messaggio(msg_risp_f) == MSG_ACKP){
            id_figlio = atoi(msg_risp_f[MSG_ID_MITTENTE]);
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
        int q_nf;
        create_queue(atoi(msg[MSG_AGGIUNGI_IDF]), &q_nf);

        msgbuf risposta;
        char ** msg_risp_f;

        insert_int(q_nf, 0, figli);

        recupero_in_cascata(q_nf);

        crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), DEPOSITO, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKP);
        risposta.msg_type = 2;
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
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

        for(i = figli->n; i > 0 && flag1; i--){
          if(leggi(myqueue, &risposta_figli, 2, 2)){
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
      int q, temp_int, flag = TRUE;
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
    if((codice_messaggio(msg) == MSG_SALVA_SPEGNI || codice_messaggio(msg) == MSG_SPEGNI){
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
      msgbuf risposta;
      int msg_queue_mit;
      crea_queue(atoi(msg[MSG_ID_MITTENTE]), &msg_queue_mit);
      crea_messaggio_base(&risposta, atoi(msg[MSG_TYPE_MITTENTE]), HUB, atoi(msg[MSG_ID_MITTENTE]), id, MSG_ACKN);
      risposta.msg_type = 2;
      msgsnd(msg_queue_mit, &risposta, sizeof(risposta.msg_text), 0);
    }
  }
}



int main() {
  int id_controller = CONTROLLER;
  int id_deposito = DEPOSITO;
  if (fork () > 0) { //codice centralina
    controller(id_controller, id_deposito);
  }
  else{ // codice magazzino
    deposito(id_deposito, id_controller);
  }
  return 0;
}
