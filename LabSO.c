//Header globale
#include "myheader.h"

//Struct
typedef struct pair_int{
    int first;
    int second;
} pair_int;

typedef struct device{
    int id;
    int fd[2];
} device;

typedef struct int_node {
    int val;
    struct int_node * next;
} int_node;

typedef struct int_list{
    int_node * head;
    int n;
} int_list;

typedef struct pair_int_device{
    int integer;
    device * val;
} pair_int_device;

typedef struct device_node{
    device * val;
    struct device_node * next;
} device_node;

typedef struct device_list{
    device_node * head;
    int n;
} device_list;

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

int type;

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
        if(str[i] == ' '){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}

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
}

void initialize_child(){
    switch (type) {
        case HUB:
            hub();
            break;

        case TIMER:
            timer();
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

void link(){
    if(
}

void leggiconsole(){
    char * str = (char *) malloc(sizeof(char) * 110);
    char ** cmd;
    device_list dvs;

    int flag = TRUE;
    while (flag) {
        scanf("%100s", str);
        str_split(str, cmd);
        if(strcmp(cmd[0], "list") == 0){

        }
        else if(strcmp(cmd[0], "add") == 0){

        }
        else if(strcmp(cmd[0], "del") == 0){

        }
        else if(strcmp(cmd[0], "link") == 0){
            link(cmd);
        }
        else if(strcmp(cmd[0], "switch") == 0){

        }
        else if(strcmp(cmd[0], "info") == 0){

        }
        else if(strcmp(cmd, "quit") == 0){
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

int main() {
    type = CONTROLLER;
    console();
    return 0;
}
