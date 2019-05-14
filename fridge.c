#include "fridge.h"

/* ---codice usato per prova--

char * plus_n (char * a) {
  char * b = (char *) malloc (sizeof(char) *(strlen(a)+2));
  strcpy(b,a);
  b[strlen(a)] = '\n';
  b[strlen(a)+1] = '\0';

  return b;
}

void send_prl(char * prl, char *dest, char *src, char *dim, char *destid, char *srcid, char *cod){
  char *tmp = (char*) malloc(sizeof(char)*20);
  int n = 6;

  for (int i = 0; (i < n); i++) {
    if (i==0) { //Tipologia destinatario - prl[0]
      //sprintf(tmp, "%d", dest); //itoa(dest,tmp);
      strcpy(tmp, dest);
    }
    else if (i==1) { //Tipologia mittente - prl[2]
      // sprintf(tmp, "%d", src); //itoa(src,tmp);
      strcpy(tmp, src);
    }
    else if (i==2) { //Dim prl (3 char) - prl[4]
      //sprintf(tmp, "%d", dim); //itoa(dim,tmp);
       strcpy(tmp, dim);
    }
    else if (i==3) { //ID destinatario - prl[8]
      //sprintf(tmp, "%d", destid); //itoa(destid,tmp);
      strcpy(tmp, destid);
    }
    else if (i==4) { //ID mittente - prl[19]
      //sprintf(tmp, "%d", srcid); //itoa(srcid,tmp);
       strcpy(tmp, srcid);
    }
    else if (i==5) { //Codice - prl[30]
      // I codici che iniziano con lo zero sono un problema
      //sprintf(tmp, "%d", cod); //itoa(cod,tmp);
       strcpy(tmp, cod);
    }
    strcat(prl,plus_n(tmp));
  }
}


//serve per inviare richieste specifiche es. apri/chiudi fridge, cambio delay etc..
// valore => cambia in base al codice op (valore da impostare o id nuovo frigo)
void send_prl_spec(char * prl, char *dest, char *src, char *dim, char *destid, char *srcid, char *cod, char *valore){
  char *tmp = (char*) malloc(sizeof(char)*20);
  int n = 7;

  for (int i = 0; (i < n); i++) {
    if (i==0) { //Tipologia destinatario - prl[0]
      //sprintf(tmp, "%d", dest); //itoa(dest,tmp);
      strcpy(tmp, dest);
    }
    else if (i==1) { //Tipologia mittente - prl[2]
      // sprintf(tmp, "%d", src); //itoa(src,tmp);
      strcpy(tmp, src);
    }
    else if (i==2) { //Dim prl (3 char) - prl[4]
      //sprintf(tmp, "%d", dim); //itoa(dim,tmp);
       strcpy(tmp, dim);
    }
    else if (i==3) { //ID destinatario - prl[8]
      //sprintf(tmp, "%d", destid); //itoa(destid,tmp);
      strcpy(tmp, destid);
    }
    else if (i==4) { //ID mittente - prl[19]
      //sprintf(tmp, "%d", srcid); //itoa(srcid,tmp);
       strcpy(tmp, srcid);
    }
    else if (i==5) { //Codice - prl[30]
      // I codici che iniziano con lo zero sono un problema
      //sprintf(tmp, "%d", cod); //itoa(cod,tmp);
       strcpy(tmp, cod);
    }
    else if(i==6){
      strcpy(tmp, valore);
    }
    strcat(prl,plus_n(tmp));
  }
}



void print_info_fridge(char ** str){ // stampa le info ricevute
    if(str[6][0] == '0'){
        printf("Stato: OFF\n" );
    }
    else{
      printf("Stato: ON\n");
    }
    if(str[7][0] == '0'){
        printf("Interruttore: OFF\n" );
    }
    else{
      printf("Interruttore: ON\n");
    }
    printf("Termostato: %d\n", atoi(str[8]));
    printf("Timer: %d\n", atoi(str[9]));
    printf("Delay: %d\n", atoi(str[10]));
    printf("Percentuale: %d\n", atoi(str[11]));
    printf("Nome: %s\n", str[12]);

}

void risposta(){
  printf("-----------Centralina: messaggio ricevuto--------------\n");
  int queue;
  msgbuf messaggio;
  create_queue(100,&queue);
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) == -1) {
     printf("Errore\n" );
  }
  else{
    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    int i;
    printf("------\n");
    for(i = 0; i < n; i++){
      printf("> %ld - %d - %s\n", strlen(msg[i]), i,  msg[i]);
    }
    printf("------\n");
    //if(controlla_validita(msg,100)){
    if(true){
      if(msg[5][0] == '0'){
        char * str = malloc(sizeof(char) * 7);
        str[0] = msg[0][0];
        str[1] = '\0';
        strcat(str,msg[5]);
        str[6] = '\0';
        int codice = atoi(str);
        printf("=> %d\n", codice);
        switch (codice) {
          case 1001:
            print_info_fridge(msg);
            break;

          default: printf("Errore 3\n" );
            break;
        }
      }
    }
    else{
      printf("Errore 1\n" );
    }
  }
  printf("-----------Fine Centralina--------------\n\n");
}

void send_info_fridge(char * header, Frigo *frigo){ //invia info
  printf("Send info\n");
  char * str = (char * ) malloc(sizeof(char) * 100);

  int queue;
  msgbuf messaggio;

  create_queue(100,&queue); //da modificare chiave
  strcpy(str, header);
  send_prl_dati(str, frigo);
  strcpy(messaggio.msg_text, str);
  messaggio.msg_type = 1;
  msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
  printf("End send\n");
}

int main(){
  int queue;
  msgbuf messaggio;
  char * str = (char * ) malloc(sizeof(char) * 100);
  char * tmp = (char * ) malloc(sizeof(char) * 100);

  create_queue(1,&queue);
  send_prl_spec(str, "6", "0", "50", "1", "0", "10001", "1");
  printf("--------Centralina: invio richiesta----------\n");
  printf("%s", str);
  printf("--------Fine invio----------\n");
  strcpy(messaggio.msg_text, str);
  messaggio.msg_type = 1;
  msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);

  send_prl_spec(tmp, "6", "0", "50", "1", "0", "10006", "2");
  printf("\n--------Centralina: invio richiesta----------\n");
  printf("%s", tmp);
  printf("--------Fine invio----------\n");
  strcpy(messaggio.msg_text, tmp);
  messaggio.msg_type = 1;
  msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);

  if(fork()==0){
    fridge(1,0);
  }
  else{
    sleep(1);
    printf("\n\n\n\n\n\n");
    fridge(2,1);
  }
  return 0;
}

*/

