//Header
#include "msg_exchange.h"

// Permessi di accesso: come gestirli?
// Controllo coda vuota o piena ?


//Funzione inizializza la coda dato un id? e intero
//in cui viene salvato il puntatore alla coda
void create_queue(int id, int * queue){
  key_t key;
  if((key = ftok("/tmp/", id)) == -1){ // crea la chiave
      printf("Errore durante la creazione della chiave\n");
      exit(1);
  }
  if(((*queue) = msgget(key, IPC_CREAT)) == -1){ // crea il file se non esiste e salva fa puntare queue alla coda
      printf("Errore durante la ricezione dell'id della coda\n");
      exit(1);
  }
}

//Funzione che invia un messaggio alla coda identificata da queue
//e puntata dal buffer di dimensione strlen + 1 del text
//priorità
void send_message(int queue, msgbuf buffer, char * text) {
  strcpy(buffer.msg_text,text);
  buffer.msg_type = 1;
  size_t msg_size = strlen(text) + 1;

  if((msgsnd(queue, &buffer, msg_size, 0)) == -1){ // invio messaggio
                                                    // IPC_NOWAIT?
      printf("Errore nell'invio del messaggio\n");
      exit(1);
  }

}
