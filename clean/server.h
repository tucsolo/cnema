#ifndef SERVER_HEAD
#define SERVER_HEAD

#define MAX_COL 100
#define MAX_ROW 100
#define MAX_SEA 9000000
#define MAX_THR 3000
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

void ParseCmdLine(int argc, char *argv[], unsigned int * port, unsigned int * row, unsigned int * col);
size_t send_msg(int fd, char *buff);
int recv_line(int sockfd, char *buf, int size);

typedef struct cinema
{
		unsigned int rows;
		unsigned int cols;
		unsigned int * seat;
		unsigned int * indexes;
} cinema;
typedef struct th_data
{
		pthread_mutex_t * th_mutex;
		pthread_cond_t * th_cond;
		pthread_t th_tid;
		int fd;
		struct cinema * mycinema;
} th_data;

struct cinema * initcinema(unsigned int cols, unsigned int rows);
void printcinema(struct cinema * room, unsigned int res_id, int fd);
void lindex(char * message, struct cinema * room, int fd);
void fill(char * message, struct cinema * room, int fd, int places, unsigned int index);
int reserve(char * message, struct cinema * room, int fd);
void unfill(char * message, struct cinema * room, int fd);
void cancels(char * message, struct cinema * room, int fd);
void cancela(char * message, struct cinema * room, int fd);
int getindex(struct cinema * room);
int getindexinfo(char * message, struct cinema * room);
int lockindex(struct cinema * room);
void checkzeros(struct cinema * room);
void * serveclient(void * arg);
void thread_spawn(struct th_data * arg, struct cinema * room);

#endif
