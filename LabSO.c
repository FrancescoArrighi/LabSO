#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

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

int type;

struct pair_int{
    int first;
    int second;
};

struct device{
    int id;
    int fd[2];
};

struct int_node{
    int val;
    struct int_node * next;
};

struct int_list{
    struct int_node * head;
    int n;
};

struct pair_int get_int(int n, struct int_list * list){
    struct pair_int rt;
    if(list->n <= n || n < 0){
        rt.first = FALSE;
    }
    else{
        struct int_node * temp = list->head;
        int i;
        for(i = 0; i < n; i++){
            temp = temp->next;
        }
        rt.first = TRUE;
        rt.second = temp->val;
    }
    return rt;
}

int insert_int(int val, int n, struct int_list * list){
    int rt;
    if(list->n < n || n < 0){
        rt = FALSE;
    }
    else{
        struct int_node * new_node = (struct int_node *) malloc(sizeof(struct int_node));
        new_node->val = val;
        if(n == 0){
            new_node->next = list->head;
            list->head = new_node;
        }
        else{
            struct int_node * temp = list->head;
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

int rm_int(int n, struct int_list * list){
    int rt;
    if(list->n <= n || n < 0){
        rt = FALSE;
    }
    else{
        struct int_node * temp = list->head;
        if(n == 0){
            list->head = temp->next;
            free(temp);
        }else{
            int i;
            for(i = 0; i < n-1; i++){
                temp = temp->next;
            }
            struct int_node * temp2 = temp->next;
            temp->next = temp2->next;
            free(temp2);
        }
        rt = TRUE;
        list->n--;
    }
    return rt;
}

struct int_list * create_int_list(){
    struct int_list * rt = (struct int_list *) malloc(sizeof(struct int_list));
    rt->head = NULL;
    rt->n = 0;
    return rt;
}

struct pair_int_device{
    int integer;
    struct device * val;
};

struct device_node{
    struct device * val;
    struct device_node * next;
};

struct device_list{
    struct device_node * head;
    int n;
};

struct pair_int_device get_device(int n, struct device_list * list){
    struct pair_int_device rt;
    if(list->n <= n || n < 0){
        rt.integer = FALSE;
    }
    else{
        struct device_node * temp = list->head;
        int i;
        for(i = 0; i < n; i++){
            temp = temp->next;
        }
        rt.integer = TRUE;
        rt.val = temp->val;
    }
    return rt;
}

int insert_device(struct device * val, int n, struct device_list * list){
    int rt;
    if(list->n < n || n < 0){
        rt = FALSE;
    }
    else{
        struct device_node * new_node = (struct device_node *) malloc(sizeof(struct device_node));
        new_node->val = val;
        if(n == 0){
            new_node->next = list->head;
            list->head = new_node;
        }
        else{
            struct device_node * temp = list->head;
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

int rm(int n, struct device_list * list){
    int rt;
    if(list->n <= n || n < 0){
        rt = FALSE;
    }
    else{
        struct device_node * temp = list->head;
        if(n == 0){
            list->head = temp->next;
            free(temp);
        }else{
            int i;
            for(i = 0; i < n-1; i++){
                temp = temp->next;
            }
            struct device_node * temp2 = temp->next;
            temp->next = temp2->next;
            free(temp2);
        }
        rt = TRUE;
        list->n--;
    }
    return rt;
}

struct device_list * create_device_list(){
    struct device_list * rt = (struct device_list *) malloc(sizeof(struct device_list));
    rt->head = NULL;
    rt->n = 0;
    return rt;
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
    struct device_list dvs;
    
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
