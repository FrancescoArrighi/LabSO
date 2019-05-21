#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 150

#define TRUE 1
#define FALSE 0

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

int main(){
  printf("FIFO LETTURA\n");
  int id = 0;
  char *fifo_w = (char *) malloc(sizeof(char)*20);
  char *fifo_r = (char *) malloc(sizeof(char)*20);
  sprintf(fifo_w, "/tmp/D_%d_W", id);
  sprintf(fifo_r, "/tmp/D_%d_R", id);
  char buf_r[BUF_SIZE], buf_w[BUF_SIZE];
  int fd1, fd2, n_arg, flag = TRUE;
  char **cmd, *str;

  //errno = EEXIST quando il path esiste già, in tale caso trascuro l'errore e procedo normalmente
  if((mkfifo(fifo_w, 0666) == -1) && (errno != EEXIST)){
    perror("Errore mkfifo");
    exit(1);
  }

  if((mkfifo(fifo_r, 0666) == -1) && (errno != EEXIST)){
    perror("Errore mkfifo");
    exit(1);
  }

  //printf("%s\n", fifo_r);

  while(flag){
    fd1 = open(fifo_r, O_RDONLY);

    read(fd1, buf_r, BUF_SIZE);
    printf("%s\n", buf_r);
    n_arg = str_split(buf_r, &cmd);
    if((atoi(cmd[0]) == id) && (n_arg >= 3)){
      printf("Operazione eseguita!\n");
    }
    else if((atoi(cmd[0]) == id) && (n_arg == 2) && (strcmp(cmd[1], "info") == 0)){
      fd2 = open(fifo_w, O_WRONLY);
      printf("apro fifo scrittura\n");
      //memset(buf_w, 0, sizeof(buf_w));
      strcpy(buf_w, "stato: 1\ninterruttore: 0\ntemp: 3\nperc: 0\n");
      write(fd2, buf_w, strlen(buf_w)+1);
      printf("Fine scrittura\n");
    }
    else if(strcmp(cmd[1], "close") == 0){ //esco dal ciclo
      printf("Chiusura fifo\n");
      flag = FALSE;
    }
    else{
      printf("Comando non valido!\n");
    }
    close(fd1);
    close(fd2);
  }
  unlink(fifo_r);
  unlink(fifo_w);
  return 0;
}
