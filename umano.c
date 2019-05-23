#include "myheader.h"
#include "useful_fun.h"

#define HELP_STRING \
"\n Sintassi: <id device> <get/set> <op> [value]\n \
Sintassi: <id device> close -> chiusura fifo con dispositivo avente come id <id device> \n \
Sintassi: exit - chiusura shell\n\n \
Operazioni supportate: \n \
 - bulb / window: \n \
  - get : info, stato, interruttore, time \n \
  - set : interruttore \n \
 - fridge: \n \
  - get : info, stato, interruttore, time, delay, percentuale, termostato \n \
  - set : interruttore, delay, percentuale, termostato \n\n \
Esempio operazioni (dove 71 è id di un frigo): \n \
  -> 71 get stato \n \
  -> 71 set stato 1 \n \
  -> 71 close \n \
  -> exit \n \
"

int check_cmd(char **cmd, int n_arg){ //controllo se il comando è tra quelli ammessibili
  int rt = FALSE;
  if((strcmp(cmd[1], "get") == 0) && (n_arg == 3)){
    if((strcmp(cmd[2], "interruttore") == 0) && (strcmp(cmd[2], "percentuale") == 0)
    && (strcmp(cmd[2], "termostato") == 0) && (strcmp(cmd[2], "delay") == 0)
    && (strcmp(cmd[2], "stato") == 0) && (strcmp(cmd[2], "time") == 0) && (strcmp(cmd[2], "info") == 0)){
      rt = TRUE;
    }
  }
  else if(strcmp(cmd[1], "set") == 0  && (n_arg == 4)){
    if((strcmp(cmd[2], "interruttore") == 0) && (strcmp(cmd[2], "percentuale") == 0)
    && (strcmp(cmd[2], "termostato") == 0) && (strcmp(cmd[2], "delay") == 0)){
      rt = TRUE;
    }
  }
  else if(strcmp(cmd[1], "interruttore") == 0  && (n_arg == 2)){
    rt = TRUE;
  }
  else if(strcmp(cmd[1], "open") == 0  && (n_arg == 2)){
    rt = TRUE;
  }
  else if(strcmp(cmd[1], "close") == 0  && (n_arg == 2)){
    rt = TRUE;
  }
  return rt;
}

int main(){
  char *fifo_r = (char *) malloc(sizeof(char)*20);
  char *fifo_w = (char *) malloc(sizeof(char)*20);
  char buf_w[BUF_SIZE], buf_r[BUF_SIZE];
  char **cmd;
  int fd, flag = TRUE, n_arg;

  printf("%s\n", HELP_STRING);

  while(flag){
    printf("------------scrittura----------\n");
    fgets(buf_w, BUF_SIZE, stdin);
    n_arg = str_split(buf_w, &cmd);

    if(strcmp(cmd[0], "exit") == 0){
      printf("Fine scrittura\n");
      flag = 0;
    }
    else if(check_cmd(cmd, n_arg) || (strcmp(cmd[1], "close") == 0)){ //se il comando è corretto o è close
      if((n_arg >= 2)){ //se abbiamo almeno 2 parametri apro fifo di scrittura e scrivo
        sprintf(fifo_w, "/tmp/D_%s_R", cmd[0]);
        printf("%s\n", fifo_w);
        fd = open(fifo_w, O_WRONLY);
        write(fd, buf_w, strlen(buf_w)+1);
        printf("buf_w: %s\n", buf_w);
        close(fd);
      }

      if((n_arg == 3) && (strcmp(cmd[1], "get") == 0)){ //se è una richiesta get apro anche quello di lettura
        printf("risposta get\n");
        sprintf(fifo_r, "/tmp/D_%s_W", cmd[0]);
        printf("%s\n", fifo_r);
        fd = open(fifo_r, O_RDONLY);
        printf("lettura risposta\n");
        read(fd, buf_r, BUF_SIZE);
        printf("%s\n", buf_r);
        close(fd);
        printf("fifo lettura chiusa\n");
      }
    }
    else{
      printf("Comando non valido!\n");
    }
  }

  return 0;

}
