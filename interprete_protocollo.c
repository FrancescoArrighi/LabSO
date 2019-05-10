void bulb_info_r(char ** str){
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
    printf("Timer: %lld\n", string_int(str[9]));
    printf("Nome: %s\n", str[11]);

}

 int controlla_validita(char ** str, long long int id){
   int rt = FALSE;
   if(string_int(msg[0]) == CONTROLLER || (string_int(msg[0]) == DEFAULT && msg[5][0] == '0'){
     if(string_int(str[3]) == id || string_int(str[3]) == 0){
       rt = TRUE;
     }
   }
   return rt;
}

void risposta(){
  int queue;
  msgbuf messaggio;
  crea_queue(100,&queue);
  if((msgrcv(queue, &messaggio ,sizeof(messaggio.msg_text), 1, 0)) == -1) {
     printf("Errore\n" );
  }
  else{
    char ** msg;
    int n = protocoll_parser(messaggio.msg_text, &msg);
    if(controlla_validita(msg,100)){
      if(msg[5][0] != '0'){
        char * str = malloc(sizeof(char) * 7);
        str[0] = msg[0][0];
        str[1] = '\0';
        strcat(str,msg[5]);
        str[7] = '\0';
        long long int codice = string_int(str);
        switch (codice) {
          case 2048384:
            bulb_info_r(rt);
            break;

          default:
            break;
        }
      }
    }
    else{
      //non posso interpretarlo
    }
  }
}
