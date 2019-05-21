#include "myheader.h"
#include "useful_fun.h"

int main(){
  char *fifo_r = (char *) malloc(sizeof(char)*20);
  char *fifo_w = (char *) malloc(sizeof(char)*20);
  char buf_w[BUF_SIZE], buf_r[BUF_SIZE];
  char **cmd;
  int fd, flag = TRUE, n_arg;

  printf("Sintassi: <id device> <op> [value]\n");
  printf("Sintassi: <id device> close -> chiusura fifo con dispositivo avente come id <id device> \n");
  printf("Sintassi: exit - chiusura shell\n");

  while(flag){
    printf("------------scrittura----------\n");
    fgets(buf_w, BUF_SIZE, stdin);
    n_arg = str_split(buf_w, &cmd);

    if(strcmp(cmd[0], "exit") == 0){
      printf("Fine scrittura\n");
      flag = 0;
    }
    else if((n_arg >= 2)){
      sprintf(fifo_w, "/tmp/D_%s_R", cmd[0]);
      printf("%s\n", fifo_w);
      fd = open(fifo_w, O_WRONLY);
      write(fd, buf_w, strlen(buf_w)+1);
      close(fd);
    }

    if((n_arg == 2) && (strcmp(cmd[1], "info") == 0)){
      printf("info\n");
      sprintf(fifo_r, "/tmp/D_%s_W", cmd[0]);
      fd = open(fifo_r, O_RDONLY);
      printf("lettura info\n");
      read(fd, buf_r, BUF_SIZE);
      printf("%s\n", buf_r);
      close(fd);
      printf("fifo lettura chiusa\n");
    }
  }

  return 0;

}
