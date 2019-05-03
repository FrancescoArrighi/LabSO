//Librerie che possiamo usare
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
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

//valori di verità
#define TRUE 1
#define FALSE 0

//dispositivi
#define CONTROLLER 0
#define HUB 1
#define TIMER 2
#define BULB 3
#define WINDOW 4
#define FRIDGE 5

//pipes
#define READ 0
#define WRITE 1

//funzioni
#define ADD 1

// altro
int type;

//Struct
typedef struct {
    int first;
    int second;
} pair_int;

typedef struct {
  int id;
  pid_t pid;
  int fd[2];
} device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct {
    int_node * head;
    int n;
} int_list;

typedef struct {
    int integer;
    device * val;
} pair_int_device;

typedef struct device_node{
    device * val;
    struct device_node * next;
} device_node;

typedef struct {
    device_node * head;
    int n;
} device_list;

typedef struct{
  int stato;
  int ovveride;
  int interruttori;
    
}


//Dichiarazioni Funzioni
pair_int get_int(int n, int_list * list);
int_list * create_int_list();
pair_int_device get_device(int n, device_list * list);
device_list * create_device_list();
int insert_int(int val, int n, int_list * list);
int rm_int(int n, int_list * list);
int insert_device(device * val, int n, device_list * list);
int rm(int n, device_list * list);
void str_split(char * str, char ** rt);
void initialize_child();
void prepare_fork(int figlio, device * devices);
void leggiconsole();
void console();

int del_device(pid_t id, device_list *list);
void add_device(int device_type, device_list *list);
void print_device();

device_list *list;
int num_id; // numero progressivo per id

int main() {
  list = create_device_list();
  device *new_device = (device*) malloc(sizeof(device));
  new_device->id = num_id;
  num_id++;
  new_device->pid = getpid();
  if(!insert_device(new_device, list->n, list)){
    free(new_device);
    perror("Errore inserimento dispositivo nella lista");
    exit(EXIT_FAILURE);
  }
  printf("Centralina(id %d) aggiunto con successo!\n", ((list->head)->val)->id);
  add_device(1, list);
  printf("\n\n");
  system("ps");
  printf("\n\n");
  
  pid_t tmp;
  printf("Inserire il id processo da eliminare: ");
  scanf("%d", &tmp);
  del_device(tmp, list);
  
  printf("\n\n");
  system("ps");
  printf("\n\n");
  return 0;
}

void print_device(){
  printf("------------Stampo lista id----------\n");
  int i = 0;
  device_node *temp = list->head;
  while(i<=list->n){
    printf("device n.%d, id %d\n", i, (temp->val)->id);
    temp = temp->next;
    i++;
  }
}

int del_device(pid_t id, device_list *list){
  // occorre capire i dispositivi figli
  printf("\n--------------delete--------------\n");
  printf("sono il processo pid=%d\n\n", getpid());
  printf("list size = %d\n", list->n);
  
  int found = FALSE;
  int current_pos = 0;
  device_node *temp = list->head;
  device *current_dvc;
  while(temp!=NULL && !found){ 
    printf("current_pos=%d\n", current_pos);
    current_dvc = temp->val;
    if(current_dvc->id==id){
      printf("trovo il device\n");
      rm(current_pos, list);
      printf("size dopo del: %d\n",list->n);
      printf("uccido il processo pid %d\n", getpid());
      found = TRUE;
      kill(current_dvc->pid, SIGTERM);  
    }
    else{
      current_pos++;
      temp = temp->next;
    }
    printf("end\n");
  }
  return found;
}

void add_device(int device_type, device_list *list){ 
  device *new_device = (device*) malloc(sizeof(device));
  new_device->id = num_id;
  num_id++;
  if(!insert_device(new_device, list->n, list)){
    free(new_device);
    perror("Errore inserimento dispositivo nella lista");
    exit(EXIT_FAILURE);
  }
  pair_int_device temp = get_device((list->n)-1, list);
  printf("Dispositivo(id %d) aggiunto con successo!\n", (temp.val)->id);

  pid_t pid = fork();
  if(pid == -1){
    rm(list->n, list);
    num_id--;
    perror("Errore fork");
    exit(1);
  }else if(pid == 0){
    pair_int_device ultimo_inserito = get_device((list->n)-1, list);
    printf("sono figlio id =%d, pid=%d, ppid=%d\n",(ultimo_inserito.val)->id, getpid(), getppid());
    printf("add: new size = %d\n", list->n);
    pair_int_device ultimo = get_device((list->n)-1, list);
    (ultimo.val)->pid = pid;
    pause();
  }else{
    sleep(1);
    pair_int_device ultimo = get_device((list->n)-1, list);
    (ultimo.val)->pid = pid;
  }
}



void prepare_fork(int figlio, device * devices){
    device *temp = (device *) malloc(sizeof(device));
    int fd0[2], fd1[2];
    pipe(fd0);
    pipe(fd1);
    if(fork() > 0){
        temp->fd[READ] = fd0[READ];
        temp->fd[WRITE] = fd1[WRITE];
    }
    else{
        temp->fd[READ] = fd1[READ];
        temp->fd[WRITE] = fd0[WRITE];
        type = figlio;
        initialize_child();
    }
}

void leggiconsole(){
    char * str = (char *) malloc(sizeof(char) * 110);
    char ** cmd;
    int flag = TRUE;
    while (flag) {
      printf("Inserire un'operazione:\n");
      printf(">");
        scanf("%100s", str);
        str_split(str, cmd);
        if(strcmp(cmd[0], "list") == 0){

        }
        else if(strcmp(cmd[0], "add") == 0){
        }
        else if(strcmp(cmd[0], "del") == 0){
	  
        }
        else if(strcmp(cmd[0], "link") == 0){
	  // link(cmd);
        }
        else if(strcmp(cmd[0], "switch") == 0){

        }
        else if(strcmp(cmd[0], "info") == 0){

        }
        else if(strcmp(cmd[0], "quit") == 0){
            flag = FALSE;
        }
        else{
            printf("comando non valido \n");
        }
    }
}

void console(){
    leggiconsole();
}

void str_split(char * str, char ** rt){
    int i = 0, j = 1;
    for(i = 0; str[i] != '\0'; i++){
        if(str[i] = ' '){
            j++;
        }
    }
    rt = (char **) malloc(sizeof(char *) * j);
    rt[0] = (char *) malloc(sizeof(char) * 110);
    while(*str != '\0'){
        if(*str == ' '){
            i++;
            j = 0;
            rt[i] = (char *) malloc(sizeof(char) * 110);
        }
        else{
            rt[i][j] = *str;
            j++;
        }
    }

}

/*
void hub(){
    //inizializza
    int n_figli = 0;
    while(1){
        //leggo
        read();
        //elaboro
        interruttore = true;
        write(canale,strigna_risposta);
    }
    }*/

void initialize_child(){
    switch (type) {
        case HUB:
            break;

        case TIMER:
            break;

        case BULB:

            break;

        case WINDOW:

            break;

        case FRIDGE:

            break;

        default:                //inserire messaggio errore
            break;
    }
}
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


