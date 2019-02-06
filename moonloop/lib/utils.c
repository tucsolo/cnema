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
* @file utils.c 
* @brief This is the utility functions of a multi channel chat application, that allows 
* communication between multiple clients using sockets.
*
* @authors Claudio Migliorelli, Emanuele Savo
*
* @date July 2018
*/

#define _GNU_SOURCE
#include <errno.h>
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
#include "utils.h"

void moonboot()
{
        printf("\n +---------------------------------------------+ \n |                         _                   |\n |     _ __  ___  ___ _ _ | |___  ___ _ __     |\n |    | '  \\/ _ \\/ _ \\ ' \\| / _ \\/ _ \\ '_ \\    |\n |    |_|_|_\\___/\\___/_||_|_\\___/\\___/ .__/    |\n |                                   |_|       |\n +-- Multi channel chat application for *NIX --+ \n |                                             |\n +--   Claudio Migliorelli - Emanuele Savo   --+ \n |     ¯¯¯¯¯¯¯ ¯¯¯¯¯¯¯¯¯¯¯   ¯¯¯¯¯¯¯¯ ¯¯¯¯     |\n +---------------------------------------------+ \n\n");
}

void eonerror(const char *errormsg)
{
	printf("\n[!]\tProblem found. Moonloop server is crashing. \n\tLast words: %s\n", errormsg);
	exit(EXIT_FAILURE);
}

int recv_line(int sockfd, char *buf, int size)
{
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && (c != '\n')) {
		n = recv(sockfd, &c, 1, 0);
		if (n > 0) {
			if (c == '\r') {
				n = recv(sockfd, &c, 1, MSG_PEEK);
				if ((n > 0) && (c == '\n')) recv(sockfd, &c, 1, 0);
				else c = '\n';
			}
			buf[i] = c;
			i++;
		} else c = '\n';
	}
	buf[i] = '\0';
	return i;
}

size_t send_msg(int fd, char *buff)
{
        size_t nleft = strlen(buff);
        ssize_t nsend = 0;
        while (nleft > 0) {
                if ((nsend = send(fd, buff, nleft, MSG_NOSIGNAL)) <= 0) {
                        if (nsend < 0 && errno == EINTR) nsend = 0;
                        if (nsend < 0 && errno == EPIPE) nsend = 0;
                        else
                                eonerror("error with send()");
                }
                nleft -= nsend;
                buff += nsend;
        }
        return nleft;
}

client_t_struct *alloc_tstruct()
{
	client_t_struct *addr = malloc(sizeof(client_t_struct));
	if (addr == NULL)
		eonerror("UTILS: tstruct malloc error");
	return addr;
}

chanserver_t_struct *alloc_cstruct()
{
	chanserver_t_struct *addr = malloc(sizeof(chanserver_t_struct));
	if (addr == NULL)
		eonerror("UTILS: cstruct malloc error");
	return addr;
} 

channel_struct *alloc_chanstruct()
{
	channel_struct *addr = malloc(sizeof(channel_struct));
	if (addr == NULL)
		eonerror("UTILS: chanstruct malloc error");
	return addr;
}
