
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main(){
  if(fork() == 0){ //READER
    int fd;
    char *myfifo = "/tmp/myfifo84";
    mkfifo(myfifo, 0666); // mkfifo(<pathname>, <permission>)
    char str1[80], str2[80];
    //while (1) {
      fd = open(myfifo, O_RDONLY); // Open FIFO for read only
      read(fd, str1, 80); // read from FIFO
      printf("\nUser1: %s\n", str1); // write and close
      close(fd);
      /*
      fd = open(myfifo, O_WRONLY); // Open FIFO for write only
      fgets(str2, 80, stdin); // input from user, maxlen=80
      write(fd, str2, strlen(str2)+1);
      close(fd);*/
    //}
  }
  else{ //WRITER
    sleep(5);
    int fd;
    char *myfifo = "/tmp/myfifo84";
    mkfifo(myfifo, 0666); // mkfifo(<pathname>, <permission>)
    char str1[80], str2[80];
    //while (1) {
      fd = open(myfifo, O_WRONLY);
      strcpy(str2, "ciao");
      //fgets(str2, 80, stdin); // input from user, maxlen=80
      write(fd, str2, strlen(str2)+1); // write and close
      close(fd);
      /*
      fd = open(myfifo, O_RDONLY); // Open FIFO for Read only
      read(fd, str1, sizeof(str1)); printf("User2: %s\n", str1); close(fd);*/
    //}
  }
  return 0;
}