void fridge(int id, int recupero){ //recupero booleano
  Frigo frigo;
  frigo.id = id;
  frigo.stato = FALSE;
  frigo.interruttore = FALSE;
  frigo.termostato = 3;// temperatura interna
  frigo.time = 0;
  frigo.nome = (char*)malloc(sizeof(char)*20);
  sprintf(frigo.nome, "FRIDGE-%d", id);
  frigo.delay = 5; //tempo di chiusura automatica
  frigo.percentuale = 0; // percentuale di riempimento
  int allarme = -1; // figlio per gestione alarme
  time_t t_start = 0; // tempo usato per calcolare eventuale tempo di apertura

  int queue;
  msgbuf messaggio;

  create_queue(id, &queue);
  printf("Id frigo: %d\n", frigo.id);

  if(recupero){
    printf("inizio\n" );
    if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
      printf("errore lettura ripristino\n");
    }
    else{
      printf("recupero\n");
      char **msg;
      int n=protocoll_parser(messaggio.msg_text, &msg);
      frigo.stato = atoi(msg[0]);
      frigo.interruttore = atoi(msg[1]);
      frigo.termostato = atoi(msg[2]);
      frigo.time = atoi(msg[3]);
      frigo.delay = atoi(msg[4]);
      frigo.percentuale = atoi(msg[5]);
      frigo.nome = msg[6];
      t_start = atoi(msg[7]);
      frigo.id = atoi(msg[8]);
      if(frigo.stato){ //se frigo è aperto
	       int new_delay = frigo.delay - frigo.time;
	        apri_frigo(&frigo, &t_start, &allarme, new_delay);
      }

      /*
      printf("Id frigo: %d\n", frigo.id);
      printf("stato: %d\n", frigo.stato);
      printf("Interruttore: %d\n", frigo.interruttore);
      printf("termostato: %d\n", frigo.termostato);
      printf("time: %d\n", frigo.time);
      printf("delay: %d\n", frigo.delay);
      printf("percentuale: %d\n", frigo.percentuale);
      printf("nome: %s\n", frigo.nome);
      */

      msgctl(queue, IPC_RMID, NULL); //rimuovo la coda precedentemente creata
      create_queue(frigo.id, &queue); //creo una nuova coda con id recuperato
    }
    printf("fine\n" );
  }

  //inizio loop
  while((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    char ** msg;
    char *header = (char*) malloc(sizeof(char) * 50);
    int n = protocoll_parser(messaggio.msg_text, &msg);
    /*int i;
    printf("------Header ricevuto\n");
    for(i = 0; i < n; i++){
      printf("> %ld - %d - %s\n", strlen(msg[i]), i,  msg[i]);
    }
    printf("------\n");*/
    if(controlla_validita(msg,frigo.id)){ // controllo se è per me o se è un messaggio in broadcast
      if(msg[5][0] == '0'){
      	header_risposta(msg, header);
      	char * str = malloc(sizeof(char) * 7);
      	str[0] = msg[0][0];
      	str[1] = '\0';
      	strcat(str,msg[5]);
      	str[6] = '\0';
      	int codice = atoi(str);
      	printf("=> codice1: %d\n", codice);
      	switch (codice) {
      	case 1: // 00001 : richiesta info
      	  if(t_start > 0){
      	    frigo.time = difftime(time(NULL), t_start);
      	  }
      	  send_info_fridge(header, &frigo);
      	  break;
      	case 2: // 00002 : richiesta kill
      	  if(allarme > 0){
      	    kill(allarme, SIGTERM);
      	  }
      	  exit(EXIT_SUCCESS);
      	  break;
      	default:
      	  printf("Errore codice istruzione!\n");
      	  break;
    	}
    }
    else if(msg[5][0] == '1'){
    	int valore = atoi(msg[6]); //valore da modificare, es. stato, temperatura
    	char * str = malloc(sizeof(char)*6);
    	strcpy(str, msg[5]);
    	str[6] = '\0';
    	int codice = atoi(str);
    	printf("=>codice2: %d\n", codice);
    	switch(codice){
    	case 10001: // modifica stato
    	  set_stato(valore, &frigo, &t_start, &allarme);
    	  printf("Stato: %d\n", frigo.stato);
    	  break;
    	case 10002:
    	  set_interruttore(valore, &frigo, &t_start, &allarme);
    	  printf("Interruttore: %d\n", frigo.interruttore);
    	  break;
    	case 10003:
    	  set_termostato(valore, &frigo);
    	  printf("Termostato: %d\n", frigo.termostato);
    	  break;
    	case 10004:
    	  set_delay(valore, &frigo);
    	  printf("Delay: %d\n", frigo.delay);
    	  break;
    	case 10005:
    	  set_perc(valore, &frigo);
    	  printf("Percentuale: %d\n", frigo.percentuale);
    	  break;
    	case 10006: // modalità recupero
    	  duplicate(valore, &frigo, t_start);
    	  if(allarme > 0){
    	    kill(allarme, SIGTERM);
    	  }
    	  exit(EXIT_SUCCESS);
    	  break;
    	default:
    	  printf("Errore codice istruzione!\n");
    	  break;
      }
    }
  }
 }
}


