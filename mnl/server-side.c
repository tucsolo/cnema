/* 
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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "server.h"

pthread_mutex_t chsxth = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
struct tmessage * peermsg ;

char * manglepacket(char * message, struct ctstruct * ctsaddr)
{
	struct ctstruct * th_data = (struct ctstruct *) ctsaddr;
	if ((strncmp(message, "MNL", sizeof(char)*3))) return "Not a Moonloop packet";
	if ((!strncmp(message, "MNLUSER", sizeof(char)*7))) 
	{
		char * usbuf;
		usbuf = malloc(sizeof(char)*17);
		if (usbuf == NULL) eonerror("error on malloc");
		strncpy(usbuf, &message[7], 16);
		usbuf[16] = '\0';
		th_data->username = usbuf;
		return "Username set";
	}
	if ((!strncmp(message, "MNLGETUSER", sizeof(char)*10))) return th_data->username;
	if ((!strncmp(message, "MNLGETCHANS", sizeof(char)*11))) 
	{
		pthread_mutex_lock(th_data->ctmutex);
		printf("\n[%u] Chanserv, wake up!", (unsigned int) th_data->thread_tid);
		//struct tmessage * commsg = 	alloc_msg();
		struct tmessage * commsg = th_data->messagep;
		commsg->ctclient = ctsaddr;
		commsg->data = NULL;
		commsg->opm = 1;
		commsg->fd = th_data->socketfd;
		pthread_cond_signal(th_data->ctcond); 
		pthread_mutex_unlock(th_data->ctmutex);
	}
	if ((!strncmp(message, "MNLCCHAN", sizeof(char)*8))) 
	{
		pthread_mutex_lock(th_data->ctmutex);
		printf("\n[%u] Chanserv, wake up!", (unsigned int) th_data->thread_tid);
		//struct tmessage * commsg = 	alloc_msg();
		struct tmessage * commsg = th_data->messagep;
		commsg->ctclient = ctsaddr;
		char * usbuf;
		usbuf = malloc(sizeof(char)*17);
		if (usbuf == NULL) eonerror("error on malloc");
		strncpy(usbuf, &message[8], 16);
		usbuf[16] = '\0';
		int i;
		for(i=0; i<17; i++) if (usbuf[i] == '\n') usbuf[i] = '\0';
		commsg->data = usbuf;
		commsg->opm = 2;
		commsg->fd = th_data->socketfd;
		pthread_cond_signal(th_data->ctcond); 
		pthread_mutex_unlock(th_data->ctmutex);
	}

	return &message[3];
}

void *client_thread(void *arg)
{
	char * message_buffer;
	struct ctstruct *thread_data = (struct ctstruct *) arg;
	thread_data->thread_tid = pthread_self() / 256;
	printf("\n[%u] Thread created", (unsigned int) thread_data->thread_tid);
	printf("\n[%u] serving fd %u\n", (unsigned int) thread_data->thread_tid, thread_data->socketfd);
	for (;;)
	{
		message_buffer = malloc(sizeof(char) * 1024);
		if (message_buffer == NULL) eonerror("Error on malloc");
		recv_line(thread_data->socketfd, message_buffer, 1024);
		if ((!strncmp(message_buffer, "killmepls", sizeof(char)*9))||(!strcmp(message_buffer, "")))
		{
			free(message_buffer);
			break;
		}
		printf("\n[%u] %s",(unsigned int) thread_data->thread_tid, manglepacket(message_buffer, thread_data));
		/*
		if ((!strncmp(message_buffer, "MNL", sizeof(char)*3)))
			printf("[%u] %s",(unsigned int) thread_data->thread_tid, message_buffer);
		*/
		free(message_buffer);
	}
	if (close(thread_data->socketfd) == -1) eonerror("error on close()");
	printf("\nClosed connection, descriptor %d", thread_data->socketfd);
	destroy_tstruct(thread_data);
	fflush(stdout);
	return 0;
}

