/*
Sistema di prenotazione posti

Realizzazione di un servizio di prenotazione posti per una sala cinematografica, supportato da un server.

Ciascun posto e' caratterizzato da un numero di fila, un numero di poltrona, e puo' essere libero o occupato.  Il server accetta e processa sequenzialmente o in concorrenza (a scelta) le richieste di prenotazione di posti dei client (residenti, in generale, su macchine diverse).

Un client deve fornire ad un utente le seguenti funzioni:
1. Visualizzare la mappa dei posti in modo da individuare quelli ancora disponibili.
2. Inviare al server l'elenco dei posti che si intende prenotare (ciascun posto da prenotare viene ancora identificato tramite numero di fila e numero di poltrona).
3. Attendere dal server la conferma di effettuata prenotazione ed un codice univoco di prenotazione.
4. Disdire una prenotazione per cui si possiede un codice.

Si precisa che la specifica richiede la realizzazione sia dell'applicazione client che di quella server.

Per progetti misti Unix/Windows e' a scelta quale delle due applicazioni sviluppare per uno dei due sistemi.
*/

#define cres  "\x1B[0m"
#define cros  "\x1B[31m"
#define cver  "\x1B[32m"
#define cgia  "\x1B[33m"
#define cblu  "\x1B[34m"
#define cmag  "\x1B[35m"
#define ccia  "\x1B[36m"
#define cbia  "\x1B[37m"

#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#define MAX_COL 3000
#define MAX_ROW 3000
#define MAX_SEA 9000000

void prinf(const char * message){ printf(cver "\n[INFO]\t%s" cbia, message); }
void prerr(const char * message){ printf(cros "\n[ERR!]\t%s" cbia, message); }
void prwar(const char * message){ printf(cgia "\n[WARN]\t%s" cbia, message); }
void eonerror(const char * message)
{
	printf(cros "\n[ERR!]\tFatal Error!" cbia);
	perror(message);
	exit(EXIT_FAILURE);
}

typedef struct reservation
{
		int * rseat;
		int index;
		struct reservation * prevr;
		struct reservation * nextr;
} reservation;

typedef struct cinema
{
		int ** seat;
		struct reservation * firstr;
		int indexes[MAX_SEA];
} cinema;

struct reservation * openres(struct reservation * prevres, struct reservation * nextres, struct cinema * cinstr)
{
	struct reservation * res;
	int indx = - 1;
	int ind;
	if ((res = malloc(sizeof(struct reservation))) == NULL) eonerror("malloc reservation");
	for(ind = 0; ind < MAX_SEA; ind++)
	{
		if (cinstr->indexes[ind] == 0)
		{
			cinstr->indexes[ind] = 1
			indx = ind;
			break;
		}
	}
	if (indx == -1) 
	{
		free(res);
		prerr("Impossible to create reservations anymore!");
		return NULL;
	}
	res->index = indx;
	res->nextr = nextres;
	res->prevr = prevres;
	return res;
}

void serveclient(int fd)
{
	prinf("K");
	if (close(fd) == -1) eonerror("closing socket");
}

int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, 0);

	//Getting terminal size
    /*struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
	int trow = w.ws_row;
	int tcol = w.ws_col;
	*/
	socklen_t sockstr;
	int listsock;
	struct sockaddr_in servaddr;	
	struct sockaddr_in clientaddr;
	int port;
	int backlog = 0;
	int socket_fd;
	
	//client_t_struct *tmp;	
	//pthread_t tid;
	
	//char dialogq;
	char * chtemp;
	
	int crow;
	int ccol;
	
	prinf("C-nema server loading!");
		
	if (argc > 2) 
	{
		prerr("Usage: ./server <port>\t Default port if not specified: 4321");
		eonerror("Too many CLI arguments");
	}
	
	/* Setting input port. From man strtol:
	 *
	 * EINVAL (not in C99) The given base contains an unsupported value.
     * ERANGE The resulting value was out of range.
     * The implementation may also set errno to EINVAL in case no conversion
     * was performed (no digits seen, and 0 returned).
     * 
     * Valid TCP port range: 0 < port < 65536 (and 1-1024 could still fail if not run by root) 
	 */
	port = 4321;
	
	if (argc == 2)
	{	
		port = strtol(argv[1], NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) eonerror("Invalid port");
		if ((port < 0) || (port >= 65536)) eonerror("Port out of range. Valid range: 1 - 65535 (1-1024 only as root)");
		if (port == 0) port = 4321;
	}
	prinf("Port set to value ");
	printf(cmag "%d\n", port);

	// Checking for saved file
	/*
	int datfile;
	if( access( "./cnema.dat" , F_OK|R_OK ) != -1 ) 
	{
		prinf("A previous booking set exists.");
		printf(cbia "\n\t\tWould you like to use it? [Yn] " cres);
		scanf("%c", &dialogq);
		if ((dialogq == 'N')||(dialog1 == 'n')): 
	}
	else 
	{
		prinf("No usable booking sets detected. Creating a new one");
		printf(cbia "\n\t\tWould you like to use it? [Yn] " cres);
   	}
   	*/
   	
   	if ((chtemp = malloc(sizeof(char) * 8)) == NULL) eonerror("malloc");
   	
   //	prinf("Stage 1 ok");
	ccol = crow = 30;
	while (1 > 0)
   	{
		printf(cbia "\tInsert column size: ");
		if ((fgets(chtemp, 7, stdin)) == NULL) break;
		//scanf("%s", chtemp);
		ccol = strtol(chtemp, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid row size");
		else if (ccol > 0) break;
	}

   	while (1 > 0)
   	{
		printf("\tInsert row size: ");
		//scanf("%s", chtemp);
		if ((fgets(chtemp, 7, stdin)) == NULL) break;
		crow = strtol(chtemp, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid row size");
		else if (crow > 0) break;
	}
	free(chtemp);
   	prinf("Creating new room with size ");
   	printf(ccia "%dx%d", ccol, crow);
   	
	/* setting the listening socket */
	listsock = socket(AF_INET, SOCK_STREAM, 0);
	if (listsock < 0) eonerror("listening socket creation");
	
	/* setting the socket address data structure */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servaddr.sin_port			= htons(port);

	/* bind socket address to the listening socket */
    if (bind(listsock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) eonerror("bind error");

	/* mark the socket as a listening socket */
	if (listen(listsock, backlog) < 0) eonerror("listen error");

	/* initializing the chanserv struct for managing requests */
	//chanserv = alloc_cstruct();
	//printf("[x]\tchanserver struct allocated - address is %p\n", (void *)chanserv);

	/* starting infinite loop for responding client requests */
	while (1 > 0)
	{
		prinf("Awaiting connections");

		//tmp = alloc_tstruct();
		sockstr = sizeof(struct sockaddr);

		socket_fd = accept(listsock, (struct sockaddr *)&clientaddr, &sockstr);
		//if (tmp->fd < 0) eonerror("accept");
		//pthread_create(&tid, NULL, chanserv_function, (void *)tmp);
		prinf("Connection incoming!");
		serveclient(socket_fd);
		prinf("Connection closed!");
	}
	return 0;		
}
