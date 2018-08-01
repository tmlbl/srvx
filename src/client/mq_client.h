#ifndef SRVX_MQ_H
#define SRVX_MQ_H

#include <czmq.h>
#include <stdio.h>

#include "zhelpers.h"

typedef struct srvx_mq_client {
	void *context;
	void *requester;
	void *subscriber;
} srvx_mq_client;

int srvx_mq_client_connect(srvx_mq_client*);

int srvx_mq_client_destroy(srvx_mq_client*);

char* srvx_mq_client_send(srvx_mq_client *client, char *msg);

char* srvx_mq_client_subscribe(srvx_mq_client *client, const char *path);

#endif // SRVX_MQ_H
