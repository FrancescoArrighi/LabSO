#include "fridge.h"
//msg.type = 1 => ricevi
//msg.type = 2 => invio

// ---codice usato per prova--
/*
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
  create_queue(2,&queue);
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 2, 0)) == -1) {
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
      if(atoi(msg[MSG_OP]) <10000){
        int codice = codice_messaggio(msg);
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




int main(){
  int queue;
  msgbuf messaggio;
  char * str;

  create_queue(1,&queue);
  crea_messaggio_base(&str, FRIDGE, DEFAULT, 1, 2, FRIDGE_SET_STATO);
  strcat(str, "1\n\0");
  printf("--------Centralina: invio richiesta----------\n");
  printf("%s", str);
  printf("--------Fine invio----------\n");
  send_message(queue, &messaggio, str, 1);

  if(fork()==0){
    fridge(1,0); //RICEVE APRI FRIGO E RECUPERO
  }
  else{
    sleep(2);
    printf("\n\n\n\n\n\n");
    crea_messaggio_base(&str, FRIDGE, DEFAULT, 1, 2, FRIDGE_RECUPERO);
    strcat(str, "0\n\0");
    send_message(queue, &messaggio, str, 1);

    if(fork()==0){
      sleep(1);
      fridge(1, 1); //riceve recupero
    }
  }
  return 0;
}*/

