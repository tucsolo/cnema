/*
* This software is released under the MIT license:
* Permission is hereby granted, free of charge, to any person obtaining a copy of
* this software and associated documentation files (the "Software"), to deal in
* the Software without restriction, including without limitation the rights to
* use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
* the Software, and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:

* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.

* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
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
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>

#include "utils.h"

#ifdef DEBUG
#define AUDIT if(1)
#else
#define AUDIT if(0)
#endif

#define MAX_SIZE (128)
#define fflush(stdin) while(getchar() != '\n')

char buffer[MAX_USERNAME_SIZE];
char username[MAX_USERNAME_SIZE];
int buffer_len;
int username_len; 

char remote_address[MAX_SIZE];
long remote_port = 0;

int delete_user(void)
{
	int res;

	res = system("rm -rf .mnl/");
	if (res == -1) return -1;

	printf("[+]\tYour user profile has been deleted\n\n");
	return 0;
}

int create_channel(void)
{	
	struct sockaddr_in servaddr;
	int conn_s;
	struct hostent *he;
	char command[MAX_REQ] = "1";
	char client_user[MAX_USERNAME_SIZE];
	int res;
	char _serv_sock[5];
	int serv_sock;

	he = NULL;

	if ((conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		eonerror("CLIENT - connection socket creation error");

	/* setting the socket address data structure */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(remote_port);

	if (inet_aton(remote_address, &servaddr.sin_addr) <= 0) {
		
		printf("[!]\tIP address not valid - trying to resolve hostname...\n\n");

		if ((he=gethostbyname(remote_address)) == NULL)
			eonerror("IP address not valid - couldn't resolve hostname");
	
		printf("[!]\tHostname resolved\n\n");
                servaddr.sin_addr = *((struct in_addr *)he->h_addr);
	}

	if (connect(conn_s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		eonerror("CLIENT - connect error");

	AUDIT
	printf("[x]\tsending command %s\n", command);

	/* sending the create channel command */
	send_msg(conn_s, command);
	send_msg(conn_s, "\n");

	AUDIT
	printf("[x]\tcreate channel command sent\n");

	strncpy(client_user, buffer, MAX_USERNAME_SIZE);

	AUDIT
	printf("[x]\tsending username %s\n", client_user);

	/* sending the username */
	res = send_msg(conn_s, client_user);
	send_msg(conn_s, "\n");

	AUDIT
	printf("[x]\tusername sent - %d bytes left\n", res);

	recv_line(conn_s, _serv_sock, 5);
	serv_sock = strtol(_serv_sock, NULL, 10);
	
	AUDIT
	printf("[x]\tserver socket retrieved is %d\n", serv_sock);

	return 0;
}

int start(void)
{
	int fd, res, size;	

	/* check if user config file exists */
	fd = open(".mnl/user", O_RDONLY, 0);
	if (fd == -1) {

		printf("[!]\tYou're not a user yet\n[...]\tCreating user profile for you...\n");
	
		if (!(access(".mnl", F_OK) != -1)) { 
			res = system("mkdir .mnl");
			if (res == -1) eonerror("CLIENT - create .mnl folder error");
		}

		/* take username string in input */
		printf("\n");
		printf("username: ");
		scanf("%s", buffer);
		printf("\n");
		username_len = strlen(buffer);
	
		res = system("touch .mnl/user");
		if (res == -1) eonerror("CLIENT - create user file error");	

		fd = open(".mnl/user", O_WRONLY, 0);
		if (fd == -1) eonerror("CLIENT - open user file error");

		res = sprintf(username, "%s", buffer);
		if (res == -1) eonerror("CLIENT - user file sprintf error");

		buffer_len = strlen(username);
		if (buffer_len == -1) eonerror("CLIENT - strlen error");

		res = write(fd, username, buffer_len);
		if (res == -1) eonerror("CLIENT - write user file error");

		fflush(stdin);

		printf("[+]\tWelcome, %s!\n\n", buffer);
	}
	
	else {
		printf("[!]\tYou're already a user, finding the config file for you...\n\n");	
		
		fd = open(".mnl/user", O_RDONLY, 0);
		if (fd == -1) eonerror("CLIENT - open user file error");
		
		size = read(fd, buffer, MAX_SIZE);
		if (size == -1) eonerror("CLIENT - read user file error");
		
		AUDIT
		printf("[x]\tusername length and username retrieved - values: %d, %s\n", size, buffer);

		printf("[+]\tWelcome, %s!\n\n\tWhat do you want to do?\n\n", buffer);
                printf("\n");	
	}

        return EXIT_SUCCESS;	
}

int main(int argc, char *argv[])
{
	int res, choice;

	moonboot();

	printf("[+]\tWelcome to the client side\n\n");
        
        if (argc != 3) {
                printf("[-]\tusage: ./client server-address port\n");
                eonerror("CLIENT - client-side launch error");
        }

	AUDIT
	printf("[x]\targvs retrieved: %s - %s\n", argv[1], argv[2]);
        
        /* setting the input port */
        remote_port = strtol(argv[2], NULL, 10);
        if ((remote_port == EINVAL) || (remote_port == ERANGE) || \
			(remote_port == 0)) {
                eonerror("CLIENT - port input error");
        }

        /* check if the input port is valid */
        if ((remote_port <= 0) || (remote_port >= 65536)) {
                eonerror("CLIENT - not a valid TCP port");
        }
        
        printf("[+]\tRPORT set to value %lu\n\n", remote_port);

	if (strcpy(remote_address, argv[1]) == NULL) eonerror("CLIENT - strcpy error");

	printf("[+]\tRADDR set to %s\n\n", remote_address);

	start();

	printf("[1]\tCreate channel\n");
        printf("[2]\tJoin channel\n");
	printf("[3]\tDelete channel\n");
        printf("[4]\tRemove user\n");
        printf("[5]\tExit\n");
        printf("\n");

retry:
	printf("\tWhat do you want to do [1,2,3,4,5]? ");
	res = scanf("%d", &choice);
	if (res == -1) eonerror("CLIENT - scanf error");

	fflush(stdin);

	printf("\n");

	AUDIT
	printf("[x]\tyour choice is %d\n", choice);

	switch(choice) {
		case 1:
			create_channel();
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;	
		default:
			printf("[!]\tUnrecognized choice - try again\n\n");
			goto retry;
	}
	
	return 0;
}
