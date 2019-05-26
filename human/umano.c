#include "myheader.h"
#include "human_fun.h"

#define HELP_STRING \
"\n Sintassi: <id device> <type device> <op> [value]\n \
Sintassi: exit - chiusura \n\n \
Operazioni supportate: \n \
 - Bulb: \n \
  - inverti interruttore \n \
  - get : info, time \n \
 - Window: \n \
  - open \n \
  - close \n \
  - get : info, time \n \
 - Fridge: \n \
  - get : info, stato, time, delay, percentuale, termostato \n \
  - set : interruttore, delay, percentuale, termostato \n\n \
Esempio operazioni (dove 71 è id di un frigo): \n \
  -> 71 fridge get info \n \
  -> 71 fridge set stato 1 \n \
  -> 71 fridge interruttore \n \
  -> exit \n \
"

int check_get(char ** cmd, int n_arg){
  int rt = FALSE;
  if((strcmp(cmd[1], "bulb") == 0) || (strcmp(cmd[1], "window") == 0) || (strcmp(cmd[1], "fridge") == 0)) {
    if((n_arg == 4) && (strcmp(cmd[2], "get") == 0)){
      if((strcmp(cmd[3], "info") == 0) || (strcmp(cmd[3], "time") == 0) || (strcmp(cmd[3], "delay") == 0)
      || (strcmp(cmd[3], "stato") == 0) || (strcmp(cmd[3], "percentuale") == 0) || (strcmp(cmd[3], "termostato") == 0)){
        rt = TRUE;
      }
    }
  }

  return rt;
}