void fridge(int id, int recupero){ //recupero booleano
  t_frigo frigo;
  frigo.id = id;
  frigo.stato = FALSE;
  frigo.interruttore = FALSE;
  frigo.termostato = 3;// temperatura interna
  frigo.time = 0;
  frigo.nome = (char * ) malloc( sizeof(char) * 20);
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
      int new_delay;
      char ** msg;
      protocoll_parser(messaggio.msg_text, &msg);
      // da aggiungere header
      frigo.stato = atoi(msg[MSG_FRIDGE_STATO]);
      frigo.interruttore = atoi(msg[MSG_FRIDGE_INTERRUTTORE]);
      frigo.termostato = atoi(msg[MSG_FRIDGE_TERM]);
      frigo.time = atoi(msg[MSG_FRIDGE_TIME]);
      frigo.delay = atoi(msg[MSG_FRIDGE_DELAY]);
      frigo.percentuale = atoi(msg[MSG_FRIDGE_PERC]);
      frigo.nome = msg[MSG_FRIDGE_NOME];
      t_start = atoi(msg[MSG_FRIDGE_TSTART]);
      frigo.id = atoi(msg[MSG_FRIDGE_ID]);
      if(frigo.stato){ //se frigo è aperto
	new_delay = frigo.delay - frigo.time; //recupero delay rimanente
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
      printf("new delay: %d\n", new_delay);
      */

      //msgctl(queue, IPC_RMID, NULL); //rimuovo la coda precedentemente creata
    }
    printf("fine\n" );
  }
  //inizio loop
  while((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    char ** msg;
    protocoll_parser(messaggio.msg_text, &msg);
    if(controlla_fridge(msg,frigo.id)){ // controllo se è per me o se è un messaggio in broadcast
      printf("%d\n",atoi(msg[MSG_OP]) );
      if(atoi(msg[MSG_OP]) < 10000){
      	int codice = codice_messaggio(msg);
      	printf("=> codice1: %d\n", codice);
      	switch (codice) {
      	case FRIDGE_INFO: // 00001 : richiesta info
      	  if(t_start > 0){
      	    frigo.time = difftime(time(NULL), t_start);
      	  }
      	  send_info_fridge(msg, &frigo);
      	  break;
      	case FRIDGE_KILL: // 00002 : richiesta kill
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
      else if(atoi(msg[MSG_OP]) > 10000){
    	int valore = atoi(msg[5]); //valore da modificare, es. stato, temperatura
    	int codice = codice_messaggio(msg);
    	printf("=>codice2: %d\n", codice);
    	switch(codice){
    	case FRIDGE_SET_STATO: // modifica stato
    	  set_stato(valore, &frigo, &t_start, &allarme);
    	  printf("Stato: %d\n", frigo.stato);
    	  break;
    	case FRIDGE_SET_INTERRUTTORE:
    	  set_interruttore(valore, &frigo, &t_start, &allarme);
    	  printf("Interruttore: %d\n", frigo.interruttore);
    	  break;
    	case FRIDGE_SET_TERMOSTATO:
    	  set_termostato(valore, &frigo);
    	  printf("Termostato: %d\n", frigo.termostato);
    	  break;
    	case FRIDGE_SET_DELAY:
    	  set_delay(valore, &frigo);
    	  printf("Delay: %d\n", frigo.delay);
    	  break;
    	case FRIDGE_SET_PERC:
    	  set_perc(valore, &frigo);
    	  printf("Percentuale: %d\n", frigo.percentuale);
    	  break;
    	case FRIDGE_RECUPERO: // modalità recupero
    	  duplicate(&frigo, t_start);
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

void send_info_fridge(char **msg, t_frigo *frigo){ //invia info
  printf("Send info\n");
  int queue;
  msgbuf messaggio;
  char *str;
  int cod_op;

  cod_op = codice_messaggio(msg) + 1000; //+1000 serve per specificare che si tratta di un messaggio di risposta
  crea_messaggio_base(&str, atoi(msg[MSG_TYPE_MITTENTE]), FRIDGE, atoi(msg[MSG_ID_MITTENTE]), frigo->id, cod_op);
  strcat(str, msg[MSG_INF_IDPADRE]); //concat id padre
  strcat(str, "\n\0");
  concat_dati(str, frigo); //concateno i dati del frigo

  printf("new msg: \n%s\n", str);
  create_queue(atoi(msg[MSG_ID_MITTENTE]), &queue); //stabilisco una coda col padre
  send_message(queue, &messaggio, str, 2);
  printf("End send\n");
}


void apri_frigo(t_frigo *frigo, time_t *t_start, int *allarme, int delay_recupero){
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
    char * id_frigo = (char * ) malloc(sizeof(char) * 10);
    sprintf(id_frigo, "%d", frigo->id);
    create_queue(frigo->id, &queue);
    //modifica
    crea_messaggio_base(&str, FRIDGE, FRIDGE, frigo->id, frigo->id, FRIDGE_SET_STATO); //invio un messaggio a se stesso...
    strcat(str, "0\n\0"); //...con stato FALSE
    send_message(queue, &messaggio, str, 1);
    exit(0);
  }
}

void chiudi_frigo(t_frigo *frigo, time_t *t_start, int *allarme){
  frigo->stato = FALSE;
  *t_start = 0; //resetto tempo di inizio apertura
  frigo->time = 0; //resetto eventuale tempo di apertura

  if(*allarme >= 0){ //elimino eventuale timer
    kill(*allarme, SIGTERM);
    *allarme = -1;
  }
}

void set_stato(int valore, t_frigo *frigo, time_t *t_start, int *allarme){ // valore: nuovo stato [1-apri], [0-chiudi]
  printf("----Setstato\n");
  if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    apri_frigo(frigo, t_start, allarme, 0);
    printf("fridge aperto con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ // chiudo un frigo aperto
    chiudi_frigo(frigo, t_start, allarme);
    printf("fridge chiuso con successo\n");
  }
}

void set_interruttore(int valore, t_frigo *frigo, time_t *t_start, int *allarme){
   printf("----Set interruttore\n");
   if((valore == TRUE) && (frigo->stato == FALSE)){ //apro un frigo chiuso
    frigo->interruttore = TRUE;
    apri_frigo(frigo, t_start, allarme, 0);
    printf("fridge aperto manualmente con successo\n");
  }
  else if((valore == FALSE) && (frigo->stato == TRUE)){ //chiudo un frigo aperto
    frigo->interruttore = FALSE;
    chiudi_frigo(frigo, t_start, allarme);
    printf("fridge chiuso munualmente con successo\n");
  }
}

void set_termostato(int valore, t_frigo *frigo){
  printf("Cambio Temperatura interna\n");

  frigo->termostato = valore;

  printf("Temperatura combiata con successo\n");
}

void set_delay(int valore, t_frigo *frigo){
  printf("Cambio delay\n");
  if(valore > 0){
    frigo->delay = valore;
  }
  else{
    printf("Errore: delay negativo\n");
  }

  printf("Delay cambiato con successo\n");
}

void set_perc(int valore, t_frigo *frigo){
  int new_perc;
  new_perc = frigo->percentuale + valore;
  if(!frigo->stato){
    printf("Apri prima il frigo!\n");
  }else if((new_perc >= 0) && (new_perc <= 100)){
    printf("Percentuale di riempimento cambiata con successo\n");
    frigo->percentuale = new_perc;
  }
  else if(new_perc > 100){
    printf("Errore: frigo troppo pieno\n");
  }
  else if(new_perc < 0){
    printf("Errore: percentuale di riempimento sotto zero\n");
  }
}

void duplicate(t_frigo *frigo, time_t t_start){
  printf("---------Duplicate--------\n");
  int queue;
  msgbuf messaggio;
  char * str = (char * ) malloc(sizeof(char) * 100);
  char *tmp = (char*) malloc(sizeof(char)*20);

  crea_messaggio_base(&str, 6, 6, frigo->id, frigo->id, 10006); //creo header
  sprintf(tmp, "%ld", t_start);
  if(t_start > 0){
    frigo->time = difftime(time(NULL), t_start);
  }
  concat_dati(str, frigo);
  strcat(str, tmp); //concateno t_start
  strcat(str, "\n");
  sprintf(tmp, "%d", frigo->id);
  strcat(str, tmp); //concateno id
  strcat(str, "\n\0");
  create_queue(frigo->id, &queue); //invio il messaggio al nuovo frigo duplicato con id tmp valore
  send_message(queue, &messaggio, str, 10);
  printf("-------Fine---------\n");
}

char * plus_only_n (char * a) {
  char * b = (char *) malloc (sizeof(char) *(strlen(a)+1));
  strcpy(b,a);
  b[strlen(a)] = '\n';

  return b;
}

void concat_dati(char* prl, t_frigo *frigo){ //concateno dati da inviar
  int i;
  char* buf = (char*) malloc(sizeof(char)*20);
  for(i=0; i<7; i++){
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

int controlla_fridge(char ** str, int id){ // controllo se sono io il destinatario
  int rt = FALSE;
  if(atoi(str[MSG_TYPE_DESTINATARIO]) == FRIDGE || (atoi(str[MSG_TYPE_DESTINATARIO]) == DEFAULT && atoi(str[MSG_OP]) < 10000)){
    if(atoi(str[MSG_ID_DESTINATARIO]) == id || atoi(str[MSG_ID_DESTINATARIO]) == 0){
      rt = TRUE;
    }
  }
  return rt;
}
