# Moonloop
Multi-channel-chat client/server application for *NIX
## Specifica
Realizzazione di un servizio "chat" via internet offerto tramite server 
che gestisce un insieme di processi client (residenti, in generale, su
macchine diverse). Il server deve essere in grado di acquisire in input 
da ogni client una linea alla volta e inviarla a tutti gli altri client
attualmente presenti nella chat.
Ogni client potrà decidere se creare un nuovo canale di conversazione, 
effettuare il join ad un canale già esistente, lasciare il canale, 
o chiudere il canale (solo nel caso che l'abbia istanziato). 
