#include "fridge.h"

int flag;

void sighandle_flag1(int sig){
  flag = 1;
}

void sighandle_flag2(int sig){
  flag = 2;
}

//Da definire da un'altra parte
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
        if(str[i] == ' '){
            t = i+1;
        }
        else if(str[i] == '\0'){
          flag = FALSE;
        }
    }
    return j;
}



int main(){
  fridge(1, 1);
  return 0;
}




void fridge(int id, int recupero){ //recupero booleano
  int stato = FALSE;
  int interruttore = FALSE;
  int termostato = 3;// temperatura interna, range[0°-7°]
  time_t t_start = 0;
  char nome[] = "FRIDGE-ID";
  int idf[N_FIGLI];
  int delay = 5; //tempo di chiusura automatica
  int percentuale = 0; // percentuale di riempimento
  contenuto contenuto_frigo;
  int allarme = -1; // figlio per gestione alarme
  
  int queue;
  msgbuf messaggio;

  if((idf[0] = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [ON]  => SIGUSR1\n  [OFF] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        send_message(queue, &messaggio, "ON", 1);
        printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //spegni
        flag = 0;
        send_message(queue, &messaggio, "OFF", 1);
        printf("=> %s\n", messaggio.msg_text);
      }
    }
  }
  else if((idf[1] = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [getTime] => SIGUSR1\n  [getTemp/setTemp] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //accendi
        flag = 0;
        send_message(queue, &messaggio, "time", 1);
        printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //temperatura
        flag = 0;
        send_message(queue, &messaggio, "temp", 1);
      }
    }
  }
  else if((idf[2] = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [getDelay/setDelay] => SIGUSR1\n  [getPerc/setPerc] => SIGUSR2\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    signal(SIGUSR2, sighandle_flag2);
    while (true) {
      sleep(2);
      if(flag == 1){ //gestione delay
        flag = 0;
        send_message(queue, &messaggio, "delay", 1);
        printf("=> %s\n", messaggio.msg_text);
      }
      else if(flag == 2){ //gestione percentuale riempimento
        flag = 0;
        send_message(queue, &messaggio, "perc", 1);
      }
    }
  }
  else if((idf[3] = fork()) == 0){// codice figlio
    flag = 0;
    create_queue(id, &queue);
    printf("\n----------------\npid_signal: %d\n  [RIP] => SIGUSR1\n----------------\n\n", getpid());
    signal(SIGUSR1, sighandle_flag1);
    while (true) {
      sleep(2);
      if(flag == 1){ //kill processi figli
        flag = 0;
        send_message(queue, &messaggio, "RIP", 1);
        printf("=> %s\n", messaggio.msg_text);
      }
    }
  }

  create_queue(id, &queue);

  if(recupero){
    printf("inizio\n" );
    if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 10, 0)) == -1) {
      printf("errore lettura ripristino\n");
    }
    else{
      printf("recupero\n");
      stato = messaggio.msg_text[0]-'0';
      interruttore = messaggio.msg_text[1]-'0';
      termostato = messaggio.msg_text[2]-'0';
      percentuale = messaggio.msg_text[3]-'0';
      delay = messaggio.msg_text[4]-'0';
      char ** rt;
      str_split(messaggio.msg_text, &rt);
      t_start = atoi(rt[1]);
    }
    printf("fine\n" );
  }
  
  //inizio loop
  while ((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) != -1) {
    printf("\n * Livello: %ld\n", messaggio.msg_type );
    if(strcmp(messaggio.msg_text, "accendi") == 0){
      stato = TRUE;
      t_start = time(NULL);
      printf("Creazione figlio allarme\n");
      if((allarme = fork())==0){
	printf("Chiusura automatica fra %d secondi\n", delay);
	sleep(delay);
	send_message(queue, &messaggio, "spegni", 1);
	exit(0);
      }
      printf("accendi\n" );
    }
    else if(strcmp(messaggio.msg_text, "spegni") == 0){
      stato = FALSE;
      if(allarme >= 0){
	kill(allarme, SIGTERM);
	allarme = -1;
      }
      printf("spegni\n" );
    }
    else if(strcmp(messaggio.msg_text, "ON") == 0){
      interruttore = TRUE;
      if(!stato){
        stato = TRUE;
        t_start = time(NULL);
      }
      printf("Creazione figlio allarme\n");
      if((allarme = fork())==0){
	printf("Chiusura automatica fra %d secondi\n", delay);
	sleep(delay);
	send_message(queue, &messaggio, "spegni", 1);
	exit(0);
      }
      
      printf("ON\n" );
    }
    else if(strcmp(messaggio.msg_text, "OFF") == 0){
      interruttore = FALSE;
      if(stato){
        stato = FALSE;
      }
      if(allarme >= 0){
	kill(allarme, SIGTERM);
	allarme = -1;
      }
      printf("OFF\n" );
    }
    else if(strcmp(messaggio.msg_text, "time") == 0){
      if(stato == TRUE){
	printf("%2lf\n", difftime(time(NULL),t_start));
      }
      else{
        printf("Frigorifero chiuso!\n");
      }
      printf("time\n" );
    }
    else if(strcmp(messaggio.msg_text, "RIP") == 0){
      messaggio.msg_type = 10;
      messaggio.msg_text[0] = '0' + stato;
      messaggio.msg_text[1] = '0' + interruttore;
      messaggio.msg_text[2] = '0' + termostato;
      messaggio.msg_text[3] = '0' + percentuale;
      messaggio.msg_text[4] = '0' + delay;
      messaggio.msg_text[5] = ' ';
      messaggio.msg_text[6] = '\0';
      char str[20];
      sprintf(str, "%ld" , t_start);
      strcat(messaggio.msg_text, str);
      send_message(queue, &messaggio, messaggio.msg_text, 10);
      for(int i = 0; i < N_FIGLI; i++){
	kill(idf[i], SIGTERM);
      }
      exit(0);
    }
    else if(strcmp(messaggio.msg_text, "delay") == 0){
      char risposta;
      printf("Tempo chiusura automatica: %d\n", delay);
      printf("Vuoi cambiare il tempo? [Y/N]: ");
      scanf("%c", &risposta);

      if((risposta == 'Y') || (risposta == 'y')){ // se sì cambio delay
	printf("Inserire il tempo di chiusura automatica desiderata: ");
	scanf("%d", &delay);
      }
    }
    else if(strcmp(messaggio.msg_text, "temp") == 0){
      int temp;
      int flag_temp = FALSE; 
      char risposta;
      
      printf("Temperatura interna: %d°\n", termostato);
      printf("Vuoi cambiare la temperatura? [Y/N]: ");
      scanf("%c", &risposta);

      if((risposta == 'Y') || (risposta == 'y')){ // se sì cambio la temperatura
	do{
	  printf("Nuova temperatura (min 0° e max 7°): ");
	  scanf("%d", &temp);
	  if((temp>=0)&&(temp<=7)){
	    termostato = temp;
	    flag_temp = TRUE;
	  }
	  else{
	    printf("Temperatura fuori range! Si consiglia di inserire un valore tra 0 e 7.\n");
	  }
	}while(!flag_temp);
      }
      
    }
    else if(strcmp(messaggio.msg_text, "perc") == 0){
      int azione = 0;

      while(azione!=3){
	printf("Percentuale di riempimento: %d\n", percentuale);
	printf("Alimenti: %d\n", contenuto_frigo.quantita_alimenti);
	printf("Bevande: %d\n", contenuto_frigo.quantita_bevande);
	printf("Azione possibile (inserire il numero corrispondente)\n");
	if(percentuale < 100){ // se il frigo non è pieno
	  printf("1) Aggiungere\n");
	}
	if(percentuale > 0){ // se il frigo non è vuoto
	  printf("2) Togliere\n");
	}
	printf("3) Uscire\n");
	printf(">> ");
	scanf("%d", &azione);
	switch(azione){
	case 1:
	  aggiungi_contenuto(&percentuale, &contenuto_frigo);
	  break;
	case 2:
	  togli_contenuto(&percentuale, &contenuto_frigo);
	  break;
	case 3:
	  printf("Fine azione\n");
	  break;
	default:
	  printf("Azione non valida!\n");
	}
      }
      
    }
    printf("\n\ninterruttore: %d\n", interruttore);
    printf("stato: %d\n", stato);
    printf("time: %ld\n", t_start);
    printf("temp: %d\n", termostato);
    printf("delay: %d\n", delay);
    printf("perc: %d\n", percentuale);
  }
  printf("Errore lettura queue FRIDGE\n");
}