int main(){
  msgbuf risposta;

  char *fifo_r = (char *) malloc(sizeof(char)*20);
  char *fifo_w = (char *) malloc(sizeof(char)*20);
  char buf_w[MSG_SIZE], buf_r[BUF_SIZE];
  char **cmd;
  int fd, flag = TRUE, n_arg;
  int ack = TRUE;

  printf("%s\n", HELP_STRING);

  while(flag){
    printf("------------scrittura----------\n");
    fgets(buf_w, BUF_SIZE, stdin);
    if(buf_w[0] != '\n'){ // risolve il problema di segmentation fault quando vado a capo non scrivendo nulla
      n_arg = str_split(buf_w, &cmd);

      if(strcmp(cmd[0], "exit") == 0){
        printf("Fine scrittura\n");
        flag = 0;
      }
      else if(n_arg == 3){ //Azioni dirette sugli interruttori (atomiche)
        sprintf(fifo_w, "/tmp/D_%s_R", cmd[0]);
        printf("%s\n", fifo_w);
        fd = open(fifo_w, O_WRONLY);
        strcpy(risposta.msg_text, "\0");

        if(strcmp(cmd[1], "window") == 0) {
          if(strcmp(cmd[2], "open") == 0){
            crea_messaggio_base(&risposta, WINDOW, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_WINDOW_OPEN);
          }
          else if(strcmp(cmd[2], "close") == 0){
            crea_messaggio_base(&risposta, WINDOW, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_WINDOW_CLOSE);
          }
          else {
            crea_messaggio_base(&risposta, WINDOW, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
            printf("Operazione non valida per Window\n");
          }
        }
        else if(strcmp(cmd[1], "bulb") == 0) {
          if(strcmp(cmd[2], "interruttore") == 0){
            crea_messaggio_base(&risposta, BULB, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_BULB_SWITCH_I);
          }
          else {
            crea_messaggio_base(&risposta, BULB, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
            printf("Operazione non valida per Bulb\n");
          }
        }
        /*else if(strcmp(cmd[1], "hub") == 0) {
          if(strcmp(cmd[2], "interruttore") == 0){
            crea_messaggio_base(&risposta, HUB, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_HUB_SWITCH_I);
          }
          else {
            crea_messaggio_base(&risposta, HUB, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
            printf("Operazione non valida per Fridge\n");
          }
        }*/

        else { //Nel caso di input sbagliati scrivo comunque un messaggio di ack negativo
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
          printf("Operazione non valida\n");
        }

        write(fd, risposta.msg_text, strlen(risposta.msg_text)+1);
        printf("buf_w: %s\n", risposta.msg_text);
        close(fd);
      }
      else if((n_arg == 5) && (strcmp(cmd[2], "set") == 0)) { //Richieste di setting
        sprintf(fifo_w, "/tmp/D_%s_R", cmd[0]);
        printf("%s\n", fifo_w);
        fd = open(fifo_w, O_WRONLY);
        strcpy(risposta.msg_text, "\0");

        if(strcmp(cmd[1], "fridge") == 0) {
          if(strcmp(cmd[3], "delay") == 0){
            crea_messaggio_base(&risposta, FRIDGE, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_SETDELAY);
            concat_string(&risposta, cmd[4]);concat_string(&risposta, cmd[4]);
          }
          else if(strcmp(cmd[3], "termostato") == 0){
            crea_messaggio_base(&risposta, FRIDGE, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_SETTERMOSTATO);
            concat_string(&risposta, cmd[4]);
          }
          else if(strcmp(cmd[3], "percentuale") == 0){
            crea_messaggio_base(&risposta, FRIDGE, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_SETPERC);
            concat_string(&risposta, cmd[4]);
          }
          else if(strcmp(cmd[3], "interruttore") == 0){
            crea_messaggio_base(&risposta, FRIDGE, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_SETINTERRUTTORE);
            concat_string(&risposta, cmd[4]);
          }
          else {
            crea_messaggio_base(&risposta, FRIDGE, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
            printf("Operazione non valida per Fridge\n");
          }
        }
        else { //Nel caso di input sbagliati scrivo comunque un messaggio di ack negativo
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
          printf("Operazione non valida\n");
        }

        write(fd, risposta.msg_text, strlen(risposta.msg_text)+1);
        printf("buf_w: %s\n", risposta.msg_text);
        close(fd);
    }
      else if((n_arg == 4) && (strcmp(cmd[2], "get") == 0) && check_get(cmd,n_arg)) { //Richieste di info o time
        sprintf(fifo_w, "/tmp/D_%s_R", cmd[0]);
        printf("%s\n", fifo_w);
        fd = open(fifo_w, O_WRONLY);
        strcpy(risposta.msg_text, "\0");
        printf("cmd 3: %s\n", cmd[3]);
        printf("cmd 1: %s\n", cmd[1]);

        if((strcmp(cmd[3], "info") == 0) && ((strcmp(cmd[1], "bulb") == 0) || (strcmp(cmd[1], "window") == 0) || (strcmp(cmd[1], "fridge") == 0))) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_INF);
        }
        else if((strcmp(cmd[3], "time") == 0) && ((strcmp(cmd[1], "bulb") == 0) || (strcmp(cmd[1], "window") == 0) || (strcmp(cmd[1], "fridge") == 0))) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_GETTIME);
        }
        else if((strcmp(cmd[3], "delay") == 0) && (strcmp(cmd[1], "fridge") == 0)) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_GETDELAY);
        }
        else if((strcmp(cmd[3], "percentuale") == 0) && (strcmp(cmd[1], "fridge") == 0)) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_GETPERC);
        }
        else if((strcmp(cmd[3], "stato") == 0) && (strcmp(cmd[1], "fridge") == 0)) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_GETSTATO);
        }
        else if((strcmp(cmd[3], "termostato") == 0) && (strcmp(cmd[1], "fridge") == 0)) {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_FRIDGE_GETTERMOSTATO);
        }
        else {
          crea_messaggio_base(&risposta, DEFAULT, DEFAULT, atoi(cmd[0]), atoi(cmd[0]), MSG_ACKN);
          ack = FALSE;
          printf("Operazione non valida\n");
        }
        write(fd, risposta.msg_text, strlen(risposta.msg_text)+1);
        printf("buf_w: %s\n", risposta.msg_text);
        close(fd);

        if (ack == TRUE) {
          //Per evitare di aprire in caso di input sbagliati dovrei fare un controllo sul codice op inviato
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
        ack = TRUE;
      }
      else{
        printf("\nComando non valido!\n");
        printf("Desideri uscire? [s/n]\n");
        fgets(buf_w, BUF_SIZE, stdin);
        //printf("Risposta: %s\n",buf_w );
        if (strcmp(buf_w, "s\n") == 0) {
          printf("Scrittura finita\n");
          flag = 0;
        }
      }
    }
  }
  return 0;

}
