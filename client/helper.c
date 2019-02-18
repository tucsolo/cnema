/*

  HELPER.C
  ========
  
*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>


/*  Read a line from a socket  */


int recv_line(int sockfd, char *buf, int size)
{
	int i = 0;
	char c = '\r' ;
	int n;
	char tbuf[size];
	
	while ((i < size - 1) && (c != '\0')) 
	{
		n = recv(sockfd, &c, 1, 0);
		if (c == '\r') 
		{
			buf[i] = '\0';
			return -1;
		}
		if (n > 0) 
		{
			buf[i] = c;
			i++;
		}
		else c = '\0';
		//printf("\n%d%c%s",i, c, buf);
	}
	buf[i] = '\0';
	//buf[i-1] = '\0';
	//printf("aa%s", tbuf);
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
