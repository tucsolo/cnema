#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "messages.h"
#include "client_helper.h"    

#define MAX_LINE 1024

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort)
{
    int n = 1;
    while ( n < argc )
	{
		if 		( !strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2) ) *szAddress = argv[++n];
		else if ( !strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2) ) *szPort = argv[++n];
		else if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) ) n = 99;
		++n;
    }
	if ((argc==1)||(argc > 90))
	{
		prinf("Usage:");
		prinf("\t./client -a <address> -p <port>  [-h]\n\n");
		exit(EXIT_SUCCESS);
	}
    return 0;
}
int getfreeid(int fd)
{
	int exr;
	int outid = 0;
	char buffer2[50];
	Writeline(fd, "y\n", 2);
    while(1 > 0)
	{
		exr = recv_line(fd, buffer2, MAX_LINE);
		outid = strtol(buffer2, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) eonerror("Error getting a reservation id");
		if (exr == -1) break;
	}
	return outid;

}
int getidinfo(int fd, int id)
{
	int exr;
	unsigned int outid = 0;
	char buffer2[100];
	snprintf(buffer2, 100, "g%u\n", id);
	Writeline(fd, buffer2, strlen(buffer2));
    while(1 > 0)
	{
		exr = recv_line(fd, buffer2, MAX_LINE);
		if (outid == 0) outid = strtol(buffer2, NULL, 10);
		if ((errno == EINVAL) || (errno == ERANGE)) eonerror("Error getting reservation id's info");
		if (exr == -1) break;
	}
	return outid;
}
void getanswer(char * command, int fd)
{
	int exr;
	char * buffer2;
	buffer2 = calloc(MAX_LINE, sizeof(char));
	if (buffer2 == NULL) eonerror("malloc on temp char");
	Writeline(fd, command, strlen(command));
	while(1 > 0)
	{
		exr = recv_line(fd, buffer2, MAX_LINE);
		printf("%s",buffer2);
		if (exr == -1) break;
	}
	if (command[0] == 'x') exit(EXIT_SUCCESS);
}
int mainmenu(int resno, int fd)
{
	int seats;
	char * chtemp;
	int command = 0;
	chtemp = calloc(512, sizeof(char));
	if (chtemp == NULL) eonerror("malloc on temp char");
	chtemp[0] = 'p';
	if (resno == -1) 
	{
		printf(ccia"\n\t\t*------------------*\n\t\t|" cbia " C:NEMA Main Menu " ccia"|\n\t\t*------------------*\n" cbia "\n\tActually you're not working on a particular reservation id.\n\tWhat do you want to do?" ccia "\n\n\t[1]\tShow cinema status" cver "\n\t[2]\tWork on an existent reservation id" cgia"\n\t[3]\tOpen a new reservation" cmag "\n\t[4]\tClean up empty reservation ids" cbia "\n\n\t[5]\tExit from this client\n");
		
		while (1 > 0)
		{
			printf(cbia"\nC:NEMA :> ");
			if ((fgets(chtemp, 7, stdin)) == NULL) prerr("Invalid command");
			command = strtol(chtemp, NULL, 10);
			if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
			else if ((command > 0)&&(command <= 5)) break;
		}
		if (command == 3) return getfreeid(fd);
		if (command == 2) while (1 > 0)
		{
			printf(cbia "\nInsert reservation id [0 to exit]: ");
			if ((fgets(chtemp, 7, stdin)) == NULL) prerr("Invalid resource id");
			resno = strtol(chtemp, NULL, 10);
			if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid resource id");
			if (getidinfo(fd, resno) == 0) prerr("Resource id not valid!");
			if (resno == 0) break;
			else return resno;
		}
	}
	if ((command == 0) && (resno != -1))
	{
		printf(cbia"\n\tYou're working on reservation" cver" #%d"cbia".\n\n\tWhat do you want to do about that?" ccia "\n\n\t[1]\tShow reserved seats" cver "\n\t[2]\tReserve specific seats\n\t[3]\tReserve some seats" cgia "\n\t[4]\tFree specific seats\n\t[5]\tFree some seats"cmag "\n\t[6]\tDelete this reservation\n\n\t" cbia "[7]\tGo back to main menu", resno);
		while (1 > 0)
		{
			printf(cbia"\nC:NEMA :> ");
			if ((fgets(chtemp, 7, stdin)) == NULL) prerr("Invalid command");
			command = strtol(chtemp, NULL, 10);
			if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
			else if ((command > 0)&&(command <= 7)) break;
		}
		if (command == 7) return -1;
		if (command == 6) 
		{
			snprintf(chtemp, 512, "d%d\n", resno);
			getanswer(chtemp, fd);
			return -1;
		}
		if (command == 1) 
		{
			snprintf(chtemp, 512, "l%d\n", resno);
			getanswer(chtemp, fd);
			return resno;
		}
		if (command == 2)
		{
			char chtemp2[50];
			unsigned int nseats = 0;
			snprintf(chtemp, 512, "r%d", resno);

			while (1 > 0)
			{
				int row = -1;
				int col = -1;
				printf("\nBooking seat #%d", nseats + 1);
				while (1 > 0)
					{
						printf(cbia"\n[Q to exit] Row\t: ");
						if ((fgets(chtemp2, 7, stdin)) == NULL) prerr("Invalid command");
						if ((chtemp2[0] == 'Q')|| (chtemp2[0] == 'q')) break;
						row = strtol(chtemp2, NULL, 10);
						if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
						if (row >= 0) break;
					}
					if (row != -1) while (1 > 0)
					{
						printf(cbia"\n[Q to exit] Column\t: ");
						if ((fgets(chtemp2, 7, stdin)) == NULL) prerr("Invalid command");
						if ((chtemp2[0] == 'Q')|| (chtemp2[0] == 'q')) break;
						col = strtol(chtemp2, NULL, 10);
						if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
						if (col >= 0) break;
					}
				if ((row == -1)||(col == -1)) break;
				nseats++;
				snprintf(chtemp2, 50, ",%d,%d", row,col);
				chtemp = strcat(chtemp, chtemp2);
			}
			chtemp = strcat(chtemp, ".\n");
			getanswer(chtemp, fd);
			return resno;
		}
		if (command == 3)
		{
			while (1 > 0)
			{
				seats = -1;
				printf(cbia "\nHow many seats do you want to book? ");
				if ((fgets(chtemp, 7, stdin)) == NULL) prerr("Invalid resource id");
				seats = strtol(chtemp, NULL, 10);
				if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid resource id");
				if (seats > -1) break;
			}
		snprintf(chtemp, 512, "f%d,%d\n", resno, seats);
		getanswer(chtemp, fd);
		return resno;
		}
		if (command == 4)
		{
			char chtemp2[50];
			unsigned int nseats = 0;
			snprintf(chtemp, 512, "c%d", resno);
			while (1 > 0)
			{
				int row = -1;
				int col = -1;
				printf("\nRemoving seat reservation #%d", nseats + 1);
				while (1 > 0)
					{
						printf(cbia"\n[Q to exit] Row\t: ");
						if ((fgets(chtemp2, 7, stdin)) == NULL) prerr("Invalid command");
						if ((chtemp2[0] == 'Q')|| (chtemp2[0] == 'q')) break;
						row = strtol(chtemp2, NULL, 10);
						if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
						if (row >= 0) break;
					}
					if (row != -1) while (1 > 0)
					{
						printf(cbia"\n[Q to exit] Column\t: ");
						if ((fgets(chtemp2, 7, stdin)) == NULL) prerr("Invalid command");
						if ((chtemp2[0] == 'Q')|| (chtemp2[0] == 'q')) break;
						col = strtol(chtemp2, NULL, 10);
						if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid command");
						if (col >= 0) break;
					}
			if ((row == -1)||(col == -1)) break;
			nseats++;
			snprintf(chtemp2, 50, ",%d,%d", row,col);
			chtemp = strcat(chtemp, chtemp2);
			}
			chtemp = strcat(chtemp, ".\n");
			getanswer(chtemp, fd);
			return resno;
		}
		if (command == 5)
		{
			while (1 > 0)
			{
				seats = -1;
				printf(cbia "\nHow many seats do you want to free? ");
				if ((fgets(chtemp, 7, stdin)) == NULL) prerr("Invalid resource id");
				seats = strtol(chtemp, NULL, 10);
				if ((errno == EINVAL) || (errno == ERANGE)) prerr("Invalid resource id");
				if (seats > -1) break;
			}
		snprintf(chtemp, 512, "u%d,%d\n", resno, seats);
		getanswer(chtemp, fd);
		return resno;
		}
		if (getidinfo(fd, resno) == 0) 
		{
			printf(cmag "Reservation #%d not valid anymore (maybe because no more seats were reserved), exiting", resno);
				return -1;
		}
	}
	if (command == 5) strcpy(chtemp, "x\n");
	if (command == 4) strcpy(chtemp, "z\n");
	if (command == 1) strcpy(chtemp, "l\n");
	getanswer(chtemp, fd);
	return resno;
}
int main(int argc, char *argv[])
{
    int       conn_s;                
    short int port;                  
    struct    sockaddr_in servaddr;  
    char     *saddr;             
    char     *sport;                
    char     *endptr;                
	struct	  hostent *he;


	he=NULL;
	setvbuf(stdout, NULL, _IONBF, 0);

    /*  Get command line arguments  */

    ParseCmdLine(argc, argv, &saddr, &sport);

	/*  Set the remote port  */

    port = strtol(sport, &endptr, 0);
    if ( *endptr ) eonerror("Invalid port");
	
    /*  Create the listening socket  */

    if ( (conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) eonerror("socket creation");

    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(port);

    /*  Set the remote IP address  */

    if ( inet_aton(saddr, &servaddr.sin_addr) <= 0 )
	{
		prinf("Not an IP. Trying resolving name");
		
		if ((he=gethostbyname(saddr)) == NULL) eonerror("dns resolve failed");
		printf("Name solved!.\n\n");
		servaddr.sin_addr = *((struct in_addr *)he->h_addr);
    }
	
    if ( connect(conn_s, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ) eonerror("connect");


	int a = -1;
//int b = fork();
/*if (b == 0) while(1 > 0)
{
	sleep(10);
	Writeline(conn_s, "Ping!\n", 6);
}*/
	while(1 > 0) a = mainmenu(a, conn_s);
	
	prwar("Server closed or not responding, exiting.");
	exit(EXIT_FAILURE);
}


