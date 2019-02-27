#ifndef MESSAGES_LIB
#define MESSAGES_LIB

#include <stdio.h>
#include <stdlib.h>

#define cres  "\x1B[0m"
#define cros  "\x1B[31m"
#define cver  "\x1B[32m"
#define cgia  "\x1B[33m"
#define cblu  "\x1B[34m"
#define cmag  "\x1B[35m"
#define ccia  "\x1B[36m"
#define cbia  "\x1B[37m"

void prinf(const char *message);
void prsoc(const char *message, int fd);
void prerr(const char *message);
void prwar(const char *message);
void eonerror(const char *message);

#endif