void apri_frigo(Frigo *frigo, time_t *t_start, int *allarme, int delay_recupero){
  frigo->stato = TRUE;
  if(!delay_recupero){ //se non sono in recupero
    time(t_start); //salvo tempo inizio apertura
  }

  if((*allarme = fork()) == 0){ //inizializzo un timer
    if(!delay_recupero){ //se non sono in recupero
      printf("Chiusura automatica fra %d secondi\n", frigo->delay);
      sleep(frigo->delay); //chiusura dopo [delay] secondi
    }
    else{ // altrimenti
      sleep(delay_recupero); // chiusura dopo [delay_recupero] secondi
    }

    int queue;
    msgbuf messaggio;
    char * str = (char * ) malloc(sizeof(char) * 100);
    char *id_frigo = (char*) malloc(sizeof(char)*10);
    sprintf(id_frigo, "%d", frigo->id);
    create_queue(frigo->id, &queue);
    send_prl_spec(str, "6", "0", "50", id_frigo, "0", "10001", "0");
    strcpy(messaggio.msg_text, str);
    messaggio.msg_type = 1;
    msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);

    exit(0);
  }
}

void chiudi_frigo(Frigo *frigo, time_t *t_start, int *allarme){
  frigo->stato = FALSE;
  *t_start = 0; //resetto tempo di inizio apertura
  frigo->time = 0; //resetto eventuale tempo di apertura

  if(*allarme >= 0){ //elimino eventuale timer
    kill(*allarme, SIGTERM);
    *allarme = -1;
  }
}

void set_stato(int valore, Frigo *frigo, time_t *t_start, int *allarme){ // valore: nuovo stato [1-apri], [0-chiudi]
  printf("----Setstato\n");
  if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    apri_frigo(frigo, t_start, allarme, 0);
    printf("Frigo aperto con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ // chiudo un frigo aperto
    chiudi_frigo(frigo, t_start, allarme);
    printf("Frigo chiuso con successo\n");
  }
}

void set_interruttore(int valore, Frigo *frigo, time_t *t_start, int *allarme){
   printf("----Set interruttore\n");
   if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    frigo->interruttore = TRUE;
    apri_frigo(frigo, t_start, allarme, 0);
    printf("Frigo aperto manualmente con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ //chiudo un frigo aperto
    frigo->interruttore = FALSE;
    chiudi_frigo(frigo, t_start, allarme);
    printf("Frigo chiuso munualmente con successo\n");
  }
}

