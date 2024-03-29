/*
 *
 * client_helper.h 
 *
 */

#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP

#include <unistd.h>
#define LISTENQ (1024)

ssize_t Readline(int fd, void *vptr, size_t maxlen);
ssize_t Writeline(int fc, const void *vptr, size_t maxlen);
int recv_line(int sockfd, char *buf, int size);
int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort);


#endif  /*  PG_SOCK_HELP  */

