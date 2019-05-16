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
#define HUB 2
#define TIMER 3
#define BULB 4
#define WINDOW 5
#define FRIDGE 6

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

//messaggio base
#define MSG_TYPE_DESTINATARIO 0
#define MSG_TYPE_MITTENTE 1
#define MSG_ID_DESTINATARIO 2
#define MSG_ID_MITTENTE 3
#define MSG_OP 4

//messaggio info
#define MSG_INFO 1 //codice messaggio
#define MSG_INFO_IDPADRE 5
#define MSG_INFO_CONTROLDV_NFIGLI 6
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
  if(node->nome == CONTROLLER){
    str_type = (char *) malloc(sizeof(char) * 11);
    strcpy(str_type, "Controller");
  }
  else if(node->nome == HUB){
    str_type = (char *) malloc(sizeof(char) * 4);
    strcpy(str_type, "Hub");
  }
  else if(node->nome == TIMER){
    str_type = (char *) malloc(sizeof(char) * 6);
    strcpy(str_type, "Timer");
  }
  else if(node->nome == BULB){
    str_type = (char *) malloc(sizeof(char) * 5);
    strcpy(str_type, "Bulb");
  }
  else if(node->nome == WINDOW){
    str_type = (char *) malloc(sizeof(char) * 7);
    strcpy(str_type, "Window");
  }
  else if(node->nome == FRIDGE){
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
    str[0] = '0';
    str[1] = '\0';
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
}

int codice_messaggio(char ** msg){
  int rt = atoi(msg[MSG_OP]);
  if(rt) >= 10000){
    rt += atoi(MSG_TYPE_DESTINATARIO) * 100000;
  }
  return rt;
}

void crea_messaggio_base(char ** msg, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op){
  char * td = itoa(type_dest);
  char * tm = itoa(type_mit);
  char * id_d = itoa(id_dest);
  char * id_m = itoa(id_mit);
  codice_op = codice_op % 100000; // rimuovo il valore "tipo dispositivo" nel caso presente NB: il tipo mittente è passato come parametro separato
  char * cod_op = itoa(codice_op);
  (*msg) = (char *) malloc(sizeof(char) * (strlen(td) + 1 + strlen(tm) + 1 + strlen(id_d) + 1 + strlen(id_m) + 1 + strlen(cod_op) + 2);
  (*msg)[0] = '\0';
  strcat((*msg), td);
  strcat((*msg), '\n');
  strcat((*msg), tm);
  strcat((*msg), '\n');
  strcat((*msg), id_d);
  strcat((*msg), '\n');
  strcat((*msg), id_m);
  strcat((*msg), '\n');
  strcat((*msg), cod_op);
  strcat((*msg), '\n');
}

void get_all_info(int_list *queue){
  msgbuf messaggio;
  int q, i;
  messaggio.msg_type = 1;
  char * msg;
  crea_messaggio_base(&msg, 0, CONTROLLER, 0, CONTROLLER, MSG_INFO);
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
    if(codice_messaggio(msg) == MSG_INFO){
      tree_insert_device(tree, atoi(msg[MSG_INFO_IDPADRE]), atoi(msg[MSG_ID_MITTENTE]), atoi(msg[MSG_TYPE_MITTENTE]), msg[MSG_INFO_NOME]);
      if(atoi(msg[MSG_TYPE_MITTENTE]) == HUB || atoi(msg[MSG_TYPE_MITTENTE]) == TIMER){
        i += atoi(msg[MSG_INFO_CONTROLDV_NFIGLI]);
      }
    }
  }
  tree_print(tree);
}


void del(char ** cmd, int n, int_list figli_controller){

}

void link(char ** cmd, int n, int_list figli_controller){

}

void list(char ** cmd, int n){

}

void add(char ** cmd, int n){

}

void swtch(char ** cmd, int n){

}

void ricomponi_messaggio(char ** cmd, int n, char ** str){ // ricomponi_messaggio(cmd, &str);
  int i, dim = 1; //dim parte da 1 per il carattere terminatore (\0) a fine stringa
  for(i = 0; i < n; i++){
    dim += strlen(cmd[i])+1;
  }

  *str = (char *) malloc(sizeof(char) * dim);
  (*str)[0] = '\0';
  for(i = 0; i < n; i++){
    strcat(*str,cmd[i]);
    strcat(*str, '\n');
  }
}

