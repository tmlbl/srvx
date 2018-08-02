#ifndef SRVX_MQ_H
#define SRVX_MQ_H

#include <czmq.h>
#include <stdio.h>
#include <zhash.h>

#include "path.h"
#include "zhelpers.h"

typedef struct srvx_mq_client {
	void *context;
	void *requester;
	void *publisher;
	void *subscriber;
	zhash_t *subscriptions;
} srvx_mq_client;

typedef struct srvx_subscription {
	void *sock;
	char *next_msg;
	size_t msg_len;
	size_t read_offset;
} srvx_subscription;

int srvx_mq_client_connect(srvx_mq_client*);

int srvx_mq_client_destroy(srvx_mq_client*);

char* srvx_mq_client_send(srvx_mq_client *client, char *msg);

// Retrieve a pointer to a subscription struct. It will be created if it does
// not already exist.
srvx_subscription* srvx_mq_client_sub(srvx_mq_client *client, const char *path);

char* srvx_mq_client_sub_read(srvx_mq_client *client, const char *path,
	size_t size, size_t offset);

size_t srvx_mq_client_sub_peek_len(srvx_mq_client *client, const char *path);

void srvx_mq_client_publish(srvx_mq_client *client, char *path, char *data);

#endif // SRVX_MQ_H