void set_termostato(int valore, Frigo *frigo){
  printf("Cambio Temperatura interna\n");

  frigo->termostato = valore;

  printf("Temperatura combiata con successo\n");
}

void set_delay(int valore, Frigo *frigo){
  printf("Cambio delay\n");
  if(valore > 0){
    frigo->delay = valore;
  }
  else{
    printf("Errore: delay negativo\n");
  }

  printf("Delay cambiato con successo\n");
}

void set_perc(int valore, Frigo *frigo){
  int new_perc;
  new_perc = frigo->percentuale + valore;
  if((new_perc >= 0) && (new_perc <= 100)){
    printf("Percentuale di riempimento cambiata con successo\n");
    frigo->percentuale = new_perc;
  }
  else if(new_perc > 100){
    printf("Frigo troppo pieno\n");
  }
  else if(new_perc < 0){
    printf("Errore: percentuale di riempimento sotto zero\n");
  }
}

void duplicate(int valore, Frigo *frigo, time_t t_start){
  printf("---------Duplicate--------\n");
  int queue;
  msgbuf messaggio;
  char * str = (char * ) malloc(sizeof(char) * 100);
  char *tmp = (char*) malloc(sizeof(char)*20);

  sprintf(tmp, "%ld", t_start);
  if(t_start > 0){
    frigo->time = difftime(time(NULL), t_start);
  }

  send_prl_dati(str, frigo);
  strcat(str, tmp); //concateno t_start
  strcat(str, "\n");
  sprintf(tmp, "%d", frigo->id);
  strcat(str, tmp); //concateno id
  strcat(str, "\n\0");
  strcpy(messaggio.msg_text, str);
  create_queue(valore, &queue); //invio il messaggio al nuovo frigo duplicato con id tmp valore
  messaggio.msg_type = 10;
  msgsnd(queue, &messaggio, sizeof(messaggio.msg_text), 0);
  printf("-------Fine---------\n");
}



void header_risposta(char **msg, char *header){ //funzione che prende header della richiesta (msg) e restituisce header della risposta (header)
  printf("-------GetHeader--------------\n");

  char *tmp = (char *) malloc(sizeof(char)*20);
  for(int i=0; i<=5; i++){
    switch(i){
    case 0:
      strcpy(tmp, msg[1]);
      break;
    case 1:
      strcpy(tmp, "6");
      break;
    case 2:
      strcpy(tmp, msg[i]);
      break;
    case 3:
      strcpy(tmp, msg[4]);
      break;
    case 4:
      strcpy(tmp, msg[3]);
      break;
    case 5:
      strcpy(tmp, msg[i]);
      char *new = strdup(tmp);
      for(int i=0; i<sizeof(new); i++){
      	if(i==1){
      	  new[i] = '1'; //setto a 1 => risposta
      	}
      }
      strcpy(tmp, new);
      break;
    default:
      break;
    }
    strcat(header, plus_only_n(tmp));
  }
  strcat(header, "\0");

  printf("Header risposta: \n%s", header);
  printf("-------EndHeader---------------\n\n");
}

char * plus_only_n (char * a) {
  char * b = (char *) malloc (sizeof(char) *(strlen(a)+1));
  strcpy(b,a);
  b[strlen(a)] = '\n';

  return b;
}

void send_prl_dati(char* prl, Frigo *frigo){ //concateno dati da inviare
  char* buf = (char*) malloc(sizeof(char)*20);
  for(int i=0; i<7; i++){
    switch(i){
    case 0:
      sprintf(buf, "%d", frigo->stato);
      break;
    case 1:
      sprintf(buf, "%d", frigo->interruttore);
      break;
    case 2:
      sprintf(buf, "%d", frigo->termostato);
      break;
    case 3:
      sprintf(buf, "%d", frigo->time);
      break;
    case 4:
      sprintf(buf, "%d", frigo->delay);
      break;
    case 5:
      sprintf(buf, "%d", frigo->percentuale);
      break;
    case 6:
      strcpy(buf, frigo->nome);
      break;
    default:
      printf("Argomento non valido");
      break;
    }
    strcat(prl, plus_only_n(buf));
  }
  strcat(prl, "\0");
}

int controlla_validita(char ** str, int id){ // controllo se sono io il destinatario
   int rt = FALSE;
   if(atoi(str[0]) == FRIDGE || (atoi(str[0]) == DEFAULT && str[5][0] == '0')){
     if(atoi(str[3]) == id || atoi(str[3]) == 0){
       rt = TRUE;
       printf("k2\n" );
     }
   }
   return rt;
}
