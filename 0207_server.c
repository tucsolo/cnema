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
#define MAX_COL 100
#define MAX_ROW 100
#define MAX_SEA 9000000

void prinf(const char * message){ printf(cver "\n[INFO]\t%s" cbia, message); }
void prsoc(const char * message, int fd){ printf(cver "\n[SOCK]\t%d says <%s>" cbia, fd, message); }
void prerr(const char * message){ printf(cros "\n[ERR!]\t%s" cbia, message); }
void prwar(const char * message){ printf(cgia "\n[WARN]\t%s" cbia, message); }
void eonerror(const char * message)
{
	printf(cros "\n[ERR!]\tFatal Error!" cbia);
	perror(message);
	exit(EXIT_FAILURE);
}
/*
typedef struct reservation
{
		unsigned int * rseat;		//	reservations array
		unsigned int index;			//	this reservation index
		struct reservation * prevr;	//	previous reservation [def:NULL]
		struct reservation * nextr;	//	next reservation 	 [def:NULL]
} reservation;
*/
typedef struct cinema
{
		unsigned int rows;
		unsigned int cols;
		unsigned int * seat;
		//struct reservation * firstr;
		unsigned int * indexes;
} cinema;

struct cinema * initcinema(unsigned int cols, unsigned int rows)
{
	if ((rows > MAX_ROW)||(cols > MAX_COL)||(rows * cols > MAX_SEA)) eonerror("cinema size exceeded, srsly dunno how");
	struct cinema * res;
	if ((res = malloc(sizeof(struct cinema))) == NULL) eonerror("malloc cinema struct");
	//unsigned int * seats = calloc((cols * rows), sizeof(unsigned int));
	//unsigned int * index = calloc((cols * rows + 1), sizeof(unsigned int));
	unsigned int * seats = malloc((cols * rows) * sizeof(unsigned int));
	unsigned int * index = malloc((cols * rows + 1) * sizeof(unsigned int));
	/*
	 * using calloc as it sets memory to zero.
	 * seat = row * columns + column
	 * 
	 * 
	 * 		-> 	indexes are cols * rows + 1 so
	 * 			seat[x] = 0 means that seat it's free
	 * 			seat[x] = n means that seat it's reserved to
	 * 						reservation n. Could have spared that one
	 * 						integer but then I should have used -1 or
	 * 						MAX_COL * MAX_SEA + 1 as free seat value.
	 *						If you have to optimize this program by
	 * 						memory usage here you may find some bits
	 *
	 */
	if (seats == NULL) eonerror("malloc cinema seats array");
	if (index  == NULL) eonerror("malloc cinema index array");
	res->rows = rows;
	res->cols = cols;
	res->seat = seats;
	res->indexes = index;
	//res->firstr = NULL;
	//res->seat[0] = 1;
	//res->seat[2*res->rowsize] = 1;
	//res->seat[2] = 1;
	//res->seat[32*res->rowsize + 2] = 1;
	//res->seat[64*res->rowsize + 6] = 0;
	return res;
}
/*	
struct reservation * openres(struct reservation * prevres, struct cinema * cinstr)
{
	struct reservation * res;
	int indx = - 1;
	int ind;
	if ((res = malloc(sizeof(struct reservation))) == NULL) eonerror("malloc on a reservation struct");
	for(ind = 1; ind <= MAX_SEA; ind++)
	{
		if (cinstr->indexes[ind] == 0)
		{
			cinstr->indexes[ind] = 1;
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
	res->nextr = NULL;
	res->prevr = prevres;
	if (prevres == NULL) cinstr->firstr = res;
	return res;
}
*/
void printcinema(struct cinema * room, unsigned int res_id)
{
	printf(cbia"\n\tCinema status");
	if (res_id != 0) printf(" for reservation id "cver "%d\n\t\t*\tReserved by you" cbia "\n\t\t_\tFree" cros "\n\t\t#\tReserved by someone else\n", res_id);
	else printf(cbia "\n\t\t_\tFree" cros "\n\t\t#\tReserved\n");
	printf(ccia "\n\t\t          ");
	unsigned int i;
	for (i = 10; i< room->cols; i++) printf("%d", i/10);
	printf(ccia "\n\t\t");
	for (i = 0; i< room->cols; i++) printf("%d", i % 10);
	unsigned int j, seat;
	for (i = 0; i < room->rows; i++)
	{
		printf(cmag "\n\t%d\t", i);
		for (j = 0; j < room->cols; j++)
		{
			seat = room->seat[i*room->cols + j];
			if (seat == 0) printf(cbia "_");
			else if (seat == res_id) printf(cver "*");
			else printf(cros "#");
		}
	}
	
	//printf(cbia "\nr32\tc2\t%d", room->seat[32*room->rowsize + 2]); 
}

