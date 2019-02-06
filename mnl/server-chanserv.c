#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include "server.h"

void print_channels(struct chs * arg)
{
	struct chs *chs_data = (struct chs *) arg;
	struct tmessage * msp = chs_data->messagep;
	//char * msbuf = malloc(sizeof(char)*50);
	//if (msbuf == NULL) eonerror("error on malloc");
	int i;
	send_msg(msp->fd, "\n\nMoonloop Chanserv salutes you! Here's the channels list:");
	printf("\nchannels : %d", chs_data->channels);
	for (i = 0; i<chs_data->channels; i++)
	{
		printf("\n\tchannel : %d %s", i, chs_data->names[i]);
		//char * cbuf = chs_data->names[i];
		if (chs_data->names[i]==NULL) break;
		send_msg(msp->fd, "\n\t[*]\t");
		send_msg(msp->fd, chs_data->names[i]);
	}
	send_msg(msp->fd, "\n\nNo more channels, m8, but you could create your own one!\n\n");
	return;
}

void create_channel(struct chs * arg)
{
	struct chs *chs_data = (struct chs *) arg;
	struct tmessage * msp = chs_data->messagep;
	chs_data->names[chs_data->channels] = malloc(sizeof(char)*17);
	if (chs_data->names[chs_data->channels] == NULL) eonerror("error on malloc");
	strcpy(chs_data->names[chs_data->channels], msp->data);
	send_msg(msp->fd, "\nMoonloop Chanserv salutes you! Channel was created!\n\n");
	//printf("Creato chan -%s- -%s-", msp->data,  chs_data->names[chs_data->channels]);
	//for (int i = 0; i<chs_data->channels; i++) printf("\n\tchannel : %d %s", i, chs_data->names[i]);
	chs_data->channels++;
	return;
}

void join_channel(struct chs * arg)
{
	struct chs *chs_data = (struct chs *) arg;
	struct tmessage * msp = chs_data->messagep;
	chs_data->names[chs_data->channels] = malloc(sizeof(char)*17);
	if (chs_data->names[chs_data->channels] == NULL) eonerror("error on malloc");
	strcpy(chs_data->names[chs_data->channels], msp->data);
	send_msg(msp->fd, "\nMoonloop Chanserv salutes you! Channel was created!\n\n");
	//printf("Creato chan -%s- -%s-", msp->data,  chs_data->names[chs_data->channels]);
	//for (int i = 0; i<chs_data->channels; i++) printf("\n\tchannel : %d %s", i, chs_data->names[i]);
	chs_data->channels++;
	return;
}

void *chanthread(void *arg)
{
	struct tchannel *tch_data = (struct tchannel *) arg;
	printf("\n[%s] Channel spawned", tch_data->name);
}

void chanspawn(struct tchannel * arg)
{
	pthread_attr_t attr;
	if ((pthread_attr_init(&attr)) != 0) eonerror("pthread_attr_init fail");
	if ((pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) eonerror("pthread_attr_setdetachstate() fail");
	if ((pthread_create(&(arg->thread_tid), &attr, &chanthread, arg)) != 0) eonerror("Error in pthread_create");
	if ((pthread_attr_destroy(&attr)) != 0) eonerror ("Error in pthread_attr_destroy()");
	return;
}
void *chanserv(void *arg)
{
	struct tchannel * tmp_tch;
	struct chs *chs_data = (struct chs *) arg;
	chs_data->names = malloc(sizeof (char *)* MAX_CHANNELS);
	if (chs_data->names==NULL) eonerror("Error on malloc");
	
	//FIXME: non funziona così. Rilavorare comunicazione tra thread
	//creating public channel
	chs_data->names[0] = malloc(sizeof("Public Channel"));
	if (chs_data->names[0]==NULL) eonerror("Error on malloc");
	chs_data->channels = 1;
	strcpy(chs_data->names[0], "Public Channel");
	tmp_tch = malloc(sizeof(struct tchannel));
	if (tmp_tch==NULL) eonerror("Error on malloc");
	tmp_tch->chs_d = chs_data;
	tmp_tch->name = chs_data->names[0];
	tmp_tch->chid = 0;
	tmp_tch->clients = malloc(sizeof(int *)*MAX_CLIENTS_PER_CHANNEL);
	if (tmp_tch->clients == NULL) eonerror("Error on malloc");
	chs_data->cthdata = malloc(sizeof(struct tchannel *)*MAX_CHANNELS);
	if (chs_data->cthdata == NULL) eonerror("Error on malloc");
	chs_data->ccmt = malloc(sizeof(pthread_mutex_t *)*MAX_CHANNELS);
	if (chs_data->ccmt == NULL) eonerror("Error on malloc");
	chs_data->ccond = malloc(sizeof(pthread_cond_t *)*MAX_CHANNELS);
	if (chs_data->ccond == NULL) eonerror("Error on malloc");
	
	chs_data->cthdata[0] = tmp_tch;
	chanspawn(tmp_tch);
	
	
	printf("\n[Chanserv] Thread spawned (%u)", (unsigned int) chs_data->thread_tid);
	struct tmessage * msp = chs_data->messagep;
	for(;;)
	{
		pthread_cond_wait( chs_data->ctcond, chs_data->ctmutex ); 
		printf("\n[Chanserv] awake");
		if (msp->opm == 1) print_channels(chs_data);
		if (msp->opm == 2) create_channel(chs_data);
		if (msp->opm == 3) join_channel(chs_data);
	}
	return 0;
}

