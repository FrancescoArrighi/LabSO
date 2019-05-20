#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define FIFO "/tmp/myfifo"

int main(){
  char buf_w[50];
  int fd, flag = 1;

  if((mkfifo(FIFO, 0666) == -1) && (errno == EEXIST)){ //se fallisce perché esiste già il file
    unlink(FIFO);
    if(mkfifo(FIFO, 0666)==-1){
      perror("Errore mkfifo");
      exit(1);
    }
  }
  else if((mkfifo(FIFO, 0666) == -1)){ //se è per altri errori
    perror("Errore mkfifo");
    exit(1);
  }



  while(flag){
    fd = open(FIFO, O_WRONLY);
    fgets(buf_w, 50, stdin);
    write(fd, buf_w, strlen(buf_w)+1);
    if(strcmp(buf_w, "exit") == 0){
      printf("Fine scrittura\n");
      flag = 0;
    }
    close(fd);
  }
  unlink(FIFO);

  return 0;
}
