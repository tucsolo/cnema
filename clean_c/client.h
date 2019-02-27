#ifndef CLIENT_HEAD
#define CLIENT_HEAD

#include <sys/socket.h>		/*  socket definitions        */
#include <sys/types.h>		/*  socket types              */
#include <arpa/inet.h>		/*  inet (3) funtions         */
#include <unistd.h>		/*  misc. UNIX functions      */
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "messages.h"
#include "client_helper.h"

#define MAX_LINE 1024

typedef struct cth_data {
	pthread_mutex_t *th_mutex;
	pthread_cond_t *th_cond;
	pthread_t th_tid;
	int fd;
} cth_data;

void *ping_client(void *arg);
void cthread_spawn(struct cth_data *arg, int fd);
int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort);
int getfreeid(int fd);
int getidinfo(int fd, int id);
void getanswer(char *command, int fd);
void getnullanswer(int fd);
int mainmenu(int resno, int fd);

#endif
