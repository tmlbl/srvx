#ifndef SRVX_MQ_H
#define SRVX_MQ_H

#include <czmq.h>
#include <stdio.h>

typedef struct srvx_mq_client {
	zsock_t *push_sock;
} srvx_mq_client;

int srvx_mq_client_connect(srvx_mq_client*);

int srvx_mq_client_destroy(srvx_mq_client*);

int srvx_mq_client_send(srvx_mq_client *client, char *msg);

#endif // SRVX_MQ_H