void lindex(char * message, struct cinema * room)
{
	int val = strtol(message + 1, NULL, 10);
	printcinema(room, val);
}
void reserve(char * message, struct cinema * room)
{
	/*
	int index, i = 0, pos = 1;
	unsigned int * rlist = malloc(sizeof(int));
	rlist[0] = i;
	if (rlist == NULL) eonerror("malloc reserving a seat, array");
	char * lastchar;
	char * thischar;
	index = strtol(message + 1, &thischar, 10);
	lastchar = thischar;
	printf("\nReservating index %d, %s", index, lastchar);
	while (1 > 0)
	{
	if (lastchar[0] == '.') break;
	i++;
	rlist[0] = i;
	rlist = realloc(rlist, sizeof(int) * i * 2 + 1);
	rlist[2*i - 1] = strtol(lastchar + 1, &thischar, 10);
	rlist[2*i] = strtol(thischar + 1, &lastchar, 10);
	//printf("\n%d %d #%s#", rlist[2*i-1], rlist[2*i], lastchar);
	if (lastchar == NULL) break;
	}
	//for (i = 0; i < rlist[0]; i++) printf("\n%d:%d", rlist[(i+1)*2-1], rlist[(i+1)*2]);
	prinf("Seats reserved with reservation index "); 
	printf(cmag "%d" cbia, index);
	*/
	int index;
	int row, col;
	char * lastchar;
	char * thischar;
	index = strtol(message + 1, &thischar, 10);
	lastchar = thischar;
	//printf("\nCreating reservation #%d", index);
	room->indexes[index] = 1;
	while (1 > 0)
	{
		if (lastchar[0] == '.') break;
		row = strtol(lastchar + 1, &thischar, 10);
		col = strtol(thischar + 1, &lastchar, 10);
		if (room->seat[row * room->cols + col] == 0) room->seat[row * room->cols + col] = index;
		
	}
	//for (i = 0; i < rlist[0]; i++) printf("\n%d:%d", rlist[(i+1)*2-1], rlist[(i+1)*2]);
	prinf("Seats reserved to reservation # "); 
	printf(cmag "%d" cbia, index);
}
void fill(char * message, struct cinema * room)
{
	unsigned int index, places, i = 0;
	char * thischar;
	index = strtol(message + 1, &thischar, 10);
	places = strtol(thischar + 1, &thischar, 10);
	//printf("\nCreating reservation #%d", index);
	room->indexes[index] = 1;
	while (places > 0)
	{
		if (room->seat[i] == 0) 
		{
			room->seat[i]= index;
			places--;
		}
		i++;		
		if (i == room->rows*room->cols) break;
	}
	//for (i = 0; i < rlist[0]; i++) printf("\n%d:%d", rlist[(i+1)*2-1], rlist[(i+1)*2]);
	prinf("Seats reserved to reservation # "); 
	printf(cmag "%d" cbia, index);
	if (places != 0) prwar("Cinema is full!");
}
void cancels(char * message, struct cinema * room)
{
	unsigned int index,row,col;
	char * lastchar;
	char * thischar;
	index = strtol(message + 1, &thischar, 10);
	lastchar = thischar;
	//printf("\Editing reservation #%d", index, lastchar);
	room->indexes[index] = 1;
	while (1 > 0)
	{
		if (lastchar[0] == '.') break;
		row = strtol(lastchar + 1, &thischar, 10);
		col = strtol(thischar + 1, &lastchar, 10);
		if (room->seat[row * room->cols + col] == index) room->seat[row * room->cols + col]= 0;
	}
	//for (i = 0; i < rlist[0]; i++) printf("\n%d:%d", rlist[(i+1)*2-1], rlist[(i+1)*2]);
	prinf("Seats freed from reservation # "); 
	printf(cmag "%d" cbia, index);
}
void cancela(char * message, struct cinema * room)
{
	unsigned int i, index;
	char * thischar;
	index = strtol(message + 1, &thischar, 10);
	//printf("\Deleting reservation #%d", index);
	room->indexes[index] = 0;
	for (i = 0; i < room->cols*room->rows; i++) if (room->seat[i] == index) room->seat[i] = 0;
	prinf("Deleted reservation # "); 
	printf(cmag "%d" cbia, index);
}
int recv_line(int sockfd, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n')) 
	{
		n = recv(sockfd, &c, 1, 0);
		if (n > 0) 
		{
			buf[i] = c;
			i++;
		}
		else c = '\n';
	}
	buf[i] = '\0';
	return i;
}
void serveclient(int fd, struct cinema * room)
{
	prinf("K");
	char tempbuf[512];
	while (recv_line(fd, tempbuf, sizeof(tempbuf)) != 0) 
	{
		/*
		 * Commands
		 * 
		 * lI				prints cinema reservations for index I
		 * rI,R,C,R,[...].	reserves seats Row.Column [...] to Index
		 * fI,N				reserves N seats for index I
		 * cI,R,C,R,[...].  cancels reservation for seats R.C to Index
		 * dI				cancels entire reservation Index
		 */
		  
		prsoc("%s", fd);
		if (tempbuf[0] == 'l') lindex(tempbuf, room);
		if (tempbuf[0] == 'r') reserve(tempbuf, room);
		if (tempbuf[0] == 'f') fill(tempbuf, room);
		if (tempbuf[0] == 'c') cancels(tempbuf, room);
		if (tempbuf[0] == 'd') cancela(tempbuf, room);
	}
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
	
	unsigned int crow;
	unsigned int ccol;
	struct cinema * room;
	
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
	printf(ccia "%d\n", port);

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
   	
   	ccol = crow = 30;
   	while (1 > 0)
   	{
		printf(cbia "\tInsert column size\t[1 - %d]: ", MAX_COL);
		if ((fgets(chtemp, 7, stdin)) == NULL) break;
		ccol = strtol(chtemp, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid row size");
		else if ((ccol > 0)&&(ccol <= MAX_COL)) break;
	}

   	while (1 > 0)
   	{
		printf("\tInsert row size\t\t[1 - %d]: ", MAX_ROW);
		if ((fgets(chtemp, 7, stdin)) == NULL) break;
		crow = strtol(chtemp, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid row size");
		else if ((crow > 0)&&(ccol <= MAX_ROW)) break;
	}

	free(chtemp);
   	prinf("Creating new room with size ");
   	printf(ccia "%dx%d", ccol, crow);

   	room = initcinema(ccol, crow);

	prinf("Room with size ");
   	printf(ccia "%dx%d" cver " created!", room->cols, room->rows);
   	
   	printcinema(room, 0);
   	/*int xx;
   	for (xx = 3; xx< 280; xx++)
   	{
		room = initcinema(32, xx);
		printf(cbia "\n32x%d\tr32\tc2\t%d %d %d",xx, room->seat[32*room->cols + 1], room->seat[32*room->cols + 2], room->seat[32*room->cols + 3]); 
		printcinema(room, 0);
		while (1 > 0)
		{
		fgets(chtemp, 2, stdin);
		if (chtemp[0] == '\n') break;
		if (chtemp[1] == '\n') break;

		//crow = strtol(chtemp, NULL, 10);
		//if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid row size");
		//else if ((crow > 0)&&(ccol <= MAX_ROW)) break;
	}
	}
	*/
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
		serveclient(socket_fd, room);
		prinf("Connection closed!");
	}
	return 0;		
}
