# LabSO2018-2019

1. Francesco Arrighi 194034
2. Wendy Briggit Flora Barreto Flores 192822
3. Guixia Zhu 193378

## Sintesi Progetto:
Ognuno dei dispositivi può comunicare con l'esterno utilizzando una `messagequeue`,la cui chiave viene generata usando il proprio id e lo scambio di messaggi avviene con un protocollo specifico e una specifica priorità. Ad esempio i dispositivi di controllo comunicano con i dispositivi terminali con messaggi di priorità 4 e riceve dai dispositivi controllati messaggi di priorità 2. In questo modo abbiamo simulato una gerarchia di messaggi per le code di ogni dispositivo.

Il nostro protocollo prevede dei campi fissi che sono :

- `MSG_TYPE_DESTINATARIO` alla posizione 0
- `MSG_TYPE_MITTENTE` alla posizione 1
- `MSG_ID_DESTINATARIO` alla posizione 2
- `MSG_ID_MITTENTE` alla posizione 3
- `MSG_OP` alla posizione 4

Il campo `MSG_OP` corrisponde al tipo d'operazione che vogliamo eseguire o che venga eseguita.

I messaggi mandati dai dispositivi di controllo attraversano tutto l'albero delle gerarchie e solo i dispositivi direttamenti interessati rispondono con un ack postitivo nel campo `MSG_OP`.

Per la creazione di questo protocollo ci siamo ispirati ai protocolli utilizzati nelle reti.

### Dispositivi di controllo
I dispositivi di controllo mandano in broadcast a tutte le `messagequeue` dei propri figli, attraverso l'uso di ACK negativi e positivi i dispositivi di controllo gestiscono la ricezione dei messaggi.

Ad esempio se ho un hub "10" con 2 figli hub e uno di loro ha sotto di lui altri 3 figli, un messaggio indirizzato all'hub "10" prima raggiunge l'hub attraversando l'intero albero delle gerarchie e viene individuato perchè è l'unico che risponde con un ACK positivo. Dopo essere stato individuato esso genera due messaggi per i due figli, e si aspetta conseguentemente 2 risposte. Uno dei figli risponde con un semplice ACK positivo quindi il contatore di risposte dell' hub "10" diminuisce ad 1, l'altro hub sotto l'hub "10" risponde informandolo del numero dei suoi figli = 3. Il contatore delle risposte aumenta dunque a 4 e non diminuirà finchè tutti i figli dell'ultimo hub non abbiano mandato un ACK di risposta all'hub "10".

#### Controller:
#### Hub:
#### Timer:

### Dispositivi di interazione
I dispositivi terminali hanno un sotttoprocesso che permette la comunicazione diretta con l'umano.

#### Bulb:
La nostra lampadina può essere accesa o spenta e ha un singolo interruttore di comando per il cambio di stato.

La sua inizializzazione `bulb(int id, int recupero, char * nome)` avviene con un determinato id, un booleano che serve a un eventuale "Recupero stato della Lampadina" e una stringa con il nome "Bulb".

Ogni volta che lo stato della lampadina viene invertito da OFF/ON il tempo di inizio viene inizializzato a 0 in modo da poter sempre calcolare il tempo di utilizzo giusto.

L'umano può interagire con la lampadina nei seguenti modi:
- `interruttore`: Inverte lo stato dell'interruttore a seconda che sia su on/off ed eventualmente fa override.
- `get info`: Chiede le info e viene stampato l'id del dispositivo, lo stato della lampadina, lo stato dell'interruttore e il tempo di utilizzo.
- `get time`: Chiede il tempo di utilizzo alla lampadina e viene stampato.

#### Window:
La nostra finestra può essere aperta o chiusa con due interruttori uno per l'apertura ed uno per la chiusura. Questi interruttori dopo essere stati azionati tornano subito in OFF per questo nelle info non gli mostriamo in quanto sono sempre allo stato OFF.

La sua inizializzazione `window(int id, int recupero, char * nome)` avviene con un determinato id, un booleano che serve a un eventuale "Recupero stato della Finestra" e una stringa con il nome "Window".

L'umano può interagire con la finestra nei seguenti modi:
- `open`: Controlla che la finestra non sia già aprta e nel caso sia chiusa l'apre e inizializza il tempo di inizio d'utilizzo a 0.
- `close`: Chiude la finestra.
- `get info`: Chiede le info e viene stampato l'id del dispositivo, lo stato della lampadina e il tempo di utilizzo.
- `get time`: Chiede il tempo di utilizzo alla finestra e viene stampato.

#### Fridge:
Il nostro frigorifero può essere aperto o chiuso con un singolo pulsante. Si richiude automaticamente dopo un tempo che può essere variato. Una “percentuale di riempimento” indica quanto contenuto c’è all’interno (0%-100%): è possibile togliere o aggiungere contenuto solo “manualmente”. Un termostato/termometro permette di gestire e impostare la temperatura interna (0°-7°).

La sua inizializzazione `fridge(int id, int recupero, char * nome)` avviene con un determinato id, un booleano che serve a un eventuale "Recupero stato del Frigorifero" e una stringa con il nome "Fridge".

L'umano può interagire con il frigorifero nei seguenti modi:

- `set`: il quarto parametro è il valore da impostare
  - `interruttore`: con valore 1 viene aperto il frigo, mentre 0 chiude il frigo.
  - `delay`: viene settato delay al valore.
  - `percentuale`: viene settato percentuale al valore.
  - `termostato`: viene settato termostato al valore.

- `get`:
  - `info`: viene ritornato informazioni del frigo.
  - `time`: viene ritornato il tempo di apertura se il frigo è aperto, altrimenti 0.
  - `stato`: viene ritornato lo stato del frigo, 1 se aperto, 0 altrimenti.
  - `delay`: viene ritornato il valore di delay.
  - `percentuale`: viene ritornata la percentuale di riempimento.
  - `termostato`: viene ritornata la temperatura interna.

### Umano
I dispositivi per comunicare con l'umano utilizzano le fifo.
Ogni dispositivo ha un corrispettivo file `"D_id dispositivo_R"` e un corrispettivo file `"D_id dispositivo_W"`.

`"D_id dispositivo_R"`
Serve al dispositivo per leggere gli input che il nostro `"Umano"` gli dà.

`"D_id dispositivo_W"`
Serve al dispositivo per scrivere gli eventuali output che il nostro `"Umano"` dovrà ricevere.