invia_broadcast(char * msg, int_list * queue){
  int_node * next = queue->head;
  msgbuf messaggio;
  strcpy(messaggio.msg_text, msg);
  messaggio.msg_type = 1;
  int i;
  for(i = 0; i < queue->n && next != NULL; i++){
    msgsnd(next->val, &messaggio, sizeof(messaggio.msg_text), 0);
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

void hub(int id, int recupero, char * nome){
  int_list * figli = create_int_list();

  int queue;
  msgbuf messaggio;
  int gateway;

  int type_child;

  if((idf1 = fork()) == 0){// codice figlio da fare
    flag = 0;
    crea_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [ON]  => SIGUSR1\n  [OFF] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "ON");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        strcpy(messaggio.msg_text, "OFF");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
    }
  }
  else if((idf2 = fork()) == 0){// codice figlio
    flag = 0;
    crea_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [getTime] => SIGUSR1\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "time");
        messaggio.msg_type = 1;
        printf("=> %s\n", messaggio.msg_text);
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
      else if(flag == 2){ //accendi
        flag = 0;
        strcpy(messaggio.msg_text, "RIP");
        messaggio.msg_type = 1;
        msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      }
    }
  }

  crea_queue(id, &queue);

  if(recupero){
    printf("inizio\n" );
     if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
        printf("errore lettura ripristino\n");
    }
    else{
      stato = messaggio.msg_text[0]-'0';
      interruttore = messaggio.msg_text[1]-'0';
      char ** rt;
      str_split(messaggio.msg_text, &rt);
      t_start = atoi(rt[1]);
    }
    printf("fine\n" );
  }

  //inizio loop
  char ** msg;
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    protocoll_parser(messaggio.msg_text, msg);
    if(atoi(msg[MSG_OP]) == MSG_INFO && atoi(msg[MSG_ID_DESTINATARIO]) == id){ //richiesta info su me stesso

    }
    else if(atoi(msg[MSG_OP]) == MSG_INFO && atoi(msg[MSG_ID_DESTINATARIO]) == 0){ //richiesta info su tutti

    }
    else if(FALSE){ //codice RIP da implementare
      messaggio.msg_type = 10;
      messaggio.msg_text[0] = '0' + stato;
      messaggio.msg_text[1] = '0' + interruttore;
      messaggio.msg_text[2] = ' ';
      messaggio.msg_text[3] = '\0';
      char str[20];
      sprintf(str, "%d" , t_start);
      strcat(messaggio.msg_text, str);
      msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
      kill(idf1, SIGTERM);
      kill(idf2, SIGTERM);
      exit(0);
    }
    else{ // messaggio generico
      msg[MSG_ID_MITTENTE]
    }
    printf("\n\ninterruttore: %d\n", interruttore);
    printf("stato: %d\n", stato);
    printf("time: %d\n", t_start);
  }
  printf("Errore lettura queue BULB\n");
}

void leggiconsole(){
    char * str = (char *) malloc(sizeof(char) * 110);
    char ** cmd;
    int_list figli;
    int n;
    int my_id = 1;
    int my_queue = crea_queue(my_id,&queue);

    int flag = TRUE;
    while (flag) {
        printf("\n>");
        scanf("%100s", str);
        n = str_split(str, cmd);
        if(strcmp(cmd[0], "list") == 0){

        }
        else if(strcmp(cmd[0], "add") == 0){
          add(cmd,n);
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

int main() {
  int queue;
  msgbuf messaggio;
  if(fork()){
    printf("inizio\n" );
    messaggio.msg_type = 1;
    strcpy(messaggio.msg_text, "ciao!");
    crea_queue(100,&queue);
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
    strcpy(messaggio.msg_text, "fail");
    printf("fine\n" );
  }
  else{
    printf("sleep\n" );
    sleep(2);
    printf("sleep fine\n" );
    crea_queue(100,&queue);
    msgrcv(queue, &messaggio, sizeof(messaggio.msg_text), 1, 0);
    printf("|%s|\n", messaggio.msg_text );
  }
/*
  if(fork() == 0){
    risposta();
  }
  else{
    sleep(1);
    lampadina_troll();
  }
    //bulb(1,1);
    /*int queue;
    msgbuf messaggio;
    crea_queue(1, &queue);
    strcpy(messaggio.msg_text,"0");
    messaggio.msg_type = 10;
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);*/
    return 0;
}
