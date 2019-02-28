#include <stdio.h>
#include <stdlib.h>
#include "messages.h"

/*
 * Just some printf macros, used to 
 * set colors and tags. 
 * 
 * Eonerror used to exit with failure
 * when a fatal error occurs.
 */

void prinf(const char *message)
{
	printf(cver "\n[INFO]\t%s" cbia, message);
}

void prsoc(const char *message, int fd)
{
	printf(ccia "\n[SOCK]\t%d says <%s>" cbia, fd, message);
}

void prerr(const char *message)
{
	printf(cros "\n[ERR!]\t%s" cbia, message);
}

void prwar(const char *message)
{
	printf(cgia "\n[WARN]\t%s" cbia, message);
}

void eonerror(const char *message)
{
	printf(cros "\n[ERR!]\tFatal Error!\n" cbia);
	perror(message);
	exit(EXIT_FAILURE);
}
