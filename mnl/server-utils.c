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
#include "server.h"

void eonerror(const char *errormsg)
{
	AUDIT
	printf("\n[!]\tHouston, we have a problem. Moonloop server is crashing. \n\tLast words: %s\n\n", errormsg);
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

struct tmessage * alloc_msg ()
{
	struct tmessage * rtaddress = malloc(sizeof(struct tmessage));
	if (rtaddress == NULL) eonerror("error on malloc");
	return rtaddress;
}

struct tmessage * initqueue ()
{
	return NULL;
}

void destroy_msg (struct tmessage * dmessage)
{
	free(dmessage->data);
	free(dmessage);
	return;
}

struct ctstruct * alloc_tstruct ()
{
	struct ctstruct * rtaddress = malloc(sizeof(struct ctstruct));
	if (rtaddress == NULL) eonerror("error on malloc");
	return rtaddress;
}

void destroy_tstruct (struct ctstruct * dtaddress)
{
	struct ctstruct *arg = (struct ctstruct *) dtaddress;
	free(arg->username);
	free(arg);
	return;
}
