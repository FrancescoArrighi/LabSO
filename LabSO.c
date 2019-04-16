#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

void leggiconsole(){
    char * cmd = (char *) malloc(sizeof(char) * 110);
    int flag = TRUE;
    while (flag) {
        scanf("%100s", cmd);
        if(strcmp(cmd, "prova1") == 0){
            printf("1\n" );
        }
        else if(strcmp(cmd, "prova2") == 0){
            printf("2\n" );
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
    leggiconsole();
    return 0;
}
