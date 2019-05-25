//Header
#include "useful_fun.h"

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
  if((key = ftok("/tmp/domotica.txt", id)) == -1){ // crea la chiave
      perror("errore ftok ");
      exit(1);
  }
  if(((*queue) = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste
      perror("errore msgget ");
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
    printf("Timer: %d\n", atoi(str[9]));
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
   //printf("k1 %d\n", atoi(str[3]));
   int i;
   for ( i = 0; str[3][i] != '\0'; i++) {
     //printf("> %d - %c\n", str[3][i], str[3][i]);
   };
   if(atoi(str[3]) == id || atoi(str[3]) == 0){
     rt = TRUE;
     //printf("k2\n" );
   }
 }
 return rt;
}

int codice_messaggio(char ** msg){
  int rt = atoi(msg[MSG_OP]);
  return rt;
}

void concat_int(msgbuf * messaggio, int n){
  char * str;
  itoa(n, &str);
  strcat(messaggio->msg_text, str);
  strcat(messaggio->msg_text, "\n");
}

void concat_string(msgbuf * messaggio, char * str){
  strcat(messaggio->msg_text, str);
  strcat(messaggio->msg_text, "\n");
}

void crea_messaggio_base(msgbuf * messaggio, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op){

  messaggio->msg_text[0] = '\0';
  concat_int(messaggio, type_dest);
  concat_int(messaggio, type_mit);
  concat_int(messaggio, id_dest);
  concat_int(messaggio, id_mit);
  concat_int(messaggio, codice_op);
}

void ricomponi_messaggio(char ** cmd, int n, msgbuf * messaggio){ // ricomponi_messaggio(cmd, &str);

  int i;
  messaggio->msg_text[0] = '\0';
  for(i = 0; i < n; i++){
    concat_string(messaggio, cmd[i]);
  }
}

void svuota_msg_queue(int queue, int p){
  msgbuf messaggio;
  errno = 0;
  while (errno != ENOMSG){
    msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), p, IPC_NOWAIT);
  }
}

void invia_broadcast(msgbuf * messaggio, int_list * queue){
  int i, next;
  //printf("\n inizio broadcast n: %d\n", queue->n);
  for(i = 0; get_int(i, &next, queue); i++){
    //printf("\n broadcast invio %d-esimo: |%d| - |%ld|\n", i, next, messaggio->msg_type);
    msgsnd(next, messaggio, sizeof(messaggio->msg_text), 0);
  }
  //printf("\n fine broadcast\n");
}

void recupero_in_cascata(int queue){
  char ** msg;
  msgbuf messaggio;
  //printf("tasso1\n");
  if(leggi(queue, &messaggio, 10, 2)){
    protocoll_parser(messaggio.msg_text, &msg);
    //printf("tasso3\n");
    int type = atoi(msg[MSG_RECUPERO_TYPE]);
    int myid = atoi(msg[MSG_RECUPERO_ID]);
    //printf("tasso4\n");
    msgsnd(queue, &messaggio ,sizeof(messaggio.msg_text), 0);
    //printf("tasso5\n");
    if(type == HUB){
      if(fork() == 0){
        //printf("tasso6\n");
        hub(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == TIMER){
      if(fork() == 0){
        //timer(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == BULB){
      if(fork() == 0){
        bulb(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == WINDOW){
      if(fork() == 0){
        window(myid, TRUE, "");
        exit(0);
      }
    }
    else if(type == FRIDGE){
      if(fork() == 0){
        fridge(myid, TRUE, "");
        exit(0);
      }
    }
  }
  else{
    printf("errore ripristino\n");
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

// Funzione che crea una stringa "/tmp/D_id_" con l'id passato aggiungendo
//una W o R a seconda che il file sia Writer o Reader
char * percorso_file(int id, int tipo){
  char * tmp1 = "/tmp/D_";
  char * tmp2;
  itoa(id,&tmp2);
  char * r = (char *) malloc(sizeof(char) * (strlen(tmp1) + strlen(tmp2) + 3));
  strcpy(r, tmp1);
  strcat(r, tmp2);

  if (tipo == READ) {
    strcat(r, "_R");
  }
  else {
    strcat(r, "_W");
  }

  return r;
}
