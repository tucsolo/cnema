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
* @file utils.h
* @brief This is the utility functions' header file of a multi channel chat application, 
* that allows communication between multiple clients using sockets.
*
* @authors Claudio Migliorelli, Emanuele Savo
*
* @date July 2018
*/


#ifndef SERVER_H
#define SERVER_H

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

#define LISTENQ (1024)
#define MAX_CHANNELS 1024
#define MAX_CLIENTS 1024
#define MAX_USERNAME_SIZE 128
#define MAX_REQ 4

typedef struct _client_t_struct {
        int fd;                                                 /* client socket fd */
	int command;						/* command sent with request */
       	int arg;						/* further arg in case of channel join */ 
        char username[MAX_USERNAME_SIZE];			/* client username */
} client_t_struct;

typedef struct _channel_struct {
        client_t_struct *owner;                                 /* pointer to the channel owner struct */
	int socket_fd;
	client_t_struct *chan_users[MAX_CLIENTS];		/* the actual chat users */
} channel_struct;

typedef struct _chanserver_t_struct {
        pthread_mutex_t chan_mutex;                             /* mutex for sending requests atomically */
} chanserver_t_struct;

/* 		utils.c			*/
void eonerror(const char *errormsg);
int recv_line(int sockfd, char *buf, int size);
size_t send_msg(int fd, char *buff);
void moonboot();
client_t_struct *alloc_tstruct();
chanserver_t_struct *alloc_cstruct();
channel_struct *alloc_chanstruct(); 
#endif
