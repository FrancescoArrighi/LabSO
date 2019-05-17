//Header
#include "useful_fun.h"

pair_int get_int(int n, int_list * list){
    pair_int rt;
    if(list->n <= n || n < 0){
        rt.first = FALSE;
    }
    else{
        int_node * temp = list->head;
        int i;
        for(i = 0; i < n; i++){
            temp = temp->next;
        }
        rt.first = TRUE;
        rt.second = temp->val;
    }
    return rt;
}

int_list * create_int_list(){
    int_list * rt = (int_list *) malloc(sizeof(int_list));
    rt->head = NULL;
    rt->n = 0;
    return rt;
}

pair_int_device get_device(int n, device_list * list){
    pair_int_device rt;
    if(list->n <= n || n < 0){
        rt.integer = FALSE;
    }
    else{
        device_node * temp = list->head;
        int i;
        for(i = 0; i < n; i++){
            temp = temp->next;
        }
        rt.integer = TRUE;
        rt.val = temp->val;
    }
    return rt;
}
device_list * create_device_list(){
    device_list * rt = (device_list *) malloc(sizeof(device_list));
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

int insert_device(device * val, int n, device_list * list){
    int rt;
    if(list->n < n || n < 0){
        rt = FALSE;
    }
    else{
        device_node * new_node = (device_node *) malloc(sizeof(device_node));
        new_node->val = val;
        if(n == 0){
            new_node->next = list->head;
            list->head = new_node;
        }
        else{
            device_node * temp = list->head;
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

int rm(int n, device_list * list){
    int rt;
    if(list->n <= n || n < 0){
        rt = FALSE;
    }
    else{
        device_node * temp = list->head;
        if(n == 0){
            list->head = temp->next;
            free(temp);
        }else{
            int i;
            for(i = 0; i < n-1; i++){
                temp = temp->next;
            }
            device_node * temp2 = temp->next;
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
        if(str[i] == ' ' || str[i] == '\n'){
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

//ritorna la dimensione della stringa (compreso il terminatore) e inserisce il risultato in str
//NB: la funzione va chiamata così itoa(numero, &str); dove str è un (char *)
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

int codice_messaggio(char ** msg){
  int rt = atoi(msg[MSG_OP]);
  if((rt) >= 10000){
    rt += atoi(msg[MSG_TYPE_DESTINATARIO]) * 100000;
  }
  return rt;
}

void crea_messaggio_base(char ** msg, int type_dest, int type_mit, int id_dest, int id_mit, int codice_op){
  char * td;
  itoa(type_dest, &td);
  char * tm;
  itoa(type_mit, &tm);
  char * id_d;
  itoa(id_dest, &id_d);
  char * id_m;
  itoa(id_mit, &id_m);
  codice_op = codice_op % 100000; // rimuovo il valore "tipo dispositivo" nel caso presente NB: il tipo mittente è passato come parametro separato
  char * cod_op;
  itoa(codice_op, &cod_op);
  (*msg) = (char *) malloc(sizeof(char) * (strlen(td) + 1 + strlen(tm) + 1 + strlen(id_d) + 1 + strlen(id_m) + 1 + strlen(cod_op) + 2));
  (*msg)[0] = '\0';
  strcat((*msg), td);
  strcat((*msg), "\n");
  strcat((*msg), tm);
  strcat((*msg), "\n");
  strcat((*msg), id_d);
  strcat((*msg), "\n");
  strcat((*msg), id_m);
  strcat((*msg), "\n");
  strcat((*msg), cod_op);
  strcat((*msg), "\n");
}
