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
* @file server-side.c 
* @brief This is the server side of a multi channel chat application, that allows 
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
#include <errno.h>
#include "utils.h"

#ifdef DEBUG 			/* exec server side with debug facilities */
#define AUDIT if(1) 
#else
#define AUDIT if(0)		/* exec server side without debug facilities */
#endif 

chanserver_t_struct *chanserv;
client_t_struct *clients[MAX_CLIENTS] = {[0 ... (MAX_CLIENTS -1)] = NULL}; 
channel_struct *channels[MAX_CHANNELS] = {[0 ... (MAX_CHANNELS -1)] = NULL};
int num_clients = 0;
int num_channels = 0;								
long port;

void *chat_thread(void *chat_fd)
{
	char msg[128];
	int fd = (int *)chat_fd;

#ifdef DEBUG
	AUDIT
	printf("[x]\texecuting debug facility\n");
#endif

	AUDIT
	printf("[x]\tchat_thread executing - channel fd is %d\n", fd);

	recv_line(fd, msg);

	AUDIT
	printf("[x]\tfirst message retrieved is %s\n", msg);

	return (void *)0;
}

void *remove_user(void *client_fd)
{
	int fd = (int *)client_fd;

	AUDIT
        printf("[x]\tclient with fd %d wants to remove user profile\n", fd);

	return (void *)0;
}

void *delete_channel(void *client_fd)
{
	int fd = (int *)client_fd;

	AUDIT
        printf("[x]\tclient with fd %d wants to delete a channel\n", fd);

	return (void *)0;
}

void *join_channel(void *client_fd)
{
	int fd = (int *)client_fd;

	AUDIT
	printf("[x]\tclient with fd %d wants to join a channel\n", fd);
		
	return (void *)0;
}

void *create_channel(void *client_fd)
{
	socklen_t sin_size;
	int list_s;
	struct sockaddr_in servaddr;
        struct sockaddr_in clientaddr;
	int i;
	channel_struct *tmp;
	client_t_struct *creator = NULL;
	int create_port = port;
	int index = num_channels;
	pthread_t tid;
	char fds[5];
	
	int fd = (int *)client_fd;

	AUDIT
	printf("[x]\tclient with fd %d wants to create a channel\n", fd);
	
	tmp = alloc_chanstruct();
	AUDIT
	printf("[x]\tchannel struct allocated - address is %p\n", (void *)tmp);

	/* setting the listening socket */
        list_s = socket(AF_INET, SOCK_STREAM, 0);
        if (list_s < 0) {
                eonerror("SERVER - listening socket creation error");
        }

	AUDIT
	printf("[x]\tlistening socket installed - fd is %d\n", list_s);

        /* setting the socket address data structure */
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family             = AF_INET;
        servaddr.sin_addr.s_addr        = htonl(INADDR_ANY);
        servaddr.sin_port               = htons(create_port++);

        /* mark the socket as a listening socket */
        if (listen(list_s, LISTENQ) < 0) {
                eonerror("SERVER - listen error");
        }

	AUDIT
	printf("[x]\tassigning fd to channel struct fd field\n");

	tmp->socket_fd = list_s;

	AUDIT
	printf("[x]\tfd value inserted - retrieved value is %d\n", tmp->socket_fd);

	for (i=0; i<MAX_CLIENTS; i++) {
		if (clients[i]->fd == fd) {
			creator = clients[i];
			break;
		}
	}

	AUDIT
	printf("[x]\tchannel creator retrieved is %s\n", creator->username);	

	tmp->owner = creator;

	AUDIT
	printf("[x]\tusername value inserted - retrieved value is %s\n", tmp->owner->username);

	if (index <= MAX_CHANNELS) {

                channels[index] = tmp;
                if (channels[index] == NULL) eonerror("SERVER - client struct insert error");

                AUDIT
                printf("[x]\tchannel insert successfully executed - address is %p\n", 			\
                                                clients[index]);

                AUDIT
                printf("[x]\tchannel values are owner: %s - fd: %d\n", channels[index]->owner->username,\
                        channels[index]->socket_fd);

                num_channels++;

		AUDIT
		printf("[x]\ttrying to assign this tmp struct - username: %s fd: %d\n", 		\
				tmp->owner->username, tmp->owner->fd);

		if (channels[index]->chan_users[0] != NULL) 
			eonerror("SERVER - created channel that is not empty");
		
		/* assigning first channel user */
		channels[index]->chan_users[0] = tmp->owner;
	
		AUDIT
		printf("[x]\tclient chan insert successfully executed - address is %p\n",       	\
				(void *)channels[index]->chan_users[0]);
		AUDIT
		printf("[x]\tclient chan values are username: %s - fd: %d - command: %d\n",     	\
				channels[index]->chan_users[0]->username,               		\
				channels[index]->chan_users[0]->fd,                     		\
				channels[index]->chan_users[0]->command);

		sprintf(fds, "%d", channels[index]->socket_fd);

		send_msg(channels[index]->chan_users[0]->fd, fds);
		send_msg(channels[index]->chan_users[0]->fd, "Welcome!");
        }


	pthread_create(&tid, NULL, chat_thread, (void *)channels[index]->socket_fd);

	return (void *)0;
}