void thread_spawn(struct ctstruct * arg)
{
	pthread_attr_t attr;
	arg->ctmutex = &chsxth;
	arg->ctcond = &cond;
	arg->messagep = peermsg;
	if ((pthread_attr_init(&attr)) != 0) eonerror("pthread_attr_init fail");
	if ((pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) eonerror("pthread_attr_setdetachstate() fail");
	if ((pthread_create(&(arg->thread_tid), &attr, &client_thread, arg)) != 0) eonerror("Error in pthread_create");
	if ((pthread_attr_destroy(&attr)) != 0) eonerror ("Error in pthread_attr_destroy()");
	return;
}

void chanserver()
{
	struct chs * chs_tmp = malloc(sizeof(struct chs));
	chs_tmp->ctmutex = &chsxth;
	chs_tmp->ctcond = &cond;
	chs_tmp->messagep = peermsg;
	if (chs_tmp == NULL) eonerror("error on malloc");
	pthread_attr_t attr;
	if ((pthread_attr_init(&attr)) != 0) eonerror("pthread_attr_init fail");
	if ((pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) eonerror("pthread_attr_setdetachstate() fail");
	if ((pthread_create(&(chs_tmp->thread_tid), &attr, &chanserv, chs_tmp)) != 0) eonerror("Error in pthread_create");
	if ((pthread_attr_destroy(&attr)) != 0) eonerror ("Error in pthread_attr_destroy()");
	return;
}

int main(int argc, char *argv[])
{
	//No buffering on stdout
	setvbuf(stdout, NULL, _IONBF, 0);
	
	int servport, optval=0, listensd=0, backlog=0;
	struct sockaddr_in servaddr, cliaddr;
	socklen_t len;
	socklen_t optlen = sizeof(optval);
	struct ctstruct *nth_tmp;
	//struct chs *chs_tmp;
	peermsg = malloc(sizeof(struct tmessage *));
	if (peermsg == NULL) eonerror("error on malloc");
	
	printf("\n\tWelcome to the sold-ehm, server side\n\tWhere there's no one here but me!\n\n");
	if (argc>2)
	{
		printf("\n\tAt most one parameter is required by Moonloop:\n\n\t\t%s <port> \n\n\tIf not specified, Moonloop will listen on port 42424.\n", argv[0]);
		eonerror("User doesn't know how to properly launch Moonloop");
	}
	if (argc==2) 
	{
		servport=strtol(argv[1], NULL, 10);
		if ((servport < 1) || (servport > 65535)) eonerror("User doesn't know that valid TCP/IP port range is 1-65535");
	}
	else servport=42424;
	printf("\n[*]\tLaunching Moonloop server at port %d\n", servport);
	
	//Lancio del chanserver
	chanserver();
	
	if ((listensd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) eonerror("error creating socket");
	memset((void *) &servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);	// il server accetta connessioni su una qualunque delle sue intefacce di rete 
	servaddr.sin_port = htons(servport);			// numero di porta del server 

	optval = 1;
	optlen = sizeof(optval);

	if (setsockopt(listensd, SOL_SOCKET, SO_REUSEPORT, &optval, optlen) < 0)
		printf("\n[!]\tWarning! Unable to set SO_REUSEPORT on listening socket");

	if ((setsockopt(listensd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen)) < 0) 
	{
		close(listensd);
		eonerror("Cannot set SO_KEEPALIVE option on socket");
	}

	printf("[*]\tSO_KEEPALIVE set\n");

	// assegna l'indirizzo al socket
	if ((bind(listensd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) eonerror("error on bind()");
	
	if (listen(listensd, backlog) < 0) eonerror("error on listen()");
	
	for (;;)
	{
		len = sizeof(cliaddr);
		nth_tmp = alloc_tstruct();
		if ((nth_tmp->socketfd = accept(listensd, (struct sockaddr *) &cliaddr, &len)) < 0) eonerror("error on accept()");
		thread_spawn(nth_tmp);
	 }
}

