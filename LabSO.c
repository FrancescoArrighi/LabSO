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

struct device{
    int id;
    int fd[2];
};

void str_split(char * str, char ** rt){
    int i = 0, j = 0;
    rt[0] = (char *) malloc(sizeof(char) * 110);
    while(*str != '\0'){
        if(*str == ' '){
            i++;
            j = 0;
            rt[i] = (char *) malloc(sizeof(char) * 110);        }
        else{
            rt[i][j] = *str;
        }
    }
}

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

void prepare_fork(int figlio, device * devices){
    device *temp = (device *) malloc(sizeof(device));
    int fd0[2], fd1[2];
    pipe(fd0);
    pipe(fd1);
    if(fork() > 0){
        temp.fd[READ] = fd0[READ];
        temp.fd[WRITE] = fd1[WRITE];
    }
    else{
        temp.fd[READ] = fd1[READ];
        temp.fd[WRITE] = fd0[WRITE];
        type = figlio;
        initialize_child();
    }
}

void leggiconsole(){
    char * str = (char *) malloc(sizeof(char) * 110);
    char ** cmd;
    device * devices;
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