void *chanserv_function(void *client_struct)
{
	pthread_t tid;
	int res;
	char request[MAX_REQ];
	char username[MAX_USERNAME_SIZE];
	int index = num_clients;
	int join_channel_fd;

	client_t_struct *tmp = (client_t_struct *)client_struct;

	AUDIT
	printf("[x]\tchanserv thread executing - client fd is %d\n", tmp->fd);

	AUDIT
	printf("[x]\ttrying to take the lock...\n");

	pthread_mutex_lock(&chanserv->chan_mutex);
	
	AUDIT
	printf("[x]\tlock obtained, entering the critical section...\n");

	res = recv_line(tmp->fd, request, MAX_REQ);
	if (res == 0) eonerror("SERVER - chanserv recv_line error");

	tmp->command = strtol(request, NULL, 10);

	AUDIT
	printf("[x]\tretrieved command is %d\n", tmp->command);

	res = recv_line(tmp->fd, username, MAX_USERNAME_SIZE);
	if (res == 0) eonerror("SERVER - chanserv recv_line error");

	strncpy(tmp->username, username, res-1);

	AUDIT
	printf("[x]\tretrived username is %s - size is %d\n", tmp->username, res);

	AUDIT
	printf("[x]\tmax clients index value is %d\n", index);

	if (tmp->command == 2) {
		res = recv_line(tmp->fd, request, MAX_REQ);
       		if (res == 0) eonerror("SERVER - chanserv recv_line error");
		tmp->arg = strtol(request, NULL, 10);

		AUDIT
		printf("[x]\tretrieved fd for channel join is %d\n", tmp->arg);
	}

	if (index <= MAX_CLIENTS) {
	
		clients[index] = tmp;
		if (clients[index] == NULL) eonerror("SERVER - client struct insert error");
	
		AUDIT
		printf("[x]\tclient insert successfully executed - address is %p\n", 			\
						clients[index]);

		AUDIT
		printf("[x]\tclient values are %s - %d\n", clients[index]->username,			\
			clients[index]->fd);

		num_clients++;

	}

	else {
		eonerror("SERVER - max clients number reached");
	}

	pthread_mutex_unlock(&chanserv->chan_mutex);

	AUDIT
	printf("[x]\tclient with fd %d sended request with code %d\n", 					\
		tmp->fd, tmp->command);

	/* spawn command thread based on user input */
	switch(tmp->command) {
		case 1:
			pthread_create(&tid, NULL, create_channel, (void *)tmp->fd);
			break;
		case 2:
			pthread_create(&tid, NULL, join_channel, (void *)tmp->fd);
			break;
		case 3:
			pthread_create(&tid, NULL, delete_channel, (void *)tmp->fd);
			break;
		case 4:
			pthread_create(&tid, NULL, remove_user, (void *)tmp->fd);
			break;
		default:			
			eonerror("SERVER - unrecognized command");
	}
	
	AUDIT
	printf("[x]\tlock released...\n");

	return (void *)0;
}

int main(int argc, char *argv[])
{
	socklen_t sin_size;
	int list_s;
	struct sockaddr_in servaddr;	
	struct sockaddr_in clientaddr;
	client_t_struct *tmp;	
	pthread_t tid;

	moonboot();	
	
	printf("[+]\tWelcome to the server side\n\n");
	
	if (argc != 2) {
		printf("[-]\tusage: ./server port\n");
		eonerror("SERVER - server-side launch error");
	}
	
	/* setting the input port */
	port = strtol(argv[1], NULL, 10);
	if ((port == EINVAL) || (port == ERANGE) || (port == 0))
		eonerror("SERVER - port input error");

	/* check if the input port is valid */
	if ((port <= 0) || (port >= 65536)) eonerror("SERVER - not a valid TCP port");
	
	printf("[+]\tLPORT set to value %lu\n\n", port);

	/* setting the listening socket */
	list_s = socket(AF_INET, SOCK_STREAM, 0);
	if (list_s < 0) eonerror("SERVER - listening socket creation error");
	
	/* setting the socket address data structure */
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family 		= AF_INET;
	servaddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	servaddr.sin_port		= htons(port);

	/* bind socket address to the listening socket */
        if (bind(list_s, (struct sockaddr_in *)&servaddr, sizeof(servaddr)) < 0)
		eonerror("SERVER - bind error");

	/* mark the socket as a listening socket */
	if (listen(list_s, LISTENQ) < 0) eonerror("SERVER - listen error");

	/* initializing the chanserv struct for managing requests */
	chanserv = alloc_cstruct();
	AUDIT
	printf("[x]\tchanserver struct allocated - address is %p\n", (void *)chanserv);

	/* starting infinite loop for responding client requests */
	for (;;) {
		printf("[.]\tWaiting for connections...\n\n");

		tmp = alloc_tstruct();
		sin_size = sizeof(struct sockaddr_in);

		AUDIT
		printf("[x]\ttstruct allocated at address %p with size %ld\n", (void *)tmp, 		\
						(long int)sin_size);

		tmp->fd = accept(list_s, (struct sockaddr_in *)&clientaddr, &sin_size);
		if (tmp->fd < 0) eonerror("SERVER - accept error");
		
		AUDIT
		printf("[x]\tthe tmp-client struct has fd %d\n", tmp->fd);

		printf("[+]\tConnection accepted from address %s\n\n", inet_ntoa(clientaddr.sin_addr));

		pthread_create(&tid, NULL, chanserv_function, (void *)tmp);

	}
	
	return 0;
		
}
