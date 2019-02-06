/*
* 
* This is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version.
* 
* This module is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
* 
* @file client-side.c 
* @brief This is the client side of a multi channel chat application, that allows 
* communication between multiple clients using sockets.
*
* @authors Claudio Migliorelli, Emanuele Savo
*
* @date July 2018
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define AUDIT if(1)

#define MAX_SIZE 128
#define MAX_LINE (1000)
#define MAX_IP_SIZE 11
#define MAX_PORT_SIZE 5
#define fflush(stdin) while(getchar() != '\n')

int eonerror(const char *errormsg)
{
	AUDIT
	printf("\n[!]\tHouston, we have a problem. Moonloop is crashing. \n\tLast words: %s\n\n", errormsg);
	exit(EXIT_FAILURE);
}

void moonboot(void)
{
	printf("\n\n\n +---------------------------------------------+ \n |                         _                   |\n |     _ __  ___  ___ _ _ | |___  ___ _ __     |\n |    | '  \\/ _ \\/ _ \\ ' \\| / _ \\/ _ \\ '_ \\    |\n |    |_|_|_\\___/\\___/_||_|_\\___/\\___/ .__/    |\n |                                   |_|       |\n +-- Multi channel chat application for *NIX --+ \n |                                             |\n +--   Claudio Migliorelli - Emanuele Savo   --+ \n |     ¯¯¯¯¯¯¯ ¯¯¯¯¯¯¯¯¯¯¯   ¯¯¯¯¯¯¯¯ ¯¯¯¯     |\n +---------------------------------------------+ \n\n");
}

int start(void)
{
	int res; int res_two; int fd; int len; int size;
	char buffer[MAX_SIZE];
	char username[MAX_SIZE];

	fd = open(".mnl/user", O_RDONLY, 0); // check if user config file exist
	if (fd == -1)	{
		printf("=> You're not a user yet\n=> Creating user profile for you...\n");

		res_two = system("mkdir .mnl");
		if (res_two == -1) eonerror("start: create .mnl folder error");

		printf("\n");
		printf("username: ");
		scanf("%s", buffer);
		printf("\n");

		res_two = system("touch .mnl/user");
		if (res_two != 0) eonerror("start: create user file error");

		fd = open(".mnl/user" , O_WRONLY, 0);
		if (fd == -1) eonerror("start: open user file error");

		res = sprintf(username, "USERNAME=%s", buffer);
		if (res == -1) eonerror("start: sprintf error");

		len = strlen(username);
		if (len == -1) eonerror("start: strlen error");

		res = write(fd, username, len);
		if (res == -1) eonerror("start: write error");

		fflush(stdin);

		printf("=> Welcome, %s!\n\nWhat do you want to do?\n", buffer);
		printf("\n");
	}

	else	{
		printf("=> You are already a user, I'm finding your config file...\n");

		fd = open(".mnl/user", O_RDONLY, 0);
		if (fd == -1) eonerror("start: open user file error");

		size = read(fd, buffer, MAX_SIZE);
		if (size == -1) eonerror("start: read error");

		len = strlen(buffer) - 9*sizeof(char); // len = len(buffer) - len("USERNAME=")
		if (len == -1) eonerror("start: strlen error");

		res = memcpy(username, &buffer[9], len);
		if (res == -1) eonerror("start: memcpy error");

		username[len+1] = "\0";

		printf("=> Welcome back, %s!\n\nWhat do you want to do?\n", username);
		printf("\n");

	}
	printf("[.] create channel\n");
	printf("[.] join channel\n");
	printf("[.] remove user\n");
	printf("[.] exit\n");
	printf("\n");

	return EXIT_SUCCESS;
}

int remove_user(void)
{
	int res;
	char input[MAX_SIZE];

	printf("Are you sure to delete you user account? [y/n] ");
	scanf("%s", input);
	printf("\n");

	if (!strcmp(input, "y"))	{
		printf(" [+] => Deleting your user account\n");		
		res = system("rm -r .mnl");
		if (res == -1) eonerror("remove_user: system error\n");
	}

	else	{
		printf(" [+] => I'm not deleting your user account\n");
	}
	printf("\n");

	return EXIT_SUCCESS;
}

int create_channel(void)
{
	int 		ds_sock;			/*    connection socket 	*/
	short int	port;				/*    port number 		*/
	struct 		sockaddr_in serveraddr;		/*    socket address structure 	*/
//	char		buffer[MAX_LINE];		/*    character buffer 		*/
	char		remote_address[MAX_IP_SIZE];	/*    holds remote IP address 	*/
	char		remote_port[MAX_PORT_SIZE];	/*    holds remote port number 	*/
	struct		hostent *he;			/*    struct for host		*/
	
	/* initialize struct */
	he = NULL;	

	/* set remote port */
	printf("=> Insert the remote port to connect to: ");
	scanf("%s", remote_port);
	printf("\n");

	AUDIT
	printf("[+]\tRPORT = %s\n\n", remote_port);

	fflush(stdin);

	/* set remote address */
	printf("=> Insert the remote ip address to connect to: ");
	if (gets(remote_address) == NULL) eonerror("create_channel: scanf error");
	printf("\n");

	AUDIT
	printf("[+]\tRHOST = %s\n\n", remote_address);

	if (inet_aton(remote_address, &serveraddr.sin_addr) <= 0)	{
		printf("=> Invalid IP address\n=> Trying to resolve DNS\n");
		if ((he=gethostbyname(remote_address)) == NULL)	{
			eonerror("create_channel: unable to resolve DNS name");
		}
		printf("=> Resolved DNS name\n");
		serveraddr.sin_addr = *((struct in_addr *)he->h_addr);
	}

	/* create listening socket */	
	ds_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (ds_sock < 0) eonerror("create_channel: socket error");

	/* initialize the socket address structure */
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	printf("=> I'm connecting to the server...\n\n");

	/* connect to the remote server */
	if (connect(ds_sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0)	{
		eonerror("create_channel: connect error");
	}

	printf("=> Connection established, have fun!\n\n");

	return EXIT_SUCCESS;

}

int main(int argc, char *argv[])
{
	char input[MAX_SIZE];
	int res;
	
	moonboot();

	res = start();
	if (res != 0)	{
		eonerror("main: start() error");
	}


	printf("=> ");
	gets(input);
	printf("\n");

	if (!strcmp(input, "create channel"))	{
		res = create_channel();
		if (res != 0)	{
			eonerror("main: create_channel() error");
		}
	}

	if (!strcmp(input, "remove user"))	{
		res = remove_user();
		if (res != 0)	{
			eonerror("main: remove_user() error");
		}
	}

	if (!strcmp(input, "exit"))	{
		exit(EXIT_SUCCESS);
	}

	return EXIT_SUCCESS;
}
