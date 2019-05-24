# LabSO2018-2019

1. Francesco Arrighi 194034
2. Wendy Briggit Flora Barreto Flores 192822
3. Guixia Zhu 193378

## Sintesi Progetto:

### Dispositivi di controllo

#### Controller:
#### Hub:
#### Timer:

### Dispositivi di interazione
Ognuno dei dispositivi terminali crea una `messagequeue` che gli permette di comunicare con i dispositivi di controllo e un sottoprocesso che gli permette di comunicare con l'umano.

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

L'umano può interagire con la lampadina nei seguenti modi:
- `open`: Controlla che la finestra non sia già aprta e nel caso sia chiusa l'apre e inizializza il tempo di inizio d'utilizzo a 0.
- `close`: Chiude la finestra.
- `get info`: Chiede le info e viene stampato l'id del dispositivo, lo stato della lampadina e il tempo di utilizzo.
- `get time`: Chiede il tempo di utilizzo alla finestra e viene stampato.

#### Fridge:
Il nostro frigerifero può essere aperto o chiuso con un singolo pulsante. Si richiude automaticamente dopo un tempo che può essere variato. Una “percentuale di riempimento” indica quanto contenuto c’è all’interno (0%-100%): è possibile togliere o aggiungere contenuto solo “manualmente”. Un termostato/termometro permette di gestire e impostare la temperatura interna.

La sua inizializzazione `fridge(int id, int recupero, char * nome)` avviene con un determinato id, un booleano che serve a un eventuale "Recupero stato del Frigorifero" e una stringa con il nome "Fridge".

L'umano può interagire con la lampadina nei seguenti modi:

- `set`: Controlla che la finestra non sia già aprta e nel caso sia chiusa l'apre e inizializza il tempo di inizio d'utilizzo a 0.
  - `close`: Chiude la finestra.
  - `get info`: Chiede le info.
  - `close`: Chiude la finestra.
  - `get info`: Chiede le info.

- `get`: Chiede il tempo di utilizzo alla finestra e viene stampato.
  - `close`: Chiude la finestra.
  - `get info`: Chiede le info.
  - `close`: Chiude la finestra.
  - `get info`: Chiede le info.



### Umano
I dispositivi per comunicare con l'umano utilizzano le fifo.
Ogni dispositivo ha un corrispettivo file `"D_id dispositivo_R"` e un corrispettivo file `"D_id dispositivo_W"`.

######`"D_id dispositivo_R"`
Serve al dispositivo per leggere gli input che il nostro `"Umano"` gli dà.

######`"D_id dispositivo_W"`
Serve al dispositivo per scrivere gli eventuali output che il nostro `"Umano"` dovrà ricevere.
