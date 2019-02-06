.DEFAULT_GOAL: help
.PHONY: default
default: help ;
CC=gcc
SSOURCE=server-side.c
SOBJECT=server-side.o
STARGET=server
CSOURCE=client-side.c
COBJECT=client-side.o
CTARGET=client
BDIR=./bin/
IDIR=./include/
LDIR=./lib/
USOURCE=utils.c
UOBJECT=utils.o
UHEADER=utils.h
INCLUDE=./include
DPORT=42424
MNLDIR=./.mnl
FLAGS=-O2 -Wall -Wextra -I./include -lpthread

bin:
	mkdir $(BDIR)

server: bin
	$(CC) -c $(LDIR)$(USOURCE) -o $(IDIR)$(UOBJECT) $(FLAGS)
	$(CC) -c $(SSOURCE) $(FLAGS)
	$(CC) $(IDIR)$(UOBJECT) $(SOBJECT) -o $(BDIR)$(STARGET) $(FLAGS)

server-debug: bin
	$(CC) -c $(LDIR)$(USOURCE) -o $(IDIR)$(UOBJECT) $(FLAGS)
	$(CC) -c $(SSOURCE) $(FLAGS) -D DEBUG
	$(CC) $(IDIR)$(UOBJECT) $(SOBJECT) -o $(BDIR)$(STARGET) $(FLAGS)

client: bin
	$(CC) -c $(LDIR)$(USOURCE) -o $(IDIR)$(UOBJECT) $(FLAGS)
	$(CC) -c $(CSOURCE) $(FLAGS)
	$(CC) $(IDIR)$(UOBJECT) $(COBJECT) -o $(BDIR)$(CTARGET) $(FLAGS)

client-debug: bin
	$(CC) -c $(LDIR)$(USOURCE) -o $(IDIR)$(UOBJECT) $(FLAGS)
	$(CC) -c $(CSOURCE) $(FLAGS) -D DEBUG
	$(CC) $(IDIR)$(UOBJECT) $(COBJECT) -o $(BDIR)$(CTARGET) $(FLAGS)

all:	server client
	@echo 'built all executables'

clean:	bin 
	@touch $(BDIR)$(STARGET)
	rm -f $(BDIR)$(STARGET)
	@touch $(BDIR)$(CTARGET)
	rm -f $(BDIR)$(CTARGET)
	@touch ./$(IDIR)$(UOBJECT)
	rm -f ./$(IDIR)$(UOBJECT)
	@touch ./$(SOBJECT)
	rm -f ./$(SOBJECT)
	@touch ./$(COBJECT)
	rm -f ./$(COBJECT)
	rm -rf $(BDIR)

run:	server
	@$(BDIR)$(STARGET) $(DPORT)
	

help:
	@echo 	""
	@echo	"	/************************ MOONLOOP MAKEFILE ************************/	"
	@echo	""
	@echo	"	user@host:~/here$$ make server		builds server"
	@echo	"	user@host:~/here$$ make server-debug	builds server in debug mode"
	@echo	"	user@host:~/here$$ make client		builds client"
	@echo   "	user@host:~/here$$ make bin		creates bin folder"
	@echo 	"	user@host:~/here$$ make clean		removes binaries and object files"
	@echo   "	user@host:~/here$$ make run           	runs the server with default settings"
	@echo	"	user@host:~/here$$ make help		prints a nice help box"
	@echo	""