void aggiungi_contenuto(int *perc, contenuto *contenuto_frigo){
  int volume_bevande = 5; // volume bevande in percentuale
  int volume_alimenti = 5;
  int volume;
  int tipologia = 0;
  int quantita = 0;
  int new_perc = *perc;
  int flag_uscita = FALSE;

  while(!flag_uscita){
    printf("Scegliere la tipologia desiderata (inserire il numero corrispondente)\n");
    printf("1) Alimenti\n");
    printf("2) Bevande\n");
    printf(">> ");
    scanf("%d", &tipologia);
    if(tipologia != 1 && tipologia != 2){
      printf("Tipologia scelta non valida!\n");
    }
    else{
      printf("Inserire la quantità: ");
      scanf("%d", &quantita);
      if(tipologia == 1){
        new_perc += quantita * volume_alimenti;
      }else{
        new_perc += quantita * volume_bevande;
      }
      
      if(new_perc > 100){
	printf("Aggiungi meno cose!\n");
      }
      else{
	printf("Aggiunto con successo!\n");
	if(tipologia == 1){
	  contenuto_frigo->quantita_alimenti += quantita;
	}
	else if(tipologia == 2){
	  contenuto_frigo->quantita_bevande += quantita;
	}
	*perc = new_perc;
	flag_uscita = TRUE;
      }
    }
  }
}

void togli_contenuto(int *perc, contenuto *contenuto_frigo){
  int volume_bevande = 5; // volume bevande in percentuale
  int volume_alimenti = 5;
  int tipologia = 0;
  int quantita = 0;
  int new_perc = *perc;
  int flag_uscita = FALSE;

  while(!flag_uscita){
    printf("Scegliere la tipologia desiderata (inserire il numero corrispondente)\n");
    printf("1) Alimenti\n");
    printf("2) Bevande\n");
    printf(">> ");
    scanf("%d", &tipologia);
    if(tipologia != 1 && tipologia != 2){
      printf("Tipologia scelta non valida!\n");
    }
    else{
      printf("Inserire la quantità: ");
      scanf("%d", &quantita);
      if(tipologia == 1){
	new_perc -= quantita * volume_alimenti;
      }
      else{
	new_perc -= quantita * volume_bevande;
      }
      
      if((tipologia == 1 && quantita > contenuto_frigo->quantita_alimenti) || (tipologia == 2 && quantita > contenuto_frigo->quantita_bevande) || (new_perc < 0)){
	printf("La quantità che vuoi togliere è maggiore della quantità contenuta!\n");
      }
      else{
	printf("Tolto con successo!\n");
	if(tipologia == 1){
	  contenuto_frigo->quantita_alimenti -= quantita;
	}
	else if(tipologia == 2){
	  contenuto_frigo->quantita_bevande -= quantita;
	}
	*perc = new_perc;
	flag_uscita = TRUE;
      }
    }
  }
}


