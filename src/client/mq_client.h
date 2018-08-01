#ifndef SRVX_MQ_H
#define SRVX_MQ_H

#include <czmq.h>
#include <stdio.h>

#include "zhelpers.h"

typedef struct srvx_mq_client {
	void *context;
	void *requester;
	void *publisher;
	void *subscriber;
} srvx_mq_client;

int srvx_mq_client_connect(srvx_mq_client*);

int srvx_mq_client_destroy(srvx_mq_client*);

char* srvx_mq_client_send(srvx_mq_client *client, char *msg);

char* srvx_mq_client_subscribe(srvx_mq_client *client, const char *path);

void srvx_mq_client_publish(srvx_mq_client *client, char *path, char *data);

#endif // SRVX_MQ_H
