.DEFAULT_GOAL: help
.PHONY: default
default: help ;
CC=gcc
CFLAGS=-Wall -Wextra -O2
CHEADERS=$(shell ls *.h)
CFILES=$(shell ls *.c)
OBFILES=$(shell ls *.o)
LDFLAGS=-pthread
CLTARGET=moonloop
SVTARGET=mnlserver

client:
	$(CC) $(CFLAGS) -o $(CLTARGET) $(CHEADERS) client*.c $(LDFLAGS)

server:
	$(CC) $(CFLAGS) -o $(SVTARGET) $(CHEADERS) server*.c $(LDFLAGS)

all:
	$(CC) $(CFLAGS) -o $(CLTARGET) $(CHEADERS) client*.c $(LDFLAGS)
	$(CC) $(CFLAGS) -o $(SVTARGET) $(CHEADERS) server*.c $(LDFLAGS)
	
clean:
	rm $(CLTARGET) $(SVTARGET) $(OBFILES)
	
help:
	@echo	""
	@echo	"	Moonloop makefile"
	@echo	""
	@echo	"	user@host:~/here$$ make client			builds client"
	@echo	"	user@host:~/here$$ make server			builds server"
	@echo	"	user@host:~/here$$ make all			builds both"
	@echo	"	user@host:~/here$$ make clean			removes binaries"
	@echo	"	user@host:~/here$$ make help			prints a nice help box"
	@echo	"	user@host:~/here$$ make whatever_else		fails miserably"
	@echo	""
