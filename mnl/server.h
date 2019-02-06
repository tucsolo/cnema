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
#include "server.h"

#define AUDIT if(1)
#define PAGE_SIZE 4096
#define MAX_SIZE 128
#define MAX_CHANNELS 128
#define MAX_CLIENTS_PER_CHANNEL 128
#define MAX_PACKET_SIZE 1024


typedef struct tchannel
{
	struct chs * chs_d;
	pthread_mutex_t * ctmutex;
	pthread_cond_t * ctcond;
	pthread_mutex_t ccmt;
	pthread_cond_t ccond;
	pthread_t thread_tid;	/* thread ID */
	char * name;
	int chid;
	int ** clients;
}tchannel;

typedef struct tmessage
{
	char * data;
	int chid;
	int opm;
	int fd;
	struct ctstruct * ctclient;
	/*	Operator Messages
	 *
	 *	1	Get Channels List (send to fd)
	 * 	2	Create Channel (char * data for name)
	 * 	3	Join Channel (char * data for name)	
	 */
}tmessage;

typedef struct ctstruct
{
	pthread_mutex_t * ctmutex;
	pthread_cond_t * ctcond;
	struct tmessage * messagep;
	int socketfd;
	pthread_t thread_tid;	/* thread ID */
	long thread_count;		/* # connections handled */
	char * username;
	char ** channels;
	int ** chid;
	pthread_mutex_t ** ccmt;
	pthread_cond_t ** ccond;
	struct tmessage ** cmsg;
}ctstruct;

typedef struct chs
{
	pthread_mutex_t * ctmutex;
	pthread_cond_t * ctcond;
	struct tmessage * messagep;
	pthread_t thread_tid;
	int channels;
	char ** names;
	pthread_t ** thids;
	struct tchannel ** cthdata;
}chs;

//server-side.c
char * manglepacket(char * message, struct ctstruct * ctsaddr);
void *client_thread(void *arg);
void chanserver();

//server-chanserv.c
void *chanserv(void *arg);

//server-utils.c
void eonerror(const char *errormsg);
void destroy_tstruct (struct ctstruct * dtaddress);
int recv_line(int sockfd, char *buf, int size);
size_t send_msg(int fd, char *buff);
struct ctstruct * alloc_tstruct ();
struct tmessage * initqueue ();
struct tmessage * alloc_msg ();
void destroy_msg (struct tmessage * dmessage);

#endif
