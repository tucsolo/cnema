/*

  HELPER.C
  ========
  
*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


/*  Read a line from a socket  */


int recv_line(int sockfd, char *buf, int size){
	int i = 0;
	char c = '\0';
	int n;

	while ((i < size - 1) && ((c != '\r')||(c != '\0')))
	{
		n = recv(sockfd, &c, 1, 0);
		if (n > 0) 
		{
			buf[i] = c;
			i++;
		}
		else c = '\0';
	}
	buf[i] = '\0';
	buf[i-1] = '\0';
	return i;
}

/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char *buffer;

    buffer = vptr;
    nleft  = n;

    while ( nleft > 0 )
	{
		if ( (nwritten = write(sockd, buffer, nleft)) <= 0 )
		{
	    	if ( errno == EINTR ) nwritten = 0;
		    else return -1;
		}
		nleft  -= nwritten;
		buffer += nwritten;
    }
    return n;
}
