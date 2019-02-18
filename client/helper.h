/*

  HELPER.H
  ========

*/


#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP


#include <unistd.h>             /*  for ssize_t data type  */

#define LISTENQ        (1024)   /*  Backlog for listen()   */


/*  Function declarations  */

ssize_t Readline(int fd, void *vptr, size_t maxlen);
ssize_t Writeline(int fc, const void *vptr, size_t maxlen);
int recv_line(int sockfd, char *buf, int size);
int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort);


#endif  /*  PG_SOCK_HELP  */

